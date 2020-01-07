// SPDX-License-Identifier: GPL-2.0
/*
 * LCD: LG4573, TFT 4.3", 480x800, RGB24
 * LCD initialization via SPI
 *
 */
#include <common.h>
#include <backlight.h>
#include <display.h>
#include <dm.h>
#include <dm/read.h>
#include <dm/uclass-internal.h>
#include <errno.h>
#include <spi.h>
#include <asm/gpio.h>

#define PWR_ON_DELAY_MSECS  120

static int lb043wv_spi_write_u16(struct spi_slave *slave, u16 val)
{
	unsigned short buf16 = htons(val);
	int ret = 0;

	ret = spi_xfer(slave, 16, &buf16, NULL,
		       SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret)
		debug("%s: Failed to send: %d\n", __func__, ret);

	return ret;
}

static void lb043wv_spi_write_u16_array(struct spi_slave *slave, u16 *buff,
					int size)
{
	int i;

	for (i = 0; i < size; i++)
		lb043wv_spi_write_u16(slave, buff[i]);
}

static void lb043wv_display_mode_settings(struct spi_slave *slave)
{
	static u16 display_mode_settings[] = {
	  0x703A,
	  0x7270,
	  0x70B1,
	  0x7208,
	  0x723B,
	  0x720F,
	  0x70B2,
	  0x7200,
	  0x72C8,
	  0x70B3,
	  0x7200,
	  0x70B4,
	  0x7200,
	  0x70B5,
	  0x7242,
	  0x7210,
	  0x7210,
	  0x7200,
	  0x7220,
	  0x70B6,
	  0x720B,
	  0x720F,
	  0x723C,
	  0x7213,
	  0x7213,
	  0x72E8,
	  0x70B7,
	  0x7246,
	  0x7206,
	  0x720C,
	  0x7200,
	  0x7200,
	};

	debug("transfer display mode settings\n");
	lb043wv_spi_write_u16_array(slave, display_mode_settings,
				    ARRAY_SIZE(display_mode_settings));
}

static void lb043wv_power_settings(struct spi_slave *slave)
{
	static u16 power_settings[] = {
	  0x70C0,
	  0x7201,
	  0x7211,
	  0x70C3,
	  0x7207,
	  0x7203,
	  0x7204,
	  0x7204,
	  0x7204,
	  0x70C4,
	  0x7212,
	  0x7224,
	  0x7218,
	  0x7218,
	  0x7202,
	  0x7249,
	  0x70C5,
	  0x726F,
	  0x70C6,
	  0x7241,
	  0x7263,
	};

	debug("transfer power settings\n");
	lb043wv_spi_write_u16_array(slave, power_settings,
				    ARRAY_SIZE(power_settings));
}

static void lb043wv_gamma_settings(struct spi_slave *slave)
{
	static u16 gamma_settings[] = {
	  0x70D0,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D1,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D2,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D3,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D4,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D5,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	};

	debug("transfer gamma settings\n");
	lb043wv_spi_write_u16_array(slave, gamma_settings,
				    ARRAY_SIZE(gamma_settings));
}

static void lb043wv_display_on(struct spi_slave *slave)
{
	static u16 sleep_out = 0x7011;
	static u16 display_on = 0x7029;

	lb043wv_spi_write_u16(slave, sleep_out);
	mdelay(PWR_ON_DELAY_MSECS);
	lb043wv_spi_write_u16(slave, display_on);
}

static int lg4573_spi_startup(struct spi_slave *slave)
{
	int ret;

	ret = spi_claim_bus(slave);
	if (ret)
		return ret;

	lb043wv_display_mode_settings(slave);
	lb043wv_power_settings(slave);
	lb043wv_gamma_settings(slave);
	lb043wv_display_on(slave);

	spi_release_bus(slave);
	return 0;
}

static int do_lgset(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	struct spi_slave *slave;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_DISPLAY,
					  DM_GET_DRIVER(lg4573_lcd), &dev);
	if (ret) {
		printf("%s: Could not get lg4573 device\n", __func__);
		return ret;
	}
	slave = dev_get_parent_priv(dev);
	if (!slave) {
		printf("%s: No slave data\n", __func__);
		return -ENODEV;
	}
	lg4573_spi_startup(slave);

	return 0;
}

U_BOOT_CMD(
	lgset,	2,	1,	do_lgset,
	"set lgdisplay",
	""
);

static int lg4573_bind(struct udevice *dev)
{
	return 0;
}

static int lg4573_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id lg4573_ids[] = {
	{ .compatible = "lg,lg4573" },
	{ }
};

struct lg4573_lcd_priv {
	struct display_timing timing;
	struct udevice *backlight;
	struct gpio_desc enable;
	int panel_bpp;
	u32 power_on_delay;
};

static int lg4573_lcd_read_timing(struct udevice *dev,
				  struct display_timing *timing)
{
	struct lg4573_lcd_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(struct display_timing));

	return 0;
}

static int lg4573_lcd_enable(struct udevice *dev, int bpp,
			     const struct display_timing *edid)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct lg4573_lcd_priv *priv = dev_get_priv(dev);
	int ret = 0;

	dm_gpio_set_value(&priv->enable, 1);
	ret = backlight_enable(priv->backlight);

	mdelay(priv->power_on_delay);
	lg4573_spi_startup(slave);

	return ret;
};

static const struct dm_display_ops lg4573_lcd_ops = {
	.read_timing = lg4573_lcd_read_timing,
	.enable = lg4573_lcd_enable,
};

static int lg4573_ofdata_to_platdata(struct udevice *dev)
{
	struct lg4573_lcd_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		debug("%s: Cannot get backlight: ret=%d\n", __func__, ret);
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

	priv->power_on_delay = dev_read_u32_default(dev, "power-on-delay", 10);

	return 0;
}

U_BOOT_DRIVER(lg4573_lcd) = {
	.name   = "lg4573",
	.id     = UCLASS_DISPLAY,
	.ops    = &lg4573_lcd_ops,
	.ofdata_to_platdata	= lg4573_ofdata_to_platdata,
	.of_match = lg4573_ids,
	.bind   = lg4573_bind,
	.probe  = lg4573_probe,
	.priv_auto_alloc_size = sizeof(struct lg4573_lcd_priv),
};
