/*
 * Copyright (C) 2017 Noralf Trønnes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LINUX_TINYDRM_ILI9806_H
#define __LINUX_TINYDRM_ILI9806_H

#include <drm/tinydrm/tinydrm.h>
#include <drm/tinydrm/tinydrm-helpers2.h>

#define DATA_CONTROL 4
#define CMD_CONTROL  0

/**
 * struct tinydrm_ili9806 - tinydrm ILI9806 device
 * @tinydrm: Base &tinydrm_device
 * @reg: Register map (optional)
 * @enabled: Pipeline is enabled
 * @tx_buf: Transmit buffer
 * @swap_bytes: Swap pixel data bytes
 * @always_tx_buf:
 * @rotation: Rotation in degrees Counter Clock Wise
 * @reset: Optional reset gpio
 * @backlight: Optional backlight device
 * @regulator: Optional regulator
 */
struct tinydrm_ili9806 {
	struct tinydrm_device tinydrm;
	void __iomem *base;
	bool enabled;
	void *tx_buf;
	bool swap_bytes;
	bool always_tx_buf;
	unsigned int rotation;
	struct regulator *regulator;
       // struct mutex cmd_lock;
};

static inline struct tinydrm_ili9806 *
tinydrm_to_ili9806(struct tinydrm_device *tdev)
{
	return container_of(tdev, struct tinydrm_ili9806, tinydrm);
}

int tinydrm_ili9806_init(struct device *dev, struct tinydrm_ili9806 *ili9806,
			 const struct drm_simple_display_pipe_funcs *funcs,
			 struct resource *reg_res, struct drm_driver *driver,
			 const struct drm_display_mode *mode,
			 unsigned int rotation);

void tinydrm_ili9806_enable_flush(struct tinydrm_ili9806 *ili9806,
			   struct drm_crtc_state *crtc_state,
			   struct drm_plane_state *plane_state);

#define i80_ctrl_command(i80, cmd, seq...) \
({ \
	u8 d[] = { seq }; \
	i80_ctrl_command_buf(i80, cmd, d, ARRAY_SIZE(d)); \
})


static inline void i80ctrl_write_data(struct tinydrm_ili9806 *ili9806, int val)
{
	writel(val, ili9806->base + DATA_CONTROL);
}

static inline void i80ctrl_write_cmd(struct tinydrm_ili9806 *ili9806, int val)
{
	writel(val, ili9806->base + CMD_CONTROL);
}

void i80_ctrl_command_buf(struct tinydrm_ili9806 *ili9806, u8 cmd, u8 *data, size_t len);

void i80_ctrl_data_buf (struct tinydrm_ili9806 *ili9806, u8 cmd, u32 *data, size_t len);


#endif /* __LINUX_TINYDRM_ILI9806_H */
