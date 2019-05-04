/*
 * FB driver for the ILI9806 LCD ili9806
 *
 * Copyright (C) 2019 Welon Knat
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/property.h>

#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/tinydrm/tinydrm-ili9806.h>


static void fb_ili9806_pipe_enable(struct drm_simple_display_pipe *pipe,
				   struct drm_crtc_state *crtc_state,
			       struct drm_plane_state *plane_state)
{
	struct tinydrm_device *tdev = pipe_to_tinydrm(pipe);
	struct tinydrm_ili9806 *ili9806 = tinydrm_to_ili9806(tdev);
	//mutex_lock(&ili9806->cmd_lock);

	//struct device *dev = tdev->drm->dev;
	i80_ctrl_command(ili9806, 0xFF, 0xFF, 0x98, 0x06);

	i80_ctrl_command(ili9806, 0xBA, 0xE0); // SPI mode SDA
	i80_ctrl_command(ili9806, 0xBC, 0x03, 0x0F, 0x63, 0x69, 0x01, 0x01, 0x1B, 0x11,
		0x70, 0x73, 0xFF,0xFF, 0x08, 0x09, 0x05, 0x00, 0xEE, 0xE2, 0x01, 0x00, 0xC1);

	i80_ctrl_command(ili9806, 0xBD, 0x01, 0x23, 0x45, 0x67, 0x01, 0x23, 0x45, 0x67);
	i80_ctrl_command(ili9806, 0xBE, 0x00, 0x22, 0x27, 0x6A, 0xBC, 0xD8, 0x92, 0x22, 0x22);

	i80_ctrl_command(ili9806, 0xC7, 0x1E); // VCOM Control 1
	i80_ctrl_command(ili9806, 0xED, 0x7F, 0x0F, 0x00);

	i80_ctrl_command(ili9806, 0xC0, 0xE3, 0x0B, 0x00); //Power Control 1
	i80_ctrl_command(ili9806, 0xFC, 0x08); //LVGL Voltage Setting
	i80_ctrl_command(ili9806, 0xDF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02); //Engineering Setting
	i80_ctrl_command(ili9806, 0xF3, 0x74); //DVDD Voltage Setting
	i80_ctrl_command(ili9806, 0xB4, 0x00, 0x00, 0x00); //Display Inversion Control
	i80_ctrl_command(ili9806, 0xF7, 0x81); //Panel Resolution Selection Set 480x854 resolution
	i80_ctrl_command(ili9806, 0xB1, 0x00, 0x10, 0x14);//Frame Rate Control 1
	i80_ctrl_command(ili9806, 0xF1, 0x29, 0x8A, 0x07); //Panel Timing Control 1
	i80_ctrl_command(ili9806, 0xF2, 0x40, 0xD2, 0x50, 0x28);//Panel Timing Control 2
	i80_ctrl_command(ili9806, 0xC1, 0x17, 0X85, 0x85, 0x20); //Power Control 2
	i80_ctrl_command(ili9806, 0xE0, 0x00, 0x0C, 0x15, 0x0D, 0x0F, 0x0C, 0x07, 0x05, 0x07,
 			0x0B, 0x10, 0x10, 0x0D, 0x17, 0x0F, 0x00);  //Positive Gamma Control
	i80_ctrl_command(ili9806, 0xE1, 0x00, 0x0D, 0x15, 0x0E, 0x10, 0x0D, 0x08, 0x06, 0x07,
			0x0C, 0x11, 0x11, 0x0E, 0x17, 0x0F, 0x00);  //Negative Gamma Correction
	i80_ctrl_command(ili9806, 0x35, 0x00); //Tearing Effect Line ON

	//i80_ctrl_command(ili9806, 0x36, 0x60);

	i80_ctrl_command(ili9806, 0x36, 0x22);
	i80_ctrl_command(ili9806, 0x37, 0x00, 0x00);

	i80_ctrl_command(ili9806, 0x2A, 0x00, 0x00, 0x03, 0x55);
	i80_ctrl_command(ili9806, 0x2B, 0x00, 0x00, 0x01, 0xDF);

	i80_ctrl_command(ili9806, 0x3A, 0x77);

	i80_ctrl_command(ili9806, 0x11);//Exit Sleep 
        msleep(120);
	i80_ctrl_command(ili9806, 0x29);// Display On 

	//tinydrm_ili9806_set_rotation(ili9806);
	//tinydrm_ili9806_set_gamma(ili9806, gamma_curves);
	
	ili9806->enabled = true;
	
	//mutex_unlock(&ili9806->cmd_lock);

	tinydrm_ili9806_enable_flush(ili9806, crtc_state, plane_state);
}

static void fb_ili9806_pipe_disable(struct drm_simple_display_pipe *pipe)
{
	struct tinydrm_device *tdev = pipe_to_tinydrm(pipe);
	struct tinydrm_ili9806 *ili9806 = tinydrm_to_ili9806(tdev);

	ili9806->enabled = false;
}

static const struct drm_simple_display_pipe_funcs fb_ili9806_funcs = {
	.enable =  fb_ili9806_pipe_enable,
	.disable = fb_ili9806_pipe_disable,
	.update = tinydrm_display_pipe_update,
	.prepare_fb = drm_gem_fb_simple_display_pipe_prepare_fb,
};


static const struct drm_display_mode fb_ili9806_mode = {
	TINYDRM_MODE(854, 480, 0, 0),
};

DEFINE_DRM_GEM_CMA_FOPS(ili9806_fops);

static struct drm_driver fb_ili9806_driver = {
	.driver_features	= DRIVER_GEM | DRIVER_MODESET | DRIVER_PRIME |
				  DRIVER_ATOMIC,
	.fops			= &ili9806_fops,
	TINYDRM_GEM_DRIVER_OPS,
	.name			= "fb_ili9806",
	.desc			= "fb_ili9806",
	.date			= "20190130",
	.major			= 1,
	.minor			= 0,
};

static struct tinydrm_ili9806 *
fb_ili9806_probe_common(struct device *dev, struct resource *reg_res,
			const struct drm_simple_display_pipe_funcs *funcs)
{
	struct tinydrm_ili9806 *ili9806;
	struct tinydrm_device *tdev;
	u32 rotation = 0;
	int ret;

	ili9806 = devm_kzalloc(dev, sizeof(*ili9806), GFP_KERNEL);
	if (!ili9806)
		return ERR_PTR(-ENOMEM);
#if 0
	ili9806->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ili9806->reset)) {
		dev_err(dev, "Failed to get gpio 'reset'\n");
		return ERR_CAST(ili9806->reset);
	}

	ili9806->backlight = tinydrm_fbtft_get_backlight(dev);
	if (IS_ERR(ili9806->backlight))
		return ERR_CAST(ili9806->backlight);
#endif
	//tinydrm_fbtft_get_rotation(dev, &rotation);
	//mutex_init(&ili9806->cmd_lock);

	ret = tinydrm_ili9806_init(dev, ili9806, funcs, reg_res, &fb_ili9806_driver,
				   &fb_ili9806_mode, rotation);
	if (ret)
		return ERR_PTR(ret);

	tdev = &ili9806->tinydrm;
	ret = devm_tinydrm_register(tdev);
	if (ret)
		return ERR_PTR(ret);

	return ili9806;
}

#if 0
static void fb_ili9806_shutdown(struct spi_device *spi)
{
	struct tinydrm_regmap *treg = to_tinydrm_regmap(tdev);

	tinydrm_shutdown(&treg->tinydrm);
}

#endif

static const struct of_device_id fb_ili9806_of_match[] = {
	{ .compatible = "ilitek,ili9806", .data = &fb_ili9806_funcs },
	{},
};
MODULE_DEVICE_TABLE(of, fb_ili9806_of_match);

static int fb_ili9806_probe_pdev(struct platform_device *pdev)
{
	const struct drm_simple_display_pipe_funcs *funcs;
	const struct platform_device_id *pdev_id;
	const struct of_device_id *match;
	struct device *dev = &pdev->dev;
	struct tinydrm_ili9806 *ili9806;
	struct resource *reg_res;
	
	
	match = of_match_device(fb_ili9806_of_match, dev);
	if (match) {
		funcs = match->data;
	} else {
		printk("[%s]: no match \n",__func__);
		pdev_id = platform_get_device_id(pdev);
		funcs = (const struct drm_simple_display_pipe_funcs *)pdev_id->driver_data;
	}

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!reg_res)
		return -ENODEV;

	ili9806 = fb_ili9806_probe_common(dev, reg_res, funcs);
	if (IS_ERR(ili9806))
		return PTR_ERR(ili9806);

	platform_set_drvdata(pdev, ili9806->tinydrm.drm);

	

	DRM_DEV_DEBUG_DRIVER(dev, "Initialized on minor %d\n",
			     ili9806->tinydrm.drm->primary->index);

	return 0;
}

static const struct platform_device_id fb_ili9806_platform_ids[] = {
	{ "fb_ili9806", (unsigned long)&fb_ili9806_funcs },
	{ },
};
MODULE_DEVICE_TABLE(platform, fb_ili9806_platform_ids);

static struct platform_driver fb_ili9806_platform_driver = {
	.driver = {
		.name   = "fb_ili9806",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(fb_ili9806_of_match),
	},
	.id_table = fb_ili9806_platform_ids,
	.probe = fb_ili9806_probe_pdev,
};

static int __init fb_ili9806_module_init(void)
{
	return platform_driver_register(&fb_ili9806_platform_driver);
}
module_init(fb_ili9806_module_init);

static void __exit fb_ili9806_module_exit(void)
{
	platform_driver_unregister(&fb_ili9806_platform_driver);
}
module_exit(fb_ili9806_module_exit);


MODULE_ALIAS("platform:ili9806");

MODULE_DESCRIPTION("DRM driver for the ILI9806 LCD ili9806");
MODULE_AUTHOR("Welon Tank");
MODULE_LICENSE("GPL");

