/*
 * Copyright (C) 2019 Welon Tank
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <asm/unaligned.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/tinydrm/tinydrm-ili9806.h>
#include <linux/dma-buf.h>

void i80_ctrl_command_buf(struct tinydrm_ili9806 *ili9806, u8 cmd, u8 *data, size_t len)
{
	int i = 0;
	writeb(cmd, ili9806->base + CMD_CONTROL);
	while(len){
		writeb( *(data+i), ili9806->base + DATA_CONTROL);
		i++;
		len--;
	}
}
EXPORT_SYMBOL(i80_ctrl_command_buf);

void i80_ctrl_data_buf (struct tinydrm_ili9806 *ili9806, u8 cmd, u32 *data, size_t len)
{
	int i = 0;
	writeb(cmd, ili9806->base + CMD_CONTROL);
	while(len){
		writel( *(data+i), ili9806->base + DATA_CONTROL);
		i++;
		len--;
	}
}
EXPORT_SYMBOL(i80_ctrl_data_buf);

void tinydrm_ili9806_enable_flush(struct tinydrm_ili9806 *ili9806,
			   struct drm_crtc_state *crtc_state,
			   struct drm_plane_state *plane_state)
{
	struct tinydrm_device *tdev = &ili9806->tinydrm;
	struct drm_framebuffer *fb = plane_state->fb;

	ili9806->enabled = true;
	if (fb)
		tdev->fb_dirty(fb, NULL, 0, 0, NULL, 0);
}
EXPORT_SYMBOL(tinydrm_ili9806_enable_flush);


static int tinydrm_ili9806_buf_copy(void *dst, struct drm_framebuffer *fb,
		      struct drm_clip_rect *clip)
{
	struct drm_gem_cma_object *cma_obj = drm_fb_cma_get_gem_obj(fb, 0);
	struct dma_buf_attachment *import_attach = cma_obj->base.import_attach;
	struct drm_format_name_buf format_name;
	void *src = cma_obj->vaddr;
	int ret = 0;

	if (import_attach) {
		ret = dma_buf_begin_cpu_access(import_attach->dmabuf,
					       DMA_FROM_DEVICE);
		if (ret)
			return ret;
	}
	
	if(fb->format->format == DRM_FORMAT_XRGB8888){
		tinydrm_memcpy(dst, src, fb, clip);
	} else {
		dev_err_once(fb->dev->dev, "Format is not supported: %s\n",
			drm_get_format_name(fb->format->format,
						 &format_name));
		return -EINVAL;
	}
		

	if (import_attach)
		ret = dma_buf_end_cpu_access(import_attach->dmabuf,
					     DMA_FROM_DEVICE);
	return ret;
}
//EXPORT_SYMBOL(tinydrm_ili9806_buf_copy);

static int tinydrm_ili9806_fb_dirty(struct drm_framebuffer *fb,
			     struct drm_file *file_priv,
			     unsigned int flags, unsigned int color,
			     struct drm_clip_rect *clips,
			     unsigned int num_clips)
{
	struct drm_gem_cma_object *cma_obj = drm_fb_cma_get_gem_obj(fb, 0);
	struct tinydrm_device *tdev = fb->dev->dev_private;
	struct tinydrm_ili9806 *ili9806 = tinydrm_to_ili9806(tdev);
	bool swap = ili9806->swap_bytes;
	struct drm_clip_rect clip;
	int ret = 0;
	bool full;
	void *tr;

	if (!ili9806->enabled)
		return 0;

	full = tinydrm_merge_clips(&clip, clips, num_clips, flags,
				   fb->width, fb->height);

	DRM_DEBUG("Flushing [FB:%d] x1=%u, x2=%u, y1=%u, y2=%u, swap=%u\n",
		  fb->base.id, clip.x1, clip.x2, clip.y1, clip.y2, swap);

	if (ili9806->always_tx_buf || swap || !full ||
	    fb->format->format == DRM_FORMAT_XRGB8888) {
		tr = ili9806->tx_buf;
		ret = tinydrm_ili9806_buf_copy(tr,fb, &clip);
		if (ret)
			return ret;
	} else {
		tr = cma_obj->vaddr;
	}

	i80_ctrl_command(ili9806, 0x2a, clip.x1>>8, clip.x1&0xff, clip.x2>>8, (clip.x2-1)&0xff);
	i80_ctrl_command(ili9806, 0x2b, clip.y1>>8, clip.y1&0xff, clip.y2>>8, (clip.y2-1)&0xff);
	i80_ctrl_data_buf(ili9806, 0x2c, tr, (clip.x2 - clip.x1) * (clip.y2 - clip.y1));

	return ret;
}

static const struct drm_framebuffer_funcs tinydrm_ili9806_fb_funcs = {
	.destroy	= drm_gem_fb_destroy,
	.create_handle	= drm_gem_fb_create_handle,
	.dirty		= tinydrm_fb_dirty,
};

static const uint32_t tinydrm_ili9806_formats[] = {
	DRM_FORMAT_XRGB8888,
};

/**
 * tinydrm_ili9806_init - Initialize &tinydrm_simple for use with ili9806
 * @dev: Parent device
 * @panel: &tinydrm_panel structure to initialize
 * @funcs: Callbacks for the panel (optional)
 * @reg: Register map
 * @driver: DRM driver
 * @mode: Display mode
 * @rotation: Initial rotation in degrees Counter Clock Wise
 *
 * This function initializes a &tinydrm_panel structure and it's underlying
 * @tinydrm_device. It also sets up the display pipeline.
 *
 * Supported formats: Native RGB565 and emulated XRGB8888.
 *
 * Objects created by this function will be automatically freed on driver
 * detach (devres).
 *
 * Returns:
 * Zero on success, negative error code on failure.
 */
int tinydrm_ili9806_init(struct device *dev, struct tinydrm_ili9806 *ili9806,
			 const struct drm_simple_display_pipe_funcs *funcs,
			 struct resource *reg_res, struct drm_driver *driver,
			 const struct drm_display_mode *mode,
			 unsigned int rotation)
{
	size_t bufsize = mode->vdisplay * mode->hdisplay * sizeof(u32);
	struct tinydrm_device *tdev = &ili9806->tinydrm;
	int ret;
	ili9806->rotation = rotation;

	ili9806->tx_buf = devm_kmalloc(dev, bufsize, GFP_KERNEL);
	if (!ili9806->tx_buf)
		return -ENOMEM;

	ili9806->base = devm_ioremap_resource(dev, reg_res);

	ret = devm_tinydrm_init(dev, tdev, &tinydrm_ili9806_fb_funcs, driver);
	if (ret)
		return ret;

	 tdev->fb_dirty = tinydrm_ili9806_fb_dirty;

	/* TODO: Maybe add DRM_MODE_CONNECTOR_SPI */
	ret = tinydrm_display_pipe_init(tdev, funcs,
					DRM_MODE_CONNECTOR_VIRTUAL,
					tinydrm_ili9806_formats,
					ARRAY_SIZE(tinydrm_ili9806_formats), mode,
					rotation);
	if (ret)
		return ret;

	tdev->drm->mode_config.preferred_depth = 32;

	drm_mode_config_reset(tdev->drm);

	DRM_DEBUG_KMS("preferred_depth=%u, rotation = %u\n",
		      tdev->drm->mode_config.preferred_depth, rotation);
	
	return 0;
}
EXPORT_SYMBOL(tinydrm_ili9806_init);

