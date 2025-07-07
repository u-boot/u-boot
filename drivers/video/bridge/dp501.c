// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024 Jonas Schwöbel <jonasschwoebel@yahoo.de>
 * Copyright (C) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <backlight.h>
#include <panel.h>
#include <video_bridge.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <power/regulator.h>

#include <asm/gpio.h>

/* TOP */
#define TOPCFG0			0x00
#define ROMI2C_PRESCALE		0x01
#define HDCPI2C_PRESCALE	0x02
#define GPIO			0x03
#define GPIO_OUT_ENB		0x04
#define TESTI2C_CTL		0x05
#define I2CMTIMEOUT		0x06
#define TOPCFG1			0x07
#define TOPCFG2			0x08
#define TOPCFG3			0x09
#define TOPCFG4			0x0A
#define CLKSWRST		0x0B
#define CADETB_CTL		0x0C

/* Video Attribute */
#define HTOTAL_L		0x10
#define HTOTAL_H		0x11
#define HSTART_L		0x12
#define HSTART_H		0x13
#define HWIDTH_L		0x14
#define HWIDTH_H		0x15
#define VTOTAL_L		0x16
#define VTOTAL_H		0x17
#define VSTART_L		0x18
#define VSTART_H		0x19
#define VHEIGHT_L		0x1A
#define VHEIGHT_H		0x1B
#define HSPHSW_L		0x1C
#define HSPHSW_H		0x1D
#define VSPVSW_L		0x1E
#define VSPVSW_H		0x1F
#define MISC0			0x20
#define MISC1			0x21

/* Video Capture */
#define VCAPCTRL0		0x24
#define VCAPCTRL1		0x25
#define VCAPCTRL2		0x26
#define VCAPCTRL3		0x27
#define VCAPCTRL4		0x28
#define VCAP_MEASURE		0x29

/* Main Link Control */
#define NVID_L			0x2C
#define NVID_M			0x2D
#define NVID_H			0x2E
#define LINK_CTRL0		0x2F
#define LINK_CTRL1		0x30
#define LINK_DEBUG		0x31
#define ERR_POS			0x32
#define ERR_PAT			0x33
#define LINK_DEB_SEL		0x34
#define IDLE_PATTERN		0x35
#define TU_SIZE			0x36
#define CRC_CTRL		0x37
#define CRC_OUT			0x38

/* AVI-2 InfoFrame */
#define SD_CTRL0		0x3A
#define SD_CTRL1		0x3B
#define SD_HB0			0x3C
#define SD_HB1			0x3D
#define SD_HB2			0x3E
#define SD_HB3			0x3F
#define SD_DB0			0x40
#define SD_DB1			0x41
#define SD_DB2			0x42
#define SD_DB3			0x43
#define SD_DB4			0x44
#define SD_DB5			0x45
#define SD_DB6			0x46
#define SD_DB7			0x47
#define SD_DB8			0x48
#define SD_DB9			0x49
#define SD_DB10			0x4A
#define SD_DB11			0x4B
#define SD_DB12			0x4C
#define SD_DB13			0x4D
#define SD_DB14			0x4E
#define SD_DB15			0x4F

/* Aux Channel and PCS */
#define DPCD_REV		0x50
#define MAX_LINK_RATE		0x51
#define MAX_LANE_COUNT		0x52
#define MAX_DOWNSPREAD		0x53
#define NORP			0x54
#define DOWNSTRMPORT_PRE	0x55
#define MLINK_CH_CODING		0x56
#define RCV_P0_CAP0		0x58
#define RCV_P0_CAP1		0x59
#define RCV_P1_CAP0		0x5A
#define RCV_P1_CAP1		0x5B
#define DOWNSPREAD_CTL		0x5C
#define LINK_BW			0x5D
#define LANE_CNT		0x5E
#define TRAINING_CTL		0x5F
#define QUALTEST_CTL		0x60
#define SINK_COUNT		0x61
#define DEV_SERVICE_IRQ		0x62
#define LANE01_STATUS		0x63
#define LANE23_STATUS		0x64
#define LANE_STATUS_UPDATE	0x65
#define SINK_STATUS		0x66
#define AUX_NOISE		0x67
#define TEST_MODE		0x69
#define TEST_PATTERN0		0x6A
#define TEST_PATTERN1		0x6B
#define TEST_PATTERN2		0x6C
#define SIGNATURE		0x6D
#define PCSCFG			0x6E
#define AUXCTRL0		0x6f
#define AUXCTRL2		0x70
#define AUXCTRL1		0x71
#define HPDCTL0			0x72
#define HPDCTL1			0x73
#define LINK_STATE_CTRL		0x74
#define SWRST			0x75
#define LINK_IRQ		0x76
#define AUXIRQ_CTRL		0x77
#define HPD2_IRQ_CTRL		0x78
#define SW_TRAIN_CTRL		0x79
#define SW_DRV_SET		0x7A
#define SW_PRE_SET		0x7B
#define DPCD_ADDR_L		0x7D
#define DPCD_ADDR_M		0x7E
#define DPCD_ADDR_H		0x7F
#define DPCD_LENGTH		0x80
#define DPCD_WDATA		0x81
#define DPCD_RDATA		0x82
#define DPCD_CTL		0x83
#define DPCD_STATUS		0x84
#define AUX_STATUS		0x85
#define I2CTOAUX_RELENGTH	0x86
#define AUX_RETRY_CTRL		0x87
#define TIMEOUT_CTRL		0x88
#define I2CCMD_OPT1		0x89
#define AUXCMD_ERR_IRQ		0x8A
#define AUXCMD_OPT2		0x8B
#define HDCP_Reserved		0x8C

/* Audio InfoFrame */
#define TX_MVID0		0x90
#define TX_MVID1		0x91
#define TX_MVID2		0x92
#define TX_MVID_OFF		0x93
#define TX_MAUD0		0x94
#define TX_MAUD1		0x95
#define TX_MAUD2		0x96
#define TX_MAUD_OFF		0x97
#define MN_CTRL			0x98
#define MOUT0			0x99
#define MOUT1			0x9A
#define MOUT2			0x9B

/* Audio Control */
#define NAUD_L			0x9F
#define NAUD_M			0xA0
#define NAUD_H			0xA1
#define AUD_CTRL0		0xA2
#define AUD_CTRL1		0xA3
#define LANE_POL		0xAA
#define LANE_EN			0xAB
#define LANE_MAP		0xAC
#define SCR_POLY0		0xAD
#define SCR_POLY1		0xAE
#define PRBS7_POLY		0xAF

/* Video Pre-process */
#define MISC_SHDOW		0xB0
#define VCAPCPCTL0		0xB1
#define VCAPCPCTL1		0xB2
#define VCAPCPCTL2		0xB3
#define CSCPAR			0xB4
#define I2CTODPCDSTATUS2	0xBA
#define AUXCTL_REG		0xBB

/*   Page 2   */
#define SEL_PIO1		0x24
#define SEL_PIO2		0x25
#define SEL_PIO3		0x26
#define CHIP_VER_L		0x82

struct dp501_priv {
	struct udevice *panel;
	struct display_timing timing;

	struct udevice *chip2;

	struct udevice *vdd;
	struct gpio_desc enable_gpio;
};

static int dp501_sw_init(struct udevice *dev)
{
	struct dp501_priv *priv = dev_get_priv(dev);
	int i;
	u8 val;

	dm_i2c_reg_write(dev, TOPCFG4, 0x30);
	udelay(200);
	dm_i2c_reg_write(dev, TOPCFG4, 0x0c);
	dm_i2c_reg_write(dev, 0x8f, 0x02);

	/* check for connected panel during 1 msec */
	for (i = 0; i < 5; i++)	{
		val = dm_i2c_reg_read(dev, 0x8d);
		val &= BIT(2);
		if (val)
			break;

		udelay(200);
	}

	if (!val) {
		log_debug("%s: panel is not connected!\n", __func__);
		return -ENODEV;
	}

	dm_i2c_reg_write(priv->chip2, SEL_PIO1, 0x02);
	dm_i2c_reg_write(priv->chip2, SEL_PIO2, 0x04);
	dm_i2c_reg_write(priv->chip2, SEL_PIO3, 0x10);

	dm_i2c_reg_write(dev, LINK_STATE_CTRL, 0xa0);
	dm_i2c_reg_write(dev, 0x8f, 0x02);
	dm_i2c_reg_write(dev, TOPCFG1, 0x16);
	dm_i2c_reg_write(dev, TOPCFG0, 0x24);
	dm_i2c_reg_write(dev, HPD2_IRQ_CTRL, 0x30);
	dm_i2c_reg_write(dev, AUXIRQ_CTRL, 0xff);
	dm_i2c_reg_write(dev, LINK_IRQ, 0xff);

	/* auto detect DVO timing */
	dm_i2c_reg_write(dev, VCAPCTRL3, 0x30);

	/* reset tpfifo at v blank */
	dm_i2c_reg_write(dev, LINK_CTRL0, 0x82);

	dm_i2c_reg_write(dev, VCAPCTRL4, 0x07);
	dm_i2c_reg_write(dev, AUX_RETRY_CTRL, 0x7f);
	dm_i2c_reg_write(dev, TIMEOUT_CTRL, 0x1e);
	dm_i2c_reg_write(dev, AUXCTL_REG, 0x06);

	/* DPCD readable */
	dm_i2c_reg_write(dev, HPDCTL0, 0xa9);

	/* Scramble on */
	dm_i2c_reg_write(dev, QUALTEST_CTL, 0x00);

	dm_i2c_reg_write(dev, 0x8f, 0x02);

	dm_i2c_reg_write(dev, VCAPCTRL0, 0xc4);

	/* set color depth 8bit (0x00: 6bit; 0x20: 8bit; 0x40: 10bit) */
	dm_i2c_reg_write(dev, MISC0, 0x20);

	dm_i2c_reg_write(dev, VCAPCPCTL2, 0x01);

	/* check if bridge returns ready status */
	for (i = 0; i < 5; i++)	{
		val = dm_i2c_reg_read(dev, LINK_IRQ);
		val &= BIT(0);
		if (val)
			break;

		udelay(200);
	}

	if (!val) {
		log_debug("%s: bridge is not ready\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static void dpcd_configure(struct udevice *dev, u32 config, bool write)
{
	dm_i2c_reg_write(dev, DPCD_ADDR_L, (u8)(config >> 8));
	dm_i2c_reg_write(dev, DPCD_ADDR_M, (u8)(config >> 16));
	dm_i2c_reg_write(dev, DPCD_ADDR_H, (u8)((config >> 24) | BIT(7)));
	dm_i2c_reg_write(dev, DPCD_LENGTH, 0x00);
	dm_i2c_reg_write(dev, LINK_IRQ, 0x20);

	if (write)
		dm_i2c_reg_write(dev, DPCD_WDATA, (u8)(config & 0xff));

	dm_i2c_reg_write(dev, DPCD_CTL, 0x01);

	udelay(10);
}

static int dump_dpcd_data(struct udevice *dev, u32 config, u8 *data)
{
	int i;
	u8 value;

	dpcd_configure(dev, config, false);

	value = dm_i2c_reg_read(dev, DPCD_CTL);
	if (value)
		return -ENODATA;

	for (i = 0; i < 5; i++) {
		value = dm_i2c_reg_read(dev, LINK_IRQ);
		value &= BIT(5);
		if (value)
			break;

		udelay(100);
	}

	if (!value)
		return -ENODATA;

	value = dm_i2c_reg_read(dev, DPCD_STATUS);
	if (!(value & 0xe0))
		*data = dm_i2c_reg_read(dev, DPCD_RDATA);
	else
		return -ENODATA;

	return 0;
}

static int dp501_dpcd_dump(struct udevice *dev, u32 config, u8 *data)
{
	int i, ret;

	for (i = 0; i < 5; i++) {
		ret = dump_dpcd_data(dev, config, data);
		if (!ret)
			break;

		udelay(100);
	}

	return ret;
}

static int dp501_reset_link(struct udevice *dev)
{
	dm_i2c_reg_write(dev, TRAINING_CTL, 0x00);
	dm_i2c_reg_write(dev, SWRST, 0xf8);
	dm_i2c_reg_write(dev, SWRST, 0x00);

	return -ENODEV;
}

static int dp501_link_training(struct udevice *dev)
{
	int i, ret;
	u8 lane, link, link_out;
	u8 lane_cnt, lane01, lane23;

	dpcd_configure(dev, 0x030000, true);
	dpcd_configure(dev, 0x03011c, true);
	dpcd_configure(dev, 0x0301f8, true);

	ret = dp501_dpcd_dump(dev, 0x90000100, &link);
	if (ret) {
		log_debug("%s: link dump failed %d\n", __func__, ret);
		return dp501_reset_link(dev);
	}

	ret = dp501_dpcd_dump(dev, 0x90000200, &lane);
	if (ret) {
		log_debug("%s: lane dump failed %d\n", __func__, ret);
		return dp501_reset_link(dev);
	}

	/* Software trainig */
	for (i = 10; i > 0; i--) {
		dm_i2c_reg_write(dev, LINK_BW, link);
		dm_i2c_reg_write(dev, LANE_CNT, lane | BIT(7));

		link_out = dm_i2c_reg_read(dev, LINK_BW);
		lane_cnt = dm_i2c_reg_read(dev, LANE_CNT);

		if (link_out == link &&
		    (lane_cnt == (lane | BIT(7))))
			break;

		udelay(500);
	}

	if (!i)
		return dp501_reset_link(dev);

	dm_i2c_reg_write(dev, LINK_STATE_CTRL, 0x00);
	dm_i2c_reg_write(dev, TRAINING_CTL, 0x0d);

	/* check if bridge returns link ready status */
	for (i = 0; i < 100; i++) {
		link_out = dm_i2c_reg_read(dev, LINK_IRQ);
		link_out &= BIT(1);
		if (link_out) {
			dm_i2c_reg_write(dev, LINK_IRQ, 0xff);
			break;
		}

		udelay(100);
	}

	if (!link_out) {
		log_debug("%s: link prepare failed %d\n",
			  __func__, link_out);
		return dp501_reset_link(dev);
	}

	lane01 = dm_i2c_reg_read(dev, LANE01_STATUS);
	lane23 = dm_i2c_reg_read(dev, LANE23_STATUS);

	switch (lane_cnt & 0xf) {
	case 4:
		if (lane01 == 0x77 &&
		    lane23 == 0x77)
			return 0;
		break;

	case 2:
		if (lane01 == 0x77)
			return 0;
		break;

	default:
		if ((lane01 & 7) == 7)
			return 0;
		break;
	}

	return dp501_reset_link(dev);
}

static int dp501_attach(struct udevice *dev)
{
	struct dp501_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dp501_sw_init(dev);
	if (ret)
		return ret;

	mdelay(90);

	ret = dp501_link_training(dev);
	if (ret)
		return ret;

	/* Perform panel HW setup */
	return panel_enable_backlight(priv->panel);
}

static int dp501_set_backlight(struct udevice *dev, int percent)
{
	struct dp501_priv *priv = dev_get_priv(dev);

	return panel_set_backlight(priv->panel, percent);
}

static int dp501_panel_timings(struct udevice *dev,
			       struct display_timing *timing)
{
	struct dp501_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));
	return 0;
}

static void dp501_hw_init(struct udevice *dev)
{
	struct dp501_priv *priv = dev_get_priv(dev);
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);

	dm_gpio_set_value(&uc_priv->reset, 1);

	regulator_set_enable_if_allowed(priv->vdd, 1);
	dm_gpio_set_value(&priv->enable_gpio, 1);

	udelay(100);

	dm_gpio_set_value(&uc_priv->reset, 0);
	mdelay(80);
}

static int dp501_setup(struct udevice *dev)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct dp501_priv *priv = dev_get_priv(dev);
	struct udevice *bus = dev_get_parent(dev);
	int ret;

	/* get panel */
	ret = uclass_get_device_by_phandle(UCLASS_PANEL, dev,
					   "panel", &priv->panel);
	if (ret) {
		log_debug("%s: Cannot get panel: ret=%d\n", __func__, ret);
		return log_ret(ret);
	}

	/* get regulators */
	ret = device_get_supply_regulator(dev, "power-supply", &priv->vdd);
	if (ret) {
		log_debug("%s: vddc regulator error: %d\n", __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	/* get gpios */
	ret = gpio_request_by_name(dev, "enable-gpios", 0,
				   &priv->enable_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: Could not decode enable-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = i2c_get_chip(bus, chip->chip_addr + 2, 1, &priv->chip2);
	if (ret) {
		log_debug("%s: cannot get second PMIC I2C chip (err %d)\n",
			  __func__, ret);
		return ret;
	}

	dp501_hw_init(dev);

	/* get EDID */
	return panel_get_display_timing(priv->panel, &priv->timing);
}

static int dp501_probe(struct udevice *dev)
{
	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	return dp501_setup(dev);
}

static const struct video_bridge_ops dp501_ops = {
	.attach			= dp501_attach,
	.set_backlight		= dp501_set_backlight,
	.get_display_timing	= dp501_panel_timings,
};

static const struct udevice_id dp501_ids[] = {
	{ .compatible = "parade,dp501" },
	{ }
};

U_BOOT_DRIVER(dp501) = {
	.name		= "dp501",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= dp501_ids,
	.ops		= &dp501_ops,
	.probe		= dp501_probe,
	.priv_auto	= sizeof(struct dp501_priv),
};
