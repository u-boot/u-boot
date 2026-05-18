// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Amarula Solutions, Dario Binacchi <dario.binacchi@amarulasolutions.com>
 */

#include <backlight.h>
#include <dm.h>
#include <mipi_display.h>
#include <panel.h>
#include <spi.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <power/regulator.h>

struct ilitek_ili9806e_priv {
	struct udevice *vdd;
	struct udevice *backlight;
	struct gpio_desc reset_gpio;
	const struct ilitek_ili9806e_desc *desc;
};

struct ilitek_ili9806e_desc {
	const struct display_timing timing;
	void (*init_sequence)(struct udevice *dev);
};

static int ilitek_ili9806e_dcs_write(struct udevice *dev, u8 cmd, const u8 *seq, int len)
{
	u16 data[16];
	int i, ret;

	if ((len + 1) > ARRAY_SIZE(data)) {
		dev_err(dev, "Command length (%d) exceeds buffer size (%lu)\n",
			len + 1, ARRAY_SIZE(data));
		return -EMSGSIZE;
	}

	data[0] = cmd;
	if (len) {
		for (i = 0; i < len; i++)
			data[i + 1] = seq[i] | 0x0100;
	}

	ret = dm_spi_xfer(dev, (len + 1) * 8 * sizeof(u16), data, NULL,
			  SPI_XFER_ONCE);
	return 0;
}

#define ilitek_ili9806e_dcs_write_seq(dev, cmd, seq...)		\
({								\
	static const u8 b[] = { seq };				\
	ilitek_ili9806e_dcs_write(dev, cmd, b, ARRAY_SIZE(b));	\
})

static int ilitek_ili9806e_enable_backlight(struct udevice *dev)
{
	struct ilitek_ili9806e_priv *priv = dev_get_priv(dev);
	const struct ilitek_ili9806e_desc *desc = priv->desc;

	desc->init_sequence(dev);

	return panel_set_backlight(dev, BACKLIGHT_DEFAULT);
}

static int ilitek_ili9806e_set_backlight(struct udevice *dev, int percent)
{
	struct ilitek_ili9806e_priv *priv = dev_get_priv(dev);
	int ret;

	ret = backlight_enable(priv->backlight);
	if (ret) {
		dev_err(dev, "Cannot enable backlight\n");
		return ret;
	}

	ret = backlight_set_brightness(priv->backlight, percent);
	if (ret)
		dev_err(dev, "Cannot set backlight brightness\n");

	return ret;
}

static int ilitek_ili9806e_get_display_timing(struct udevice *dev,
					      struct display_timing *timing)
{
	struct ilitek_ili9806e_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->desc->timing, sizeof(*timing));

	return 0;
}

static int ilitek_ili9806e_of_to_plat(struct udevice *dev)
{
	struct ilitek_ili9806e_priv *priv = dev_get_priv(dev);
	int ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret = device_get_supply_regulator(dev, "vdd-supply", &priv->vdd);
		if (ret) {
			dev_err(dev, "Cannot get vdd supply\n");
			return ret;
		}
	}

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		dev_err(dev, "Cannot get backlight\n");
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Cannot get reset GPIO\n");
		return ret;
	}

	return 0;
}

static int ilitek_ili9806e_hw_init(struct udevice *dev)
{
	struct ilitek_ili9806e_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		dev_err(dev, "Cannot enter reset\n");
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->vdd, 1);
	if (ret) {
		dev_err(dev, "Cannot enable vdd-supply\n");
		return ret;
	}

	mdelay(20);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		dev_err(dev, "Cannot exit reset\n");
		return ret;
	}

	mdelay(20);

	return 0;
}

static int ilitek_ili9806e_probe(struct udevice *dev)
{
	struct ilitek_ili9806e_priv *priv = dev_get_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int ret;

	ret = spi_set_wordlen(slave, 9);
	if (ret) {
		dev_err(dev, "Cannot set SPI.bits_per_word\n");
		return ret;
	}

	ret = spi_claim_bus(slave);
	if (ret) {
		dev_err(dev, "Cannot get SPI bus\n");
		return ret;
	}

	priv->desc = (struct ilitek_ili9806e_desc *)dev_get_driver_data(dev);

	return ilitek_ili9806e_hw_init(dev);
}

static const struct panel_ops ilitek_ili9806e_ops = {
	.enable_backlight = ilitek_ili9806e_enable_backlight,
	.set_backlight = ilitek_ili9806e_set_backlight,
	.get_display_timing = ilitek_ili9806e_get_display_timing,
};

static void rk050hr345_ct106a_init(struct udevice *dev)
{
	/* Switch to page 1 */
	ilitek_ili9806e_dcs_write_seq(dev, 0xff, 0xff, 0x98, 0x06, 0x04, 0x01);
	/* Interface Settings */
	ilitek_ili9806e_dcs_write_seq(dev, 0x08, 0x10);
	ilitek_ili9806e_dcs_write_seq(dev, 0x21, 0x01);
	/* Panel Settings */
	ilitek_ili9806e_dcs_write_seq(dev, 0x30, 0x01);
	ilitek_ili9806e_dcs_write_seq(dev, 0x31, 0x00);
	/* Power Control */
	ilitek_ili9806e_dcs_write_seq(dev, 0x40, 0x15);
	ilitek_ili9806e_dcs_write_seq(dev, 0x41, 0x44);
	ilitek_ili9806e_dcs_write_seq(dev, 0x42, 0x03);
	ilitek_ili9806e_dcs_write_seq(dev, 0x43, 0x09);
	ilitek_ili9806e_dcs_write_seq(dev, 0x44, 0x09);
	ilitek_ili9806e_dcs_write_seq(dev, 0x50, 0x78);
	ilitek_ili9806e_dcs_write_seq(dev, 0x51, 0x78);
	ilitek_ili9806e_dcs_write_seq(dev, 0x52, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x53, 0x3a);
	ilitek_ili9806e_dcs_write_seq(dev, 0x57, 0x50);
	/* Timing Control */
	ilitek_ili9806e_dcs_write_seq(dev, 0x60, 0x07);
	ilitek_ili9806e_dcs_write_seq(dev, 0x61, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x62, 0x08);
	ilitek_ili9806e_dcs_write_seq(dev, 0x63, 0x00);
	/* Gamma Settings */
	ilitek_ili9806e_dcs_write_seq(dev, 0xa0, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa1, 0x03);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa2, 0x0b);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa3, 0x0f);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa4, 0x0b);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa5, 0x1b);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa6, 0x0a);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa7, 0x0a);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa8, 0x02);
	ilitek_ili9806e_dcs_write_seq(dev, 0xa9, 0x07);
	ilitek_ili9806e_dcs_write_seq(dev, 0xaa, 0x05);
	ilitek_ili9806e_dcs_write_seq(dev, 0xab, 0x03);
	ilitek_ili9806e_dcs_write_seq(dev, 0xac, 0x0e);
	ilitek_ili9806e_dcs_write_seq(dev, 0xad, 0x32);
	ilitek_ili9806e_dcs_write_seq(dev, 0xae, 0x2d);
	ilitek_ili9806e_dcs_write_seq(dev, 0xaf, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc0, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc1, 0x03);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc2, 0x0e);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc3, 0x10);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc4, 0x09);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc5, 0x17);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc6, 0x09);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc7, 0x07);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc8, 0x04);
	ilitek_ili9806e_dcs_write_seq(dev, 0xc9, 0x09);
	ilitek_ili9806e_dcs_write_seq(dev, 0xca, 0x06);
	ilitek_ili9806e_dcs_write_seq(dev, 0xcb, 0x06);
	ilitek_ili9806e_dcs_write_seq(dev, 0xcc, 0x0c);
	ilitek_ili9806e_dcs_write_seq(dev, 0xcd, 0x25);
	ilitek_ili9806e_dcs_write_seq(dev, 0xce, 0x20);
	ilitek_ili9806e_dcs_write_seq(dev, 0xcf, 0x00);

	/* Switch to page 6 */
	ilitek_ili9806e_dcs_write_seq(dev, 0xff, 0xff, 0x98, 0x06, 0x04, 0x06);
	/* GIP settings */
	ilitek_ili9806e_dcs_write_seq(dev, 0x00, 0x21);
	ilitek_ili9806e_dcs_write_seq(dev, 0x01, 0x09);
	ilitek_ili9806e_dcs_write_seq(dev, 0x02, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x03, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x04, 0x01);
	ilitek_ili9806e_dcs_write_seq(dev, 0x05, 0x01);
	ilitek_ili9806e_dcs_write_seq(dev, 0x06, 0x80);
	ilitek_ili9806e_dcs_write_seq(dev, 0x07, 0x05);
	ilitek_ili9806e_dcs_write_seq(dev, 0x08, 0x02);
	ilitek_ili9806e_dcs_write_seq(dev, 0x09, 0x80);
	ilitek_ili9806e_dcs_write_seq(dev, 0x0a, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x0b, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x0c, 0x0a);
	ilitek_ili9806e_dcs_write_seq(dev, 0x0d, 0x0a);
	ilitek_ili9806e_dcs_write_seq(dev, 0x0e, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x0f, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x10, 0xe0);
	ilitek_ili9806e_dcs_write_seq(dev, 0x11, 0xe4);
	ilitek_ili9806e_dcs_write_seq(dev, 0x12, 0x04);
	ilitek_ili9806e_dcs_write_seq(dev, 0x13, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x14, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x15, 0xc0);
	ilitek_ili9806e_dcs_write_seq(dev, 0x16, 0x08);
	ilitek_ili9806e_dcs_write_seq(dev, 0x17, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x18, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x19, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x1a, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x1b, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x1c, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x1d, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x20, 0x01);
	ilitek_ili9806e_dcs_write_seq(dev, 0x21, 0x23);
	ilitek_ili9806e_dcs_write_seq(dev, 0x22, 0x45);
	ilitek_ili9806e_dcs_write_seq(dev, 0x23, 0x67);
	ilitek_ili9806e_dcs_write_seq(dev, 0x24, 0x01);
	ilitek_ili9806e_dcs_write_seq(dev, 0x25, 0x23);
	ilitek_ili9806e_dcs_write_seq(dev, 0x26, 0x45);
	ilitek_ili9806e_dcs_write_seq(dev, 0x27, 0x67);
	ilitek_ili9806e_dcs_write_seq(dev, 0x30, 0x01);
	ilitek_ili9806e_dcs_write_seq(dev, 0x31, 0x11);
	ilitek_ili9806e_dcs_write_seq(dev, 0x32, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, 0x33, 0xee);
	ilitek_ili9806e_dcs_write_seq(dev, 0x34, 0xff);
	ilitek_ili9806e_dcs_write_seq(dev, 0x35, 0xbb);
	ilitek_ili9806e_dcs_write_seq(dev, 0x36, 0xca);
	ilitek_ili9806e_dcs_write_seq(dev, 0x37, 0xdd);
	ilitek_ili9806e_dcs_write_seq(dev, 0x38, 0xac);
	ilitek_ili9806e_dcs_write_seq(dev, 0x39, 0x76);
	ilitek_ili9806e_dcs_write_seq(dev, 0x3a, 0x67);
	ilitek_ili9806e_dcs_write_seq(dev, 0x3b, 0x22);
	ilitek_ili9806e_dcs_write_seq(dev, 0x3c, 0x22);
	ilitek_ili9806e_dcs_write_seq(dev, 0x3d, 0x22);
	ilitek_ili9806e_dcs_write_seq(dev, 0x3e, 0x22);
	ilitek_ili9806e_dcs_write_seq(dev, 0x3f, 0x22);
	ilitek_ili9806e_dcs_write_seq(dev, 0x40, 0x22);
	ilitek_ili9806e_dcs_write_seq(dev, 0x52, 0x10);
	ilitek_ili9806e_dcs_write_seq(dev, 0x53, 0x10);

	/* Switch to page 7 */
	ilitek_ili9806e_dcs_write_seq(dev, 0xff, 0xff, 0x98, 0x06, 0x04, 0x07);
	ilitek_ili9806e_dcs_write_seq(dev, 0x17, 0x22);
	ilitek_ili9806e_dcs_write_seq(dev, 0x02, 0x77);
	ilitek_ili9806e_dcs_write_seq(dev, 0xe1, 0x79);
	ilitek_ili9806e_dcs_write_seq(dev, 0xb3, 0x10);

	/* Switch to page 0 */
	ilitek_ili9806e_dcs_write_seq(dev, 0xff, 0xff, 0x98, 0x06, 0x04, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, MIPI_DCS_SET_ADDRESS_MODE, 0x00);
	ilitek_ili9806e_dcs_write_seq(dev, MIPI_DCS_EXIT_SLEEP_MODE);

	mdelay(120);

	ilitek_ili9806e_dcs_write_seq(dev, MIPI_DCS_SET_DISPLAY_ON);

	mdelay(120);
}

static const struct ilitek_ili9806e_desc rk050hr345_ct106a_desc = {
	.timing = {
		.pixelclock.typ = 27000000,
		.hactive.typ = 480,
		.hfront_porch.typ = 10,
		.hback_porch.typ = 10,
		.hsync_len.typ = 10,
		.vactive.typ = 854,
		.vfront_porch.typ = 10,
		.vback_porch.typ = 10,
		.vsync_len.typ = 10,
		.flags = DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW,
	},
	.init_sequence = rk050hr345_ct106a_init,
};

static const struct udevice_id ilitek_ili9806e_ids[] = {
	{
		.compatible = "rocktech,rk050hr345-ct106a",
		.data = (ulong)&rk050hr345_ct106a_desc,
	},
	{ }
};

U_BOOT_DRIVER(ilitek_ili9806e) = {
	.name		= "ilitek_ili9806e",
	.id		= UCLASS_PANEL,
	.of_match	= ilitek_ili9806e_ids,
	.ops		= &ilitek_ili9806e_ops,
	.of_to_plat	= ilitek_ili9806e_of_to_plat,
	.probe		= ilitek_ili9806e_probe,
	.priv_auto	= sizeof(struct ilitek_ili9806e_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
