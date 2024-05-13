// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <backlight.h>
#include <dm.h>
#include <edid.h>
#include <i2c.h>
#include <log.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>
#include <power/regulator.h>

#define EDID_I2C_ADDR	0x50

struct simple_panel_priv {
	struct udevice *reg;
	struct udevice *backlight;
	struct gpio_desc enable;
};

static int simple_panel_enable_backlight(struct udevice *dev)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: start, backlight = '%s'\n", __func__, priv->backlight->name);
	dm_gpio_set_value(&priv->enable, 1);
	ret = backlight_enable(priv->backlight);
	debug("%s: done, ret = %d\n", __func__, ret);
	if (ret)
		return ret;

	return 0;
}

static int simple_panel_set_backlight(struct udevice *dev, int percent)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: start, backlight = '%s'\n", __func__, priv->backlight->name);
	dm_gpio_set_value(&priv->enable, 1);
	ret = backlight_set_brightness(priv->backlight, percent);
	debug("%s: done, ret = %d\n", __func__, ret);
	if (ret)
		return ret;

	return 0;
}

#if CONFIG_IS_ENABLED(I2C_EDID) && CONFIG_IS_ENABLED(DM_I2C)
static int simple_panel_get_edid_timing(struct udevice *dev,
					struct display_timing *timings)
{
	struct udevice *panel_ddc, *panel_edid;
	struct display_timing edid_timing;
	u8 edid_buf[EDID_SIZE] = { 0 };
	int ret, bpc;
	/* Check for DDC i2c if no timings are provided */
	ret = uclass_get_device_by_phandle(UCLASS_I2C, dev,
					   "ddc-i2c-bus",
					   &panel_ddc);
	if (ret) {
		log_debug("%s: cannot get DDC i2c bus: error %d\n",
			  __func__, ret);
		return ret;
	}

	ret = dm_i2c_probe(panel_ddc, EDID_I2C_ADDR, 0, &panel_edid);
	if (ret) {
		log_debug("%s: cannot probe EDID: error %d\n",
			  __func__, ret);
		return ret;
	}

	ret = dm_i2c_read(panel_edid, 0, edid_buf, sizeof(edid_buf));
	if (ret) {
		log_debug("%s: cannot dump EDID buffer: error %d\n",
			  __func__, ret);
		return ret;
	}

	ret = edid_get_timing(edid_buf, sizeof(edid_buf),
			      &edid_timing, &bpc);
	if (ret) {
		log_debug("%s: cannot decode EDID info: error %d\n",
			  __func__, ret);
		return ret;
	}

	memcpy(timings, &edid_timing, sizeof(*timings));

	return 0;
}
#else
static int simple_panel_get_edid_timing(struct udevice *dev,
					struct display_timing *timings)
{
	return -ENOTSUPP;
}
#endif

static int simple_panel_get_display_timing(struct udevice *dev,
					   struct display_timing *timings)
{
	const void *blob = gd->fdt_blob;
	int ret;

	/* Check for timing subnode if panel node first */
	ret = fdtdec_decode_display_timing(blob, dev_of_offset(dev),
					   0, timings);
	if (!ret)
		return ret;

	return simple_panel_get_edid_timing(dev, timings);
}

static int simple_panel_of_to_plat(struct udevice *dev)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
						   "power-supply", &priv->reg);
		if (ret) {
			debug("%s: Warning: cannot get power supply: ret=%d\n",
			      __func__, ret);
			if (ret != -ENOENT)
				return ret;
		}
	}

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
						   "backlight", &priv->backlight);
	if (ret) {
		debug("%s: Cannot get backlight: ret=%d\n", __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT);
	if (ret) {
		debug("%s: Warning: cannot get enable GPIO: ret=%d\n",
		      __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	return 0;
}

static int simple_panel_probe(struct udevice *dev)
{
	struct simple_panel_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_panel_plat *dsi_data =
		(struct mipi_dsi_panel_plat *)dev_get_driver_data(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->reg, true);
	if (ret && ret != -ENOSYS) {
		debug("%s: failed to enable regulator '%s' %d\n",
		      __func__, priv->reg->name, ret);
		return ret;
	}

	if (dsi_data)
		memcpy(plat, dsi_data, sizeof(struct mipi_dsi_panel_plat));

	return 0;
}

static const struct panel_ops simple_panel_ops = {
	.enable_backlight	= simple_panel_enable_backlight,
	.set_backlight		= simple_panel_set_backlight,
	.get_display_timing	= simple_panel_get_display_timing,
};

static const struct mipi_dsi_panel_plat panasonic_vvx10f004b00 = {
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
		      MIPI_DSI_CLOCK_NON_CONTINUOUS,
	.format = MIPI_DSI_FMT_RGB888,
	.lanes = 4,
};

static const struct udevice_id simple_panel_ids[] = {
	{ .compatible = "simple-panel" },
	{ .compatible = "auo,b133xtn01" },
	{ .compatible = "auo,b116xw03" },
	{ .compatible = "auo,b133htn01" },
	{ .compatible = "boe,nv140fhmn49" },
	{ .compatible = "lg,lb070wv8" },
	{ .compatible = "sharp,lq123p1jx31" },
	{ .compatible = "boe,nv101wxmn51" },
	{ .compatible = "panasonic,vvx10f004b00",
	  .data = (ulong)&panasonic_vvx10f004b00 },
	{ }
};

U_BOOT_DRIVER(simple_panel) = {
	.name		= "simple_panel",
	.id		= UCLASS_PANEL,
	.of_match	= simple_panel_ids,
	.ops		= &simple_panel_ops,
	.of_to_plat	= simple_panel_of_to_plat,
	.probe		= simple_panel_probe,
	.priv_auto	= sizeof(struct simple_panel_priv),
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
};
