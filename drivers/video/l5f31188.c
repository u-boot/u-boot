/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 * Hyungwon Hwang <human.hwang@samsung.com>
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#include <common.h>
#include <asm/arch/mipi_dsim.h>

#define SCAN_FROM_LEFT_TO_RIGHT 0
#define SCAN_FROM_RIGHT_TO_LEFT 1
#define SCAN_FROM_TOP_TO_BOTTOM 0
#define SCAN_FROM_BOTTOM_TO_TOP 1

static void l5f31188_sleep_in(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE, 0x10, 0x00);
}

static void l5f31188_sleep_out(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE, 0x11, 0x00);
}

static void l5f31188_set_gamma(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE, 0x26, 0x00);
}

static void l5f31188_display_off(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE, 0x28, 0x00);
}

static void l5f31188_display_on(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE, 0x29, 0x00);
}

static void l5f31188_ctl_memory_access(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops,
		int h_direction, int v_direction)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, 0x36,
			(((h_direction & 0x1) << 1) | (v_direction & 0x1)));
}

static void l5f31188_set_pixel_format(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, 0x3A, 0x70);
}

static void l5f31188_write_disbv(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops, unsigned int brightness)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, 0x51, brightness);
}

static void l5f31188_write_ctrld(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, 0x53, 0x2C);
}

static void l5f31188_write_cabc(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops,
			unsigned int wm_mode)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, 0x55, wm_mode);
}

static void l5f31188_write_cabcmb(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops, unsigned int min_brightness)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, 0x5E,
			min_brightness);
}

static void l5f31188_set_extension(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	const unsigned char data_to_send[] = {
		0xB9, 0xFF, 0x83, 0x94
	};

	ops->cmd_write(dev, MIPI_DSI_DCS_LONG_WRITE,
			(unsigned int)data_to_send, ARRAY_SIZE(data_to_send));
}

static void l5f31188_set_dgc_lut(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	const unsigned char data_to_send[] = {
		0xC1, 0x01, 0x00, 0x04, 0x0E, 0x18, 0x1E, 0x26,
		0x2F, 0x36, 0x3E, 0x47, 0x4E, 0x56, 0x5D, 0x65,
		0x6D, 0x75, 0x7D, 0x84, 0x8C, 0x94, 0x9C, 0xA4,
		0xAD, 0xB5, 0xBD, 0xC5, 0xCC, 0xD4, 0xDE, 0xE5,
		0xEE, 0xF7, 0xFF, 0x3F, 0x9A, 0xCE, 0xD4, 0x21,
		0xA1, 0x26, 0x54, 0x00, 0x00, 0x04, 0x0E, 0x19,
		0x1F, 0x27, 0x30, 0x37, 0x40, 0x48, 0x50, 0x58,
		0x60, 0x67, 0x6F, 0x77, 0x7F, 0x87, 0x8F, 0x97,
		0x9F, 0xA7, 0xB0, 0xB8, 0xC0, 0xC8, 0xCE, 0xD8,
		0xE0, 0xE7, 0xF0, 0xF7, 0xFF, 0x3C, 0xEB, 0xFD,
		0x2F, 0x66, 0xA8, 0x2C, 0x46, 0x00, 0x00, 0x04,
		0x0E, 0x18, 0x1E, 0x26, 0x30, 0x38, 0x41, 0x4A,
		0x52, 0x5A, 0x62, 0x6B, 0x73, 0x7B, 0x83, 0x8C,
		0x94, 0x9C, 0xA5, 0xAD, 0xB6, 0xBD, 0xC5, 0xCC,
		0xD4, 0xDD, 0xE3, 0xEB, 0xF2, 0xF9, 0xFF, 0x3F,
		0xA4, 0x8A, 0x8F, 0xC7, 0x33, 0xF5, 0xE9, 0x00
	};
	ops->cmd_write(dev, MIPI_DSI_DCS_LONG_WRITE,
			(unsigned int)data_to_send, ARRAY_SIZE(data_to_send));
}

static void l5f31188_set_tcon(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	const unsigned char data_to_send[] = {
		0xC7, 0x00, 0x20
	};
	ops->cmd_write(dev, MIPI_DSI_DCS_LONG_WRITE,
			(unsigned int)data_to_send, ARRAY_SIZE(data_to_send));
}

static void l5f31188_set_ptba(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	const unsigned char data_to_send[] = {
		0xBF, 0x06, 0x10
	};
	ops->cmd_write(dev, MIPI_DSI_DCS_LONG_WRITE,
			(unsigned int)data_to_send, ARRAY_SIZE(data_to_send));
}

static void l5f31188_set_eco(struct mipi_dsim_device *dev,
		struct mipi_dsim_master_ops *ops)
{
	ops->cmd_write(dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, 0xC6, 0x0C);
}

static int l5f31188_panel_init(struct mipi_dsim_device *dev)
{
	struct mipi_dsim_master_ops *ops = dev->master_ops;

	l5f31188_set_extension(dev, ops);
	l5f31188_set_dgc_lut(dev, ops);

	l5f31188_set_eco(dev, ops);
	l5f31188_set_tcon(dev, ops);
	l5f31188_set_ptba(dev, ops);
	l5f31188_set_gamma(dev, ops);
	l5f31188_ctl_memory_access(dev, ops,
			SCAN_FROM_LEFT_TO_RIGHT, SCAN_FROM_TOP_TO_BOTTOM);
	l5f31188_set_pixel_format(dev, ops);
	l5f31188_write_disbv(dev, ops, 0xFF);
	l5f31188_write_ctrld(dev, ops);
	l5f31188_write_cabc(dev, ops, 0x0);
	l5f31188_write_cabcmb(dev, ops, 0x0);

	l5f31188_sleep_out(dev, ops);

	/* 120 msec */
	udelay(120 * 1000);

	return 0;
}

static void l5f31188_display_enable(struct mipi_dsim_device *dev)
{
	struct mipi_dsim_master_ops *ops = dev->master_ops;
	l5f31188_display_on(dev, ops);
}

static struct mipi_dsim_lcd_driver l5f31188_dsim_ddi_driver = {
	.name = "l5f31188",
	.id = -1,

	.mipi_panel_init = l5f31188_panel_init,
	.mipi_display_on = l5f31188_display_enable,
};

void l5f31188_init(void)
{
	exynos_mipi_dsi_register_lcd_driver(&l5f31188_dsim_ddi_driver);
}
