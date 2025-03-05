// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/ofnode_graph.h>
#include <log.h>
#include <misc.h>
#include <mipi_display.h>
#include <mipi_dsi.h>
#include <backlight.h>
#include <video_bridge.h>
#include <panel.h>
#include <power/regulator.h>
#include <spi.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/gpio.h>

#define SSD2825_DEVICE_ID_REG			0xB0
#define SSD2825_RGB_INTERFACE_CTRL_REG_1	0xB1
#define SSD2825_RGB_INTERFACE_CTRL_REG_2	0xB2
#define SSD2825_RGB_INTERFACE_CTRL_REG_3	0xB3
#define SSD2825_RGB_INTERFACE_CTRL_REG_4	0xB4
#define SSD2825_RGB_INTERFACE_CTRL_REG_5	0xB5
#define SSD2825_RGB_INTERFACE_CTRL_REG_6	0xB6
#define   SSD2825_NON_BURST			BIT(2)
#define   SSD2825_BURST				BIT(3)
#define   SSD2825_PCKL_HIGH			BIT(13)
#define   SSD2825_HSYNC_HIGH			BIT(14)
#define   SSD2825_VSYNC_HIGH			BIT(15)
#define SSD2825_CONFIGURATION_REG		0xB7
#define   SSD2825_CONF_REG_HS			BIT(0)
#define   SSD2825_CONF_REG_CKE			BIT(1)
#define   SSD2825_CONF_REG_SLP			BIT(2)
#define   SSD2825_CONF_REG_VEN			BIT(3)
#define   SSD2825_CONF_REG_HCLK			BIT(4)
#define   SSD2825_CONF_REG_CSS			BIT(5)
#define   SSD2825_CONF_REG_DCS			BIT(6)
#define   SSD2825_CONF_REG_REN			BIT(7)
#define   SSD2825_CONF_REG_ECD			BIT(8)
#define   SSD2825_CONF_REG_EOT			BIT(9)
#define   SSD2825_CONF_REG_LPE			BIT(10)
#define SSD2825_VC_CTRL_REG			0xB8
#define SSD2825_PLL_CTRL_REG			0xB9
#define SSD2825_PLL_CONFIGURATION_REG		0xBA
#define SSD2825_CLOCK_CTRL_REG			0xBB
#define SSD2825_PACKET_SIZE_CTRL_REG_1		0xBC
#define SSD2825_PACKET_SIZE_CTRL_REG_2		0xBD
#define SSD2825_PACKET_SIZE_CTRL_REG_3		0xBE
#define SSD2825_PACKET_DROP_REG			0xBF
#define SSD2825_OPERATION_CTRL_REG		0xC0
#define SSD2825_MAX_RETURN_SIZE_REG		0xC1
#define SSD2825_RETURN_DATA_COUNT_REG		0xC2
#define SSD2825_ACK_RESPONSE_REG		0xC3
#define SSD2825_LINE_CTRL_REG			0xC4
#define SSD2825_INTERRUPT_CTRL_REG		0xC5
#define SSD2825_INTERRUPT_STATUS_REG		0xC6
#define SSD2825_ERROR_STATUS_REG		0xC7
#define SSD2825_DATA_FORMAT_REG			0xC8
#define SSD2825_DELAY_ADJ_REG_1			0xC9
#define SSD2825_DELAY_ADJ_REG_2			0xCA
#define SSD2825_DELAY_ADJ_REG_3			0xCB
#define SSD2825_DELAY_ADJ_REG_4			0xCC
#define SSD2825_DELAY_ADJ_REG_5			0xCD
#define SSD2825_DELAY_ADJ_REG_6			0xCE
#define SSD2825_HS_TX_TIMER_REG_1		0xCF
#define SSD2825_HS_TX_TIMER_REG_2		0xD0
#define SSD2825_LP_RX_TIMER_REG_1		0xD1
#define SSD2825_LP_RX_TIMER_REG_2		0xD2
#define SSD2825_TE_STATUS_REG			0xD3
#define SSD2825_SPI_READ_REG			0xD4
#define SSD2825_PLL_LOCK_REG			0xD5
#define SSD2825_TEST_REG			0xD6
#define SSD2825_TE_COUNT_REG			0xD7
#define SSD2825_ANALOG_CTRL_REG_1		0xD8
#define SSD2825_ANALOG_CTRL_REG_2		0xD9
#define SSD2825_ANALOG_CTRL_REG_3		0xDA
#define SSD2825_ANALOG_CTRL_REG_4		0xDB
#define SSD2825_INTERRUPT_OUT_CTRL_REG		0xDC
#define SSD2825_RGB_INTERFACE_CTRL_REG_7	0xDD
#define SSD2825_LANE_CONFIGURATION_REG		0xDE
#define SSD2825_DELAY_ADJ_REG_7			0xDF
#define SSD2825_INPUT_PIN_CTRL_REG_1		0xE0
#define SSD2825_INPUT_PIN_CTRL_REG_2		0xE1
#define SSD2825_BIDIR_PIN_CTRL_REG_1		0xE2
#define SSD2825_BIDIR_PIN_CTRL_REG_2		0xE3
#define SSD2825_BIDIR_PIN_CTRL_REG_3		0xE4
#define SSD2825_BIDIR_PIN_CTRL_REG_4		0xE5
#define SSD2825_BIDIR_PIN_CTRL_REG_5		0xE6
#define SSD2825_BIDIR_PIN_CTRL_REG_6		0xE7
#define SSD2825_BIDIR_PIN_CTRL_REG_7		0xE8
#define SSD2825_CABC_BRIGHTNESS_CTRL_REG_1	0xE9
#define SSD2825_CABC_BRIGHTNESS_CTRL_REG_2	0xEA
#define SSD2825_CABC_BRIGHTNESS_STATUS_REG	0xEB
#define SSD2825_READ_REG			0xFF
#define   SSD2825_SPI_READ_REG_RESET		0xFA

#define SSD2825_CMD_MASK		0x00
#define SSD2825_DAT_MASK		0x01

#define SSD2825_CMD_SEND		BIT(0)
#define SSD2825_DAT_SEND		BIT(1)
#define SSD2825_DSI_SEND		BIT(2)

#define SSD2828_LP_CLOCK_DIVIDER(n)	(((n) - 1) & 0x3F)
#define SSD2825_LP_MIN_CLK		5000 /* KHz */
#define SSD2825_REF_MIN_CLK		2000 /* KHz */

static const char * const ssd2825_supplies[] = {
	"dvdd-supply", "avdd-supply", "vddio-supply"
};

struct ssd2825_bridge_priv {
	struct mipi_dsi_host host;
	struct mipi_dsi_device device;

	struct udevice *panel;
	struct display_timing timing;

	struct udevice *supplies[ARRAY_SIZE(ssd2825_supplies)];

	struct gpio_desc power_gpio;

	struct clk *tx_clk;

	u32 pll_freq_kbps;	/* PLL in kbps */

	u32 hzd;		/* HS Zero Delay in ns */
	u32 hpd;		/* HS Prepare Delay is ns */
};

static int ssd2825_spi_write(struct udevice *dev, int reg,
			     const void *buf, int flags)
{
	u8 command[2];

	if (flags & SSD2825_CMD_SEND) {
		command[0] = SSD2825_CMD_MASK;
		command[1] = reg;
		dm_spi_xfer(dev, 9, &command,
			    NULL, SPI_XFER_ONCE);
	}

	if (flags & SSD2825_DAT_SEND) {
		u16 data = *(u16 *)buf;
		u8 cmd1, cmd2;

		/* send low byte first and then high byte */
		cmd1 = (data & 0x00FF);
		cmd2 = (data & 0xFF00) >> 8;

		command[0] = SSD2825_DAT_MASK;
		command[1] = cmd1;
		dm_spi_xfer(dev, 9, &command,
			    NULL, SPI_XFER_ONCE);

		command[0] = SSD2825_DAT_MASK;
		command[1] = cmd2;
		dm_spi_xfer(dev, 9, &command,
			    NULL, SPI_XFER_ONCE);
	}

	if (flags & SSD2825_DSI_SEND) {
		u16 data = *(u16 *)buf;
		data &= 0x00FF;

		debug("%s: dsi command (0x%x)\n",
		      __func__, data);

		command[0] = SSD2825_DAT_MASK;
		command[1] = data;
		dm_spi_xfer(dev, 9, &command,
			    NULL, SPI_XFER_ONCE);
	}

	return 0;
}

static int ssd2825_spi_read(struct udevice *dev, int reg,
			    void *data, int flags)
{
	u8 command[2];

	command[0] = SSD2825_CMD_MASK;
	command[1] = SSD2825_SPI_READ_REG;
	dm_spi_xfer(dev, 9, &command,
		    NULL, SPI_XFER_ONCE);

	command[0] = SSD2825_DAT_MASK;
	command[1] = SSD2825_SPI_READ_REG_RESET;
	dm_spi_xfer(dev, 9, &command,
		    NULL, SPI_XFER_ONCE);

	command[0] = SSD2825_DAT_MASK;
	command[1] = 0;
	dm_spi_xfer(dev, 9, &command,
		    NULL, SPI_XFER_ONCE);

	command[0] = SSD2825_CMD_MASK;
	command[1] = reg;
	dm_spi_xfer(dev, 9, &command,
		    NULL, SPI_XFER_ONCE);

	command[0] = SSD2825_CMD_MASK;
	command[1] = SSD2825_SPI_READ_REG_RESET;
	dm_spi_xfer(dev, 9, &command,
		    NULL, SPI_XFER_ONCE);

	dm_spi_xfer(dev, 16, NULL,
		    (u8 *)data, SPI_XFER_ONCE);

	return 0;
}

static void ssd2825_write_register(struct udevice *dev, u8 reg,
				   u16 command)
{
	ssd2825_spi_write(dev, reg, &command,
			  SSD2825_CMD_SEND |
			  SSD2825_DAT_SEND);
}

static void ssd2825_write_dsi(struct udevice *dev, const u8 *command,
			      int len)
{
	int i;

	ssd2825_spi_write(dev, SSD2825_PACKET_SIZE_CTRL_REG_1, &len,
			  SSD2825_CMD_SEND | SSD2825_DAT_SEND);

	ssd2825_spi_write(dev, SSD2825_PACKET_DROP_REG, NULL,
			  SSD2825_CMD_SEND);

	for (i = 0; i < len; i++)
		ssd2825_spi_write(dev, 0, &command[i], SSD2825_DSI_SEND);
}

static ssize_t ssd2825_bridge_transfer(struct mipi_dsi_host *host,
				       const struct mipi_dsi_msg *msg)
{
	struct udevice *dev = (struct udevice *)host->dev;
	u16 config;
	int ret;

	ret = ssd2825_spi_read(dev, SSD2825_CONFIGURATION_REG,
			       &config, 0);
	if (ret)
		return ret;

	switch (msg->type) {
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_DCS_LONG_WRITE:
		config |= SSD2825_CONF_REG_DCS;
		break;
	case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
	case MIPI_DSI_GENERIC_LONG_WRITE:
		config &= ~SSD2825_CONF_REG_DCS;
		break;
	default:
		return 0;
	}

	ssd2825_write_register(dev, SSD2825_CONFIGURATION_REG, config);
	ssd2825_write_register(dev, SSD2825_VC_CTRL_REG, 0x0000);
	ssd2825_write_dsi(dev, msg->tx_buf, msg->tx_len);

	return 0;
}

static const struct mipi_dsi_host_ops ssd2825_bridge_host_ops = {
	.transfer	= ssd2825_bridge_transfer,
};

/*
 * PLL configuration register settings.
 *
 * See the "PLL Configuration Register Description" in the SSD2825 datasheet.
 */
static u16 construct_pll_config(struct ssd2825_bridge_priv *priv,
				u32 desired_pll_freq_kbps, u32 reference_freq_khz)
{
	u32 div_factor = 1, mul_factor, fr = 0;

	while (reference_freq_khz / (div_factor + 1) >= SSD2825_REF_MIN_CLK)
		div_factor++;
	if (div_factor > 31)
		div_factor = 31;

	mul_factor = DIV_ROUND_UP(desired_pll_freq_kbps * div_factor,
				  reference_freq_khz);

	priv->pll_freq_kbps = reference_freq_khz * mul_factor / div_factor;

	if (priv->pll_freq_kbps >= 501000)
		fr = 3;
	else if (priv->pll_freq_kbps >= 251000)
		fr = 2;
	else if (priv->pll_freq_kbps >= 126000)
		fr = 1;

	return (fr << 14) | (div_factor << 8) | mul_factor;
}

static void ssd2825_setup_pll(struct udevice *dev)
{
	struct ssd2825_bridge_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct display_timing *dt = &priv->timing;
	u16 pll_config, lp_div;
	u32 nibble_delay, nibble_freq_khz;
	u32 pclk_mult, tx_freq_khz, pd_lines;
	u8 hzd, hpd;

	tx_freq_khz = clk_get_rate(priv->tx_clk) / 1000;
	if (!tx_freq_khz || tx_freq_khz < 0)
		tx_freq_khz = SSD2825_REF_MIN_CLK;

	pd_lines = mipi_dsi_pixel_format_to_bpp(device->format);
	pclk_mult = pd_lines / device->lanes + 1;

	pll_config = construct_pll_config(priv, pclk_mult *
					  dt->pixelclock.typ / 1000,
					  tx_freq_khz);

	lp_div = priv->pll_freq_kbps / (SSD2825_LP_MIN_CLK * 8);

	/* nibble_delay in nanoseconds */
	nibble_freq_khz = priv->pll_freq_kbps / 4;
	nibble_delay = 1000 * 1000 / nibble_freq_khz;

	hzd = priv->hzd / nibble_delay;
	hpd = (priv->hpd - 4 * nibble_delay) / nibble_delay;

	/* Disable PLL */
	ssd2825_write_register(dev, SSD2825_PLL_CTRL_REG, 0x0000);
	ssd2825_write_register(dev, SSD2825_LINE_CTRL_REG, 0x0001);

	/* Set delays */
	ssd2825_write_register(dev, SSD2825_DELAY_ADJ_REG_1, (hzd << 8) | hpd);

	/* Set PLL coeficients */
	ssd2825_write_register(dev, SSD2825_PLL_CONFIGURATION_REG, pll_config);

	/* Clock Control Register */
	ssd2825_write_register(dev, SSD2825_CLOCK_CTRL_REG,
			       SSD2828_LP_CLOCK_DIVIDER(lp_div));

	/* Enable PLL */
	ssd2825_write_register(dev, SSD2825_PLL_CTRL_REG, 0x0001);
	ssd2825_write_register(dev, SSD2825_VC_CTRL_REG, 0x0000);
}

static int ssd2825_bridge_attach(struct udevice *dev)
{
	struct ssd2825_bridge_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct display_timing *dt = &priv->timing;
	u8 pixel_format;
	int ret;

	/* Set pixel format */
	switch (device->format) {
	case MIPI_DSI_FMT_RGB565:
		pixel_format = 0x00;
		break;
	case MIPI_DSI_FMT_RGB666_PACKED:
		pixel_format = 0x01;
		break;
	case MIPI_DSI_FMT_RGB666:
		pixel_format = 0x02;
		break;
	case MIPI_DSI_FMT_RGB888:
	default:
		pixel_format = 0x03;
		break;
	}

	/* Perform SW reset */
	ssd2825_write_register(dev, SSD2825_OPERATION_CTRL_REG, 0x0100);

	/* Set panel timings */
	ssd2825_write_register(dev, SSD2825_RGB_INTERFACE_CTRL_REG_1,
			       dt->vsync_len.typ << 8 | dt->hsync_len.typ);
	ssd2825_write_register(dev, SSD2825_RGB_INTERFACE_CTRL_REG_2,
			       (dt->vsync_len.typ + dt->vback_porch.typ) << 8 |
			       (dt->hsync_len.typ + dt->hback_porch.typ));
	ssd2825_write_register(dev, SSD2825_RGB_INTERFACE_CTRL_REG_3,
			       dt->vfront_porch.typ << 8 | dt->hfront_porch.typ);
	ssd2825_write_register(dev, SSD2825_RGB_INTERFACE_CTRL_REG_4,
			       dt->hactive.typ);
	ssd2825_write_register(dev, SSD2825_RGB_INTERFACE_CTRL_REG_5,
			       dt->vactive.typ);
	ssd2825_write_register(dev, SSD2825_RGB_INTERFACE_CTRL_REG_6,
			       SSD2825_HSYNC_HIGH | SSD2825_VSYNC_HIGH |
			       SSD2825_PCKL_HIGH | SSD2825_NON_BURST |
			       pixel_format);
	ssd2825_write_register(dev, SSD2825_LANE_CONFIGURATION_REG,
			       device->lanes - 1);
	ssd2825_write_register(dev, SSD2825_TEST_REG, 0x0004);

	/* Call PLL configuration */
	ssd2825_setup_pll(dev);

	mdelay(10);

	/* Initial DSI configuration register set */
	ssd2825_write_register(dev, SSD2825_CONFIGURATION_REG,
			       SSD2825_CONF_REG_CKE | SSD2825_CONF_REG_DCS |
			       SSD2825_CONF_REG_ECD | SSD2825_CONF_REG_EOT);
	ssd2825_write_register(dev, SSD2825_VC_CTRL_REG, 0x0000);

	/* Perform panel setup */
	ret = panel_enable_backlight(priv->panel);
	if (ret)
		return ret;

	ssd2825_write_register(dev, SSD2825_CONFIGURATION_REG,
			       SSD2825_CONF_REG_HS | SSD2825_CONF_REG_VEN |
			       SSD2825_CONF_REG_DCS | SSD2825_CONF_REG_ECD |
			       SSD2825_CONF_REG_EOT);
	ssd2825_write_register(dev, SSD2825_PLL_CTRL_REG, 0x0001);
	ssd2825_write_register(dev, SSD2825_VC_CTRL_REG, 0x0000);

	return 0;
}

static int ssd2825_bridge_set_panel(struct udevice *dev, int percent)
{
	struct ssd2825_bridge_priv *priv = dev_get_priv(dev);

	return panel_set_backlight(priv->panel, percent);
}

static int ssd2825_bridge_panel_timings(struct udevice *dev,
					struct display_timing *timing)
{
	struct ssd2825_bridge_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));

	return 0;
}

static int ssd2825_bridge_hw_init(struct udevice *dev)
{
	struct ssd2825_bridge_priv *priv = dev_get_priv(dev);
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);
	int i, ret;

	ret = clk_prepare_enable(priv->tx_clk);
	if (ret) {
		log_debug("%s: error enabling tx_clk (%d)\n",
			  __func__, ret);
		return ret;
	}

	/* enable supplies */
	for (i = 0; i < ARRAY_SIZE(ssd2825_supplies); i++) {
		ret = regulator_set_enable_if_allowed(priv->supplies[i], 1);
		if (ret) {
			log_debug("%s: cannot enable %s %d\n", __func__,
				  ssd2825_supplies[i], ret);
			return ret;
		}
	}
	mdelay(10);

	ret = dm_gpio_set_value(&uc_priv->reset, 1);
	if (ret) {
		log_debug("%s: error entering reset (%d)\n",
			  __func__, ret);
		return ret;
	}
	mdelay(10);

	ret = dm_gpio_set_value(&uc_priv->reset, 0);
	if (ret) {
		log_debug("%s: error exiting reset (%d)\n",
			  __func__, ret);
		return ret;
	}
	mdelay(10);

	return 0;
}

static int ssd2825_bridge_get_panel(struct udevice *dev)
{
	struct ssd2825_bridge_priv *priv = dev_get_priv(dev);
	int i, ret;

	u32 num = ofnode_graph_get_port_count(dev_ofnode(dev));

	for (i = 0; i < num; i++) {
		ofnode remote = ofnode_graph_get_remote_node(dev_ofnode(dev), i, -1);

		ret = uclass_get_device_by_ofnode(UCLASS_PANEL, remote,
						  &priv->panel);
		if (!ret)
			return 0;
	}

	/* If this point is reached, no panels were found */
	return -ENODEV;
}

static int ssd2825_bridge_probe(struct udevice *dev)
{
	struct ssd2825_bridge_priv *priv = dev_get_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct mipi_dsi_panel_plat *mipi_plat;
	int i, ret;

	ret = spi_claim_bus(slave);
	if (ret) {
		log_err("SPI bus allocation failed (%d)\n", ret);
		return ret;
	}

	ret = ssd2825_bridge_get_panel(dev);
	if (ret) {
		log_debug("%s: panel not found, ret %d\n", __func__, ret);
		return ret;
	}

	panel_get_display_timing(priv->panel, &priv->timing);

	mipi_plat = dev_get_plat(priv->panel);
	mipi_plat->device = device;

	priv->host.dev = (struct device *)dev;
	priv->host.ops = &ssd2825_bridge_host_ops;

	device->host = &priv->host;
	device->lanes = mipi_plat->lanes;
	device->format = mipi_plat->format;
	device->mode_flags = mipi_plat->mode_flags;

	/* get supplies */
	for (i = 0; i < ARRAY_SIZE(ssd2825_supplies); i++) {
		ret = device_get_supply_regulator(dev, ssd2825_supplies[i],
						  &priv->supplies[i]);
		if (ret) {
			log_debug("%s: cannot get %s %d\n", __func__,
				  ssd2825_supplies[i], ret);
			if (ret != -ENOENT)
				return log_ret(ret);
		}
	}

	/* get clk */
	priv->tx_clk = devm_clk_get_optional(dev, NULL);
	if (IS_ERR(priv->tx_clk)) {
		log_err("cannot get tx_clk: %ld\n", PTR_ERR(priv->tx_clk));
		return PTR_ERR(priv->tx_clk);
	}

	priv->hzd = dev_read_u32_default(dev, "solomon,hs-zero-delay-ns", 133);
	priv->hpd = dev_read_u32_default(dev, "solomon,hs-prep-delay-ns", 40);

	return ssd2825_bridge_hw_init(dev);
}

static const struct video_bridge_ops ssd2825_bridge_ops = {
	.attach			= ssd2825_bridge_attach,
	.set_backlight		= ssd2825_bridge_set_panel,
	.get_display_timing	= ssd2825_bridge_panel_timings,
};

static const struct udevice_id ssd2825_bridge_ids[] = {
	{ .compatible = "solomon,ssd2825" },
	{ }
};

U_BOOT_DRIVER(ssd2825) = {
	.name		= "ssd2825",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= ssd2825_bridge_ids,
	.ops		= &ssd2825_bridge_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= ssd2825_bridge_probe,
	.priv_auto	= sizeof(struct ssd2825_bridge_priv),
};
