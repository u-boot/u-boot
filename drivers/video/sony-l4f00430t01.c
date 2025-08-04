// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Ion Agorria <ion@agorria.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <spi.h>
#include <mipi_display.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include <asm/gpio.h>

#define SONY_L4F00430T01_DCS_CMD	0
#define SONY_L4F00430T01_DCS_DATA	1

struct sony_l4f00430t01_priv {
	struct udevice *vdd1v8;
	struct udevice *vdd3v0;

	struct udevice *backlight;

	struct gpio_desc reset_gpio;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= 24000000,
	.hactive.typ		= 480,
	.hfront_porch.typ	= 10,
	.hback_porch.typ	= 20,
	.hsync_len.typ		= 10,
	.vactive.typ		= 800,
	.vfront_porch.typ	= 3,
	.vback_porch.typ	= 4,
	.vsync_len.typ		= 2,
	.flags			= DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW |
				  DISPLAY_FLAGS_DE_HIGH | DISPLAY_FLAGS_PIXDATA_NEGEDGE,
};

static int sony_l4f00430t01_write(struct udevice *dev, u8 cmd, const u8 *seq, int len)
{
	u8 data[2];
	int i, ret;

	data[0] = SONY_L4F00430T01_DCS_CMD;
	data[1] = cmd;

	ret = dm_spi_xfer(dev, 9, &data, NULL, SPI_XFER_ONCE);
	if (ret)
		return ret;

	for (i = 0; i < len; i++) {
		data[0] = SONY_L4F00430T01_DCS_DATA;
		data[1] = seq[i];

		ret = dm_spi_xfer(dev, 9, &data, NULL, SPI_XFER_ONCE);
		if (ret)
			return ret;
	}

	return 0;
}

#define sony_l4f00430t01_write_seq(dev, cmd, seq...) do {		\
		static const u8 b[] = { seq };				\
		sony_l4f00430t01_write(dev, cmd, b, ARRAY_SIZE(b));	\
	} while (0)

static int sony_l4f00430t01_enable_backlight(struct udevice *dev)
{
	sony_l4f00430t01_write_seq(dev, MIPI_DCS_SET_ADDRESS_MODE, 0xd4);
	mdelay(25);

	sony_l4f00430t01_write_seq(dev, MIPI_DCS_EXIT_SLEEP_MODE);
	mdelay(150);

	sony_l4f00430t01_write_seq(dev, MIPI_DCS_SET_DISPLAY_ON);

	return 0;
}

static int sony_l4f00430t01_set_backlight(struct udevice *dev, int percent)
{
	struct sony_l4f00430t01_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_debug("%s: cannot get backlight: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return backlight_set_brightness(priv->backlight, percent);
}

static int sony_l4f00430t01_get_display_timing(struct udevice *dev,
					       struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int sony_l4f00430t01_of_to_plat(struct udevice *dev)
{
	struct sony_l4f00430t01_priv *priv = dev_get_priv(dev);
	int ret;

	ret = device_get_supply_regulator(dev, "vdd1v8-supply", &priv->vdd1v8);
	if (ret) {
		log_debug("%s: cannot get vdd1v8-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vdd3v0-supply", &priv->vdd3v0);
	if (ret) {
		log_debug("%s: cannot get vdd3v0-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: cannot decode reset-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int sony_l4f00430t01_hw_init(struct udevice *dev)
{
	struct sony_l4f00430t01_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_debug("%s: error entering reset (%d)\n", __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->vdd1v8, 1);
	if (ret) {
		log_debug("%s: enabling vdd1v8-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->vdd3v0, 1);
	if (ret) {
		log_debug("%s: enabling vdd3v0-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}
	mdelay(15);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_debug("%s: error exiting reset (%d)\n", __func__, ret);
		return ret;
	}
	mdelay(100);

	return 0;
}

static int sony_l4f00430t01_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int ret;

	ret = spi_claim_bus(slave);
	if (ret) {
		log_err("SPI bus allocation failed (%d)\n", ret);
		return ret;
	}

	return sony_l4f00430t01_hw_init(dev);
}

static const struct panel_ops sony_l4f00430t01_ops = {
	.enable_backlight	= sony_l4f00430t01_enable_backlight,
	.set_backlight		= sony_l4f00430t01_set_backlight,
	.get_display_timing	= sony_l4f00430t01_get_display_timing,
};

static const struct udevice_id sony_l4f00430t01_ids[] = {
	{ .compatible = "sony,l4f00430t01" },
	{ }
};

U_BOOT_DRIVER(sony_l4f00430t01) = {
	.name		= "sony_l4f00430t01",
	.id		= UCLASS_PANEL,
	.of_match	= sony_l4f00430t01_ids,
	.ops		= &sony_l4f00430t01_ops,
	.of_to_plat	= sony_l4f00430t01_of_to_plat,
	.probe		= sony_l4f00430t01_probe,
	.priv_auto	= sizeof(struct sony_l4f00430t01_priv),
};
