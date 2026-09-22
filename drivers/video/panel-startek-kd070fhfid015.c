// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2016 InforceComputing
 * Copyright (C) 2016 Linaro Ltd
 * Copyright (C) 2026 BayLibre, SAS
 *
 * Authors:
 * - Vinay Simha BN <simhavcs@gmail.com>
 * - Sumit Semwal <sumit.semwal@linaro.org>
 * - Guillaume La Roque <glaroque@baylibre.com>
 *
 * U-Boot port:
 * Authors:
 * - Julien Stephan <jstephan@baylibre.com>
 */

#include <asm-generic/gpio.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <mipi_display.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <power/regulator.h>

#define DSI_REG_MCAP	0xb0
#define DSI_REG_IS	0xb3 /* Interface Setting */
#define DSI_REG_IIS	0xb4 /* Interface ID Setting */
#define DSI_REG_CTRL	0xb6

struct stk_panel {
	const struct drm_display_mode *mode;
	struct udevice *dev;
	struct gpio_desc *enable_gpio; /* Power IC supply enable */
	struct gpio_desc *reset_gpio; /* External reset */
	struct mipi_dsi_device *dsi;
	struct udevice *iovcc;
	struct udevice *power;
};

static const struct drm_display_mode default_mode = {
	.clock = 163204,
	.hdisplay = 1200,
	.hsync_start = 1200 + 144,
	.hsync_end = 1200 + 144 + 16,
	.htotal = 1200 + 144 + 16 + 45,
	.vdisplay = 1920,
	.vsync_start = 1920 + 8,
	.vsync_end = 1920 + 8 + 4,
	.vtotal = 1920 + 8 + 4 + 4,
};

static int stk015_panel_init(struct stk_panel *stk)
{
	struct mipi_dsi_device *dsi = stk->dsi;
	struct udevice *dev = stk->dev;
	int ret;

	ret = mipi_dsi_dcs_soft_reset(dsi);
	if (ret < 0) {
		dev_err(dev, "failed to mipi_dsi_dcs_soft_reset: %d\n", ret);
		return ret;
	}
	mdelay(5);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "failed to set exit sleep mode: %d\n", ret);
		return ret;
	}
	mdelay(120);

	mipi_dsi_generic_write_seq(dsi, DSI_REG_MCAP, 0x04);

	/* Interface setting, video mode */
	mipi_dsi_generic_write_seq(dsi, DSI_REG_IS, 0x14, 0x08, 0x00, 0x22, 0x00);
	mipi_dsi_generic_write_seq(dsi, DSI_REG_IIS, 0x0c, 0x00);
	mipi_dsi_generic_write_seq(dsi, DSI_REG_CTRL, 0x3a, 0xd3);

	ret = mipi_dsi_dcs_set_display_brightness(dsi, 0x77);
	if (ret < 0) {
		dev_err(dev, "failed to write display brightness: %d\n", ret);
		return ret;
	}

	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY,
			       MIPI_DCS_WRITE_MEMORY_START);

	ret = mipi_dsi_dcs_set_pixel_format(dsi, 0x77);
	if (ret < 0) {
		dev_err(dev, "failed to set pixel format: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_column_address(dsi, 0, stk->mode->hdisplay - 1);
	if (ret < 0) {
		dev_err(dev, "failed to set column address: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_page_address(dsi, 0, stk->mode->vdisplay - 1);
	if (ret < 0) {
		dev_err(dev, "failed to set page address: %d\n", ret);
		return ret;
	}

	return 0;
}

static int stk015_panel_on(struct stk_panel *stk)
{
	struct mipi_dsi_device *dsi = stk->dsi;
	struct udevice *dev = stk->dev;
	int ret;

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0)
		dev_err(dev, "failed to set display on: %d\n", ret);

	mdelay(20);

	return ret;
}

static int stk015_panel_enable_backlight(struct udevice *dev)
{
	struct stk_panel *stk = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	stk->dsi = dsi;
	ret = mipi_dsi_attach(dsi);
	if (ret < 0)
		return ret;

	dm_gpio_set_value(stk->reset_gpio, 0);
	dm_gpio_set_value(stk->enable_gpio, 0);
	ret = regulator_enable(stk->iovcc);
	if (ret < 0)
		goto detach;

	mdelay(8);
	ret = regulator_enable(stk->power);
	if (ret < 0)
		goto iovccoff;

	mdelay(20);
	dm_gpio_set_value(stk->enable_gpio, 1);
	mdelay(20);
	dm_gpio_set_value(stk->reset_gpio, 1);
	mdelay(10);

	ret = stk015_panel_init(stk);
	if (ret < 0) {
		dev_err(dev, "failed to init panel: %d\n", ret);
		goto poweroff;
	}

	ret = stk015_panel_on(stk);
	if (ret < 0) {
		dev_err(dev, "failed to set panel on: %d\n", ret);
		goto poweroff;
	}

	return 0;

poweroff:
	regulator_disable(stk->power);
iovccoff:
	regulator_disable(stk->iovcc);
	dm_gpio_set_value(stk->reset_gpio, 0);
	dm_gpio_set_value(stk->enable_gpio, 0);
detach:
	mipi_dsi_detach(dsi);

	return ret;
}

static int stk015_panel_add(struct stk_panel *stk)
{
	struct udevice *dev = stk->dev;
	int ret;

	stk->mode = &default_mode;

	ret = device_get_supply_regulator(dev, "iovcc-supply", &stk->iovcc);
	if (ret) {
		dev_err(dev, "Failed to get iovcc regulator: %d\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "power-supply", &stk->power);
	if (ret) {
		dev_err(dev, "Failed to get power regulator: %d\n", ret);
		return ret;
	}

	stk->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_IS_OUT);
	if (IS_ERR(stk->reset_gpio)) {
		ret = PTR_ERR(stk->reset_gpio);
		dev_err(dev, "cannot get reset-gpios %d\n", ret);
		return ret;
	}

	stk->enable_gpio = devm_gpiod_get(dev, "enable", GPIOD_IS_OUT);
	if (IS_ERR(stk->enable_gpio)) {
		ret = PTR_ERR(stk->enable_gpio);
		dev_err(dev, "cannot get enable-gpio %d\n", ret);
		return ret;
	}

	return 0;
}

static int stk015_panel_probe(struct udevice *dev)
{
	struct stk_panel *stk = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	int ret;

	stk->dev = dev;

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM;

	ret = stk015_panel_add(stk);
	if (ret < 0)
		return ret;

	return 0;
}

static int stk015_panel_get_modes(struct udevice *dev, const struct drm_display_mode **modes)
{
	struct stk_panel *stk = dev_get_priv(dev);

	if (!stk->mode)
		return -ENODEV;

	*modes = stk->mode;

	return 1;
}

static const struct panel_ops stk015_panel_ops = {
	.enable_backlight	= stk015_panel_enable_backlight,
	.get_modes		= stk015_panel_get_modes,
};

static const struct udevice_id stk015_of_match[] = {
	{ .compatible = "startek,kd070fhfid015", },
	{ }
};

U_BOOT_DRIVER(stk015_panel_driver) = {
	.name		= "panel-startek-kd070fhfid015",
	.id		= UCLASS_PANEL,
	.of_match	= stk015_of_match,
	.ops		= &stk015_panel_ops,
	.probe		= stk015_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct stk_panel),
};
