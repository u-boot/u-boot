// SPDX-License-Identifier: GPL-2.0+
/*
 * Motorcomm YT8511/YT8531/YT8531S/YT8821/YT8521S PHY driver.
 *
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Copyright (C) 2024 Motorcomm Electronic Technology Co., Ltd.
 */

#include <config.h>
#include <malloc.h>
#include <phy.h>
#include <linux/bitfield.h>

#define PHY_ID_YT8511				0x0000010a
#define PHY_ID_YT8531				0x4f51e91b
#define PHY_ID_YT8821				0x4f51ea19
#define PHY_ID_YT8531S				0x4f51e91a
#define PHY_ID_YT8521S				0x0000011a
#define PHY_ID_MASK				GENMASK(31, 0)

/* Extended Register's Address Offset Register */
#define YTPHY_PAGE_SELECT			0x1E

/* Extended Register's Data Register */
#define YTPHY_PAGE_DATA			0x1F

#define YTPHY_SYNCE_CFG_REG			0xA012

#define YT8531_PAD_DRIVE_STRENGTH_CFG_REG		0xA010
#define YT8531_RGMII_RXC_DS_MASK		GENMASK(15, 13)
#define YT8531_RGMII_RXD_DS_HI_MASK		BIT(12)		/* Bit 2 of rxd_ds */
#define YT8531_RGMII_RXD_DS_LOW_MASK		GENMASK(5, 4)	/* Bit 1/0 of rxd_ds */
#define YT8531_RGMII_RX_DS_DEFAULT		0x3

#define YTPHY_DTS_OUTPUT_CLK_DIS		0
#define YTPHY_DTS_OUTPUT_CLK_25M		25000000
#define YTPHY_DTS_OUTPUT_CLK_125M		125000000

#define YT8521S_SCR_SYNCE_ENABLE		BIT(5)
/* 1b0 output 25m clock   *default*
 * 1b1 output 125m clock
 */
#define YT8521S_SCR_CLK_FRE_SEL_125M		BIT(3)
#define YT8521S_SCR_CLK_SRC_MASK		GENMASK(2, 1)
#define YT8521S_SCR_CLK_SRC_PLL_125M		0
#define YT8521S_SCR_CLK_SRC_UTP_RX		1
#define YT8521S_SCR_CLK_SRC_SDS_RX		2
#define YT8521S_SCR_CLK_SRC_REF_25M		3

#define YT8511_EXT_CLK_GATE	0x0c
#define YT8511_EXT_DELAY_DRIVE	0x0d
#define YT8511_EXT_SLEEP_CTRL	0x27

/* 2b00 25m from pll
 * 2b01 25m from xtl *default*
 * 2b10 62.m from pll
 * 2b11 125m from pll
 */
#define YT8511_CLK_125M		(BIT(2) | BIT(1))
#define YT8511_PLLON_SLP	BIT(14)

/* RX Delay enabled = 1.8ns 1000T, 8ns 10/100T */
#define YT8511_DELAY_RX		BIT(0)

/* TX Gig-E Delay is bits 7:4, default 0x5
 * TX Fast-E Delay is bits 15:12, default 0xf
 * Delay = 150ps * N - 250ps
 * On = 2000ps, off = 50ps
 */
#define YT8511_DELAY_GE_TX_EN	(0xf << 4)
#define YT8511_DELAY_GE_TX_DIS	(0x2 << 4)
#define YT8511_DELAY_FE_TX_EN	(0xf << 12)
#define YT8511_DELAY_FE_TX_DIS	(0x2 << 12)

#define YT8531_SCR_SYNCE_ENABLE		BIT(6)
/* 1b0 output 25m clock   *default*
 * 1b1 output 125m clock
 */
#define YT8531_SCR_CLK_FRE_SEL_125M		BIT(4)
#define YT8531_SCR_CLK_SRC_MASK		GENMASK(3, 1)
#define YT8531_SCR_CLK_SRC_PLL_125M		0
#define YT8531_SCR_CLK_SRC_UTP_RX		1
#define YT8531_SCR_CLK_SRC_SDS_RX		2
#define YT8531_SCR_CLK_SRC_CLOCK_FROM_DIGITAL	3
#define YT8531_SCR_CLK_SRC_REF_25M		4
#define YT8531_SCR_CLK_SRC_SSC_25M		5

/* 1b0 use original tx_clk_rgmii  *default*
 * 1b1 use inverted tx_clk_rgmii.
 */
#define YT8531_RC1R_TX_CLK_SEL_INVERTED	BIT(14)
#define YT8531_RC1R_RX_DELAY_MASK		GENMASK(13, 10)
#define YT8531_RC1R_FE_TX_DELAY_MASK		GENMASK(7, 4)
#define YT8531_RC1R_GE_TX_DELAY_MASK		GENMASK(3, 0)
#define YT8531_RC1R_RGMII_0_000_NS		0
#define YT8531_RC1R_RGMII_0_150_NS		1
#define YT8531_RC1R_RGMII_0_300_NS		2
#define YT8531_RC1R_RGMII_0_450_NS		3
#define YT8531_RC1R_RGMII_0_600_NS		4
#define YT8531_RC1R_RGMII_0_750_NS		5
#define YT8531_RC1R_RGMII_0_900_NS		6
#define YT8531_RC1R_RGMII_1_050_NS		7
#define YT8531_RC1R_RGMII_1_200_NS		8
#define YT8531_RC1R_RGMII_1_350_NS		9
#define YT8531_RC1R_RGMII_1_500_NS		10
#define YT8531_RC1R_RGMII_1_650_NS		11
#define YT8531_RC1R_RGMII_1_800_NS		12
#define YT8531_RC1R_RGMII_1_950_NS		13
#define YT8531_RC1R_RGMII_2_100_NS		14
#define YT8531_RC1R_RGMII_2_250_NS		15

/* Phy gmii clock gating Register */
#define YT8531_CLOCK_GATING_REG		0xC
#define YT8531_CGR_RX_CLK_EN			BIT(12)

/* Specific Status Register */
#define YTPHY_SPECIFIC_STATUS_REG		0x11
#define YTPHY_DUPLEX_MASK			BIT(13)
#define YTPHY_DUPLEX_SHIFT			13
#define YTPHY_SPEED_MASK			((0x3 << 14) | BIT(9))
#define YTPHY_SPEED_10M				((0x0 << 14))
#define YTPHY_SPEED_100M			((0x1 << 14))
#define YTPHY_SPEED_1000M			((0x2 << 14))
#define YTPHY_SPEED_10G				((0x3 << 14))
#define YTPHY_SPEED_2500M			((0x0 << 14) | BIT(9))

#define YT8531_EXTREG_SLEEP_CONTROL1_REG	0x27
#define YT8531_ESC1R_SLEEP_SW			BIT(15)
#define YT8531_ESC1R_PLLON_SLP			BIT(14)

#define YT8531_RGMII_CONFIG1_REG		0xA003

#define YT8531_CHIP_CONFIG_REG			0xA001
#define YT8531_CCR_SW_RST			BIT(15)
/* 1b0 disable 1.9ns rxc clock delay  *default*
 * 1b1 enable 1.9ns rxc clock delay
 */
#define YT8531_CCR_RXC_DLY_EN			BIT(8)
#define YT8531_CCR_RXC_DLY_1_900_NS		1900

#define YT8531_CCR_CFG_LDO_MASK		GENMASK(5, 4)
#define YT8531_CCR_CFG_LDO_3V3			0x0
#define YT8531_CCR_CFG_LDO_1V8			0x2

/* bits in struct ytphy_plat_priv->flag */
#define TX_CLK_ADJ_ENABLED			BIT(0)
#define AUTO_SLEEP_DISABLED			BIT(1)
#define KEEP_PLL_ENABLED			BIT(2)
#define TX_CLK_10_INVERTED			BIT(3)
#define TX_CLK_100_INVERTED			BIT(4)
#define TX_CLK_1000_INVERTED			BIT(5)

#define YT8821_SDS_EXT_CSR_CTRL_REG		0x23
#define YT8821_SDS_EXT_CSR_VCO_LDO_EN		BIT(15)
#define YT8821_SDS_EXT_CSR_VCO_BIAS_LPF_EN	BIT(8)

#define YT8821_UTP_EXT_PI_CTRL_REG		0x56
#define YT8821_UTP_EXT_PI_RST_N_FIFO		BIT(5)
#define YT8821_UTP_EXT_PI_TX_CLK_SEL_AFE	BIT(4)
#define YT8821_UTP_EXT_PI_RX_CLK_3_SEL_AFE	BIT(3)
#define YT8821_UTP_EXT_PI_RX_CLK_2_SEL_AFE	BIT(2)
#define YT8821_UTP_EXT_PI_RX_CLK_1_SEL_AFE	BIT(1)
#define YT8821_UTP_EXT_PI_RX_CLK_0_SEL_AFE	BIT(0)

#define YT8821_UTP_EXT_VCT_CFG6_CTRL_REG	0x97
#define YT8821_UTP_EXT_FECHO_AMP_TH_HUGE	GENMASK(15, 8)

#define YT8821_UTP_EXT_ECHO_CTRL_REG		0x336
#define YT8821_UTP_EXT_TRACE_LNG_GAIN_THR_1000	GENMASK(14, 8)

#define YT8821_UTP_EXT_GAIN_CTRL_REG		0x340
#define YT8821_UTP_EXT_TRACE_MED_GAIN_THR_1000	GENMASK(6, 0)

#define YT8821_UTP_EXT_RPDN_CTRL_REG		0x34E
#define YT8821_UTP_EXT_RPDN_BP_FFE_LNG_2500	BIT(15)
#define YT8821_UTP_EXT_RPDN_BP_FFE_SHT_2500	BIT(7)
#define YT8821_UTP_EXT_RPDN_IPR_SHT_2500	GENMASK(6, 0)

#define YT8821_UTP_EXT_TH_20DB_2500_CTRL_REG	0x36A
#define YT8821_UTP_EXT_TH_20DB_2500		GENMASK(15, 0)

#define YT8821_UTP_EXT_TRACE_CTRL_REG		0x372
#define YT8821_UTP_EXT_TRACE_LNG_GAIN_THE_2500	GENMASK(14, 8)
#define YT8821_UTP_EXT_TRACE_MED_GAIN_THE_2500	GENMASK(6, 0)

#define YT8821_UTP_EXT_ALPHA_IPR_CTRL_REG	0x374
#define YT8821_UTP_EXT_ALPHA_SHT_2500		GENMASK(14, 8)
#define YT8821_UTP_EXT_IPR_LNG_2500		GENMASK(6, 0)

#define YT8821_UTP_EXT_PLL_CTRL_REG		0x450
#define YT8821_UTP_EXT_PLL_SPARE_CFG		GENMASK(7, 0)

#define YT8821_UTP_EXT_DAC_IMID_CH_2_3_CTRL_REG	0x466
#define YT8821_UTP_EXT_DAC_IMID_CH_3_10_ORG	GENMASK(14, 8)
#define YT8821_UTP_EXT_DAC_IMID_CH_2_10_ORG	GENMASK(6, 0)

#define YT8821_UTP_EXT_DAC_IMID_CH_0_1_CTRL_REG	0x467
#define YT8821_UTP_EXT_DAC_IMID_CH_1_10_ORG	GENMASK(14, 8)
#define YT8821_UTP_EXT_DAC_IMID_CH_0_10_ORG	GENMASK(6, 0)

#define YT8821_UTP_EXT_DAC_IMSB_CH_2_3_CTRL_REG	0x468
#define YT8821_UTP_EXT_DAC_IMSB_CH_3_10_ORG	GENMASK(14, 8)
#define YT8821_UTP_EXT_DAC_IMSB_CH_2_10_ORG	GENMASK(6, 0)

#define YT8821_UTP_EXT_DAC_IMSB_CH_0_1_CTRL_REG	0x469
#define YT8821_UTP_EXT_DAC_IMSB_CH_1_10_ORG	GENMASK(14, 8)
#define YT8821_UTP_EXT_DAC_IMSB_CH_0_10_ORG	GENMASK(6, 0)

#define YT8821_UTP_EXT_MU_COARSE_FR_CTRL_REG	0x4B3
#define YT8821_UTP_EXT_MU_COARSE_FR_F_FFE	GENMASK(14, 12)
#define YT8821_UTP_EXT_MU_COARSE_FR_F_FBE	GENMASK(10, 8)

#define YT8821_UTP_EXT_MU_FINE_FR_CTRL_REG	0x4B5
#define YT8821_UTP_EXT_MU_FINE_FR_F_FFE		GENMASK(14, 12)
#define YT8821_UTP_EXT_MU_FINE_FR_F_FBE		GENMASK(10, 8)

#define YT8821_UTP_EXT_VGA_LPF1_CAP_CTRL_REG	0x4D2
#define YT8821_UTP_EXT_VGA_LPF1_CAP_OTHER	GENMASK(7, 4)
#define YT8821_UTP_EXT_VGA_LPF1_CAP_2500	GENMASK(3, 0)

#define YT8821_UTP_EXT_VGA_LPF2_CAP_CTRL_REG	0x4D3
#define YT8821_UTP_EXT_VGA_LPF2_CAP_OTHER	GENMASK(7, 4)
#define YT8821_UTP_EXT_VGA_LPF2_CAP_2500	GENMASK(3, 0)

#define YT8821_UTP_EXT_TXGE_NFR_FR_THP_CTRL_REG	0x660
#define YT8821_UTP_EXT_NFR_TX_ABILITY		BIT(3)

#define YT8821_CHIP_MODE_FORCE_BX2500		1

/* chip config register */
#define YTPHY_CCR_MODE_SEL_MASK			GENMASK(2, 0)

#define YTPHY_REG_SPACE_SELECT_REG		0xA000
#define YTPHY_RSSR_SPACE_MASK			BIT(1)
#define YTPHY_RSSR_FIBER_SPACE			(0x1 << 1)
#define YTPHY_RSSR_UTP_SPACE			(0x0 << 1)

struct ytphy_plat_priv {
	u32 rx_delay_ps;
	u32 tx_delay_ps;
	u32 clk_out_frequency;
	u32 flag;
};

/**
 * struct ytphy_cfg_reg_map - map a config value to a register value
 * @cfg: value in device configuration
 * @reg: value in the register
 */
struct ytphy_cfg_reg_map {
	u32 cfg;
	u32 reg;
};

static const struct ytphy_cfg_reg_map ytphy_rgmii_delays[] = {
	/* for tx delay / rx delay with YT8531_CCR_RXC_DLY_EN is not set. */
	{ 0,	YT8531_RC1R_RGMII_0_000_NS },
	{ 150,	YT8531_RC1R_RGMII_0_150_NS },
	{ 300,	YT8531_RC1R_RGMII_0_300_NS },
	{ 450,	YT8531_RC1R_RGMII_0_450_NS },
	{ 600,	YT8531_RC1R_RGMII_0_600_NS },
	{ 750,	YT8531_RC1R_RGMII_0_750_NS },
	{ 900,	YT8531_RC1R_RGMII_0_900_NS },
	{ 1050,	YT8531_RC1R_RGMII_1_050_NS },
	{ 1200,	YT8531_RC1R_RGMII_1_200_NS },
	{ 1350,	YT8531_RC1R_RGMII_1_350_NS },
	{ 1500,	YT8531_RC1R_RGMII_1_500_NS },
	{ 1650,	YT8531_RC1R_RGMII_1_650_NS },
	{ 1800,	YT8531_RC1R_RGMII_1_800_NS },
	{ 1950,	YT8531_RC1R_RGMII_1_950_NS },	/* default tx/rx delay */
	{ 2100,	YT8531_RC1R_RGMII_2_100_NS },
	{ 2250,	YT8531_RC1R_RGMII_2_250_NS },

	/* only for rx delay with YT8531_CCR_RXC_DLY_EN is set. */
	{ 0    + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_000_NS },
	{ 150  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_150_NS },
	{ 300  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_300_NS },
	{ 450  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_450_NS },
	{ 600  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_600_NS },
	{ 750  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_750_NS },
	{ 900  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_900_NS },
	{ 1050 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_050_NS },
	{ 1200 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_200_NS },
	{ 1350 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_350_NS },
	{ 1500 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_500_NS },
	{ 1650 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_650_NS },
	{ 1800 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_800_NS },
	{ 1950 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_950_NS },
	{ 2100 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_2_100_NS },
	{ 2250 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_2_250_NS }
};

static u32 ytphy_get_delay_reg_value(struct phy_device *phydev,
				     u32 val,
				     u16 *rxc_dly_en)
{
	int tb_size = ARRAY_SIZE(ytphy_rgmii_delays);
	int tb_size_half = tb_size / 2;
	int i;

	/* when rxc_dly_en is NULL, it is get the delay for tx, only half of
	 * tb_size is valid.
	 */
	if (!rxc_dly_en)
		tb_size = tb_size_half;

	for (i = 0; i < tb_size; i++) {
		if (ytphy_rgmii_delays[i].cfg == val) {
			if (rxc_dly_en && i < tb_size_half)
				*rxc_dly_en = 0;
			return ytphy_rgmii_delays[i].reg;
		}
	}

	pr_warn("Unsupported value %d, using default (%u)\n",
		val, YT8531_RC1R_RGMII_1_950_NS);

	/* when rxc_dly_en is not NULL, it is get the delay for rx.
	 * The rx default in dts and ytphy_rgmii_clk_delay_config is 1950 ps,
	 * so YT8531_CCR_RXC_DLY_EN should not be set.
	 */
	if (rxc_dly_en)
		*rxc_dly_en = 0;

	return YT8531_RC1R_RGMII_1_950_NS;
}

static int ytphy_modify_ext(struct phy_device *phydev, u16 regnum, u16 mask,
			    u16 set)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, YTPHY_PAGE_SELECT, regnum);
	if (ret < 0)
		return ret;

	return phy_modify(phydev, MDIO_DEVAD_NONE, YTPHY_PAGE_DATA, mask, set);
}

static int ytphy_read_ext(struct phy_device *phydev, u16 regnum)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, YTPHY_PAGE_SELECT, regnum);
	if (ret < 0)
		return ret;

	return phy_read(phydev, MDIO_DEVAD_NONE, YTPHY_PAGE_DATA);
}

static int ytphy_rgmii_clk_delay_config(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;
	u16 rxc_dly_en = YT8531_CCR_RXC_DLY_EN;
	u32 rx_reg, tx_reg;
	u16 mask, val = 0;
	int ret;

	rx_reg = ytphy_get_delay_reg_value(phydev, priv->rx_delay_ps,
					   &rxc_dly_en);
	tx_reg = ytphy_get_delay_reg_value(phydev, priv->tx_delay_ps,
					   NULL);

	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		rxc_dly_en = 0;
		break;
	case PHY_INTERFACE_MODE_RGMII_RXID:
		val |= FIELD_PREP(YT8531_RC1R_RX_DELAY_MASK, rx_reg);
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		rxc_dly_en = 0;
		val |= FIELD_PREP(YT8531_RC1R_GE_TX_DELAY_MASK, tx_reg);
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		val |= FIELD_PREP(YT8531_RC1R_RX_DELAY_MASK, rx_reg) |
		       FIELD_PREP(YT8531_RC1R_GE_TX_DELAY_MASK, tx_reg);
		break;
	default: /* do not support other modes */
		return -EOPNOTSUPP;
	}

	ret = ytphy_modify_ext(phydev, YT8531_CHIP_CONFIG_REG,
			       YT8531_CCR_RXC_DLY_EN, rxc_dly_en);
	if (ret < 0)
		return ret;

	/* Generally, it is not necessary to adjust YT8531_RC1R_FE_TX_DELAY */
	mask = YT8531_RC1R_RX_DELAY_MASK | YT8531_RC1R_GE_TX_DELAY_MASK;
	return ytphy_modify_ext(phydev, YT8531_RGMII_CONFIG1_REG, mask, val);
}

static int yt8531_parse_status(struct phy_device *phydev)
{
	int val;
	int speed, speed_mode;

	val = phy_read(phydev, MDIO_DEVAD_NONE, YTPHY_SPECIFIC_STATUS_REG);
	if (val < 0)
		return val;

	speed_mode = (val & YTPHY_SPEED_MASK);
	switch (speed_mode) {
	case YTPHY_SPEED_1000M:
		speed = SPEED_1000;
		break;
	case YTPHY_SPEED_100M:
		speed = SPEED_100;
		break;
	case YTPHY_SPEED_10M:
		speed = SPEED_10;
		break;
	}

	phydev->speed = speed;
	phydev->duplex = (val & YTPHY_DUPLEX_MASK) >> YTPHY_DUPLEX_SHIFT;

	return 0;
}

static int yt8531_startup(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;
	u16 val = 0;
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	ret = yt8531_parse_status(phydev);
	if (ret)
		return ret;

	if (phydev->speed < 0)
		return -EINVAL;

	if (!(priv->flag & TX_CLK_ADJ_ENABLED))
		return 0;

	switch (phydev->speed) {
	case SPEED_1000:
		if (priv->flag & TX_CLK_1000_INVERTED)
			val = YT8531_RC1R_TX_CLK_SEL_INVERTED;
		break;
	case SPEED_100:
		if (priv->flag & TX_CLK_100_INVERTED)
			val = YT8531_RC1R_TX_CLK_SEL_INVERTED;
		break;
	case SPEED_10:
		if (priv->flag & TX_CLK_10_INVERTED)
			val = YT8531_RC1R_TX_CLK_SEL_INVERTED;
		break;
	default:
		printf("UNKNOWN SPEED\n");
		return -EINVAL;
	}

	ret = ytphy_modify_ext(phydev, YT8531_RGMII_CONFIG1_REG,
			       YT8531_RC1R_TX_CLK_SEL_INVERTED, val);
	if (ret < 0)
		pr_warn("Modify TX_CLK_SEL err:%d\n", ret);

	return 0;
}

static void ytphy_dt_parse(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;

	priv->clk_out_frequency = ofnode_read_u32_default(phydev->node,
							  "motorcomm,clk-out-frequency-hz",
							  YTPHY_DTS_OUTPUT_CLK_DIS);
	priv->rx_delay_ps = ofnode_read_u32_default(phydev->node,
						    "rx-internal-delay-ps",
						    YT8531_RC1R_RGMII_1_950_NS);
	priv->tx_delay_ps = ofnode_read_u32_default(phydev->node,
						    "tx-internal-delay-ps",
						    YT8531_RC1R_RGMII_1_950_NS);

	if (ofnode_read_bool(phydev->node, "motorcomm,auto-sleep-disabled"))
		priv->flag |= AUTO_SLEEP_DISABLED;

	if (ofnode_read_bool(phydev->node, "motorcomm,keep-pll-enabled"))
		priv->flag |= KEEP_PLL_ENABLED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-adj-enabled"))
		priv->flag |= TX_CLK_ADJ_ENABLED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-10-inverted"))
		priv->flag |= TX_CLK_10_INVERTED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-100-inverted"))
		priv->flag |= TX_CLK_100_INVERTED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-1000-inverted"))
		priv->flag |= TX_CLK_1000_INVERTED;
}

static int yt8511_config(struct phy_device *phydev)
{
	u32 ge, fe;
	int ret;

	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;

	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		ge = YT8511_DELAY_GE_TX_DIS;
		fe = YT8511_DELAY_FE_TX_DIS;
		break;
	case PHY_INTERFACE_MODE_RGMII_RXID:
		ge = YT8511_DELAY_RX | YT8511_DELAY_GE_TX_DIS;
		fe = YT8511_DELAY_FE_TX_DIS;
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		ge = YT8511_DELAY_GE_TX_EN;
		fe = YT8511_DELAY_FE_TX_EN;
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		ge = YT8511_DELAY_RX | YT8511_DELAY_GE_TX_EN;
		fe = YT8511_DELAY_FE_TX_EN;
		break;
	default: /* do not support other modes */
		return -EOPNOTSUPP;
	}

	ret = ytphy_modify_ext(phydev, YT8511_EXT_CLK_GATE,
			       (YT8511_DELAY_RX | YT8511_DELAY_GE_TX_EN), ge);
	if (ret < 0)
		return ret;
	/* set clock mode to 125m */
	ret = ytphy_modify_ext(phydev, YT8511_EXT_CLK_GATE,
			       YT8511_CLK_125M, YT8511_CLK_125M);
	if (ret < 0)
		return ret;
	ret = ytphy_modify_ext(phydev, YT8511_EXT_DELAY_DRIVE,
			       YT8511_DELAY_FE_TX_EN, fe);
	if (ret < 0)
		return ret;
	/* sleep control, disable PLL in sleep for now */
	ret = ytphy_modify_ext(phydev, YT8511_EXT_SLEEP_CTRL, YT8511_PLLON_SLP,
			       0);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * struct ytphy_ldo_vol_map - map a current value to a register value
 * @vol: ldo voltage
 * @ds:  value in the register
 * @cur: value in device configuration
 */
struct ytphy_ldo_vol_map {
	u32 vol;
	u32 ds;
	u32 cur;
};

static const struct ytphy_ldo_vol_map yt8531_ldo_vol[] = {
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 0, .cur = 1200},
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 1, .cur = 2100},
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 2, .cur = 2700},
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 3, .cur = 2910},
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 4, .cur = 3110},
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 5, .cur = 3600},
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 6, .cur = 3970},
	{.vol = YT8531_CCR_CFG_LDO_1V8, .ds = 7, .cur = 4350},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 0, .cur = 3070},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 1, .cur = 4080},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 2, .cur = 4370},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 3, .cur = 4680},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 4, .cur = 5020},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 5, .cur = 5450},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 6, .cur = 5740},
	{.vol = YT8531_CCR_CFG_LDO_3V3, .ds = 7, .cur = 6140},
};

static u32 yt8531_get_ldo_vol(struct phy_device *phydev)
{
	u32 val;

	val = ytphy_read_ext(phydev, YT8531_CHIP_CONFIG_REG);
	val = FIELD_GET(YT8531_CCR_CFG_LDO_MASK, val);

	return val <= YT8531_CCR_CFG_LDO_1V8 ? val : YT8531_CCR_CFG_LDO_1V8;
}

static int yt8531_get_ds_map(struct phy_device *phydev, u32 cur)
{
	u32 vol;
	int i;

	vol = yt8531_get_ldo_vol(phydev);
	for (i = 0; i < ARRAY_SIZE(yt8531_ldo_vol); i++) {
		if (yt8531_ldo_vol[i].vol == vol && yt8531_ldo_vol[i].cur == cur)
			return yt8531_ldo_vol[i].ds;
	}

	return -EINVAL;
}

static int yt8531_set_ds(struct phy_device *phydev)
{
	u32 ds_field_low, ds_field_hi, val;
	int ret, ds;

	/* set rgmii rx clk driver strength */
	if (!ofnode_read_u32(phydev->node, "motorcomm,rx-clk-drv-microamp", &val)) {
		ds = yt8531_get_ds_map(phydev, val);
		if (ds < 0) {
			pr_warn("No matching current value was found.");
			return -EINVAL;
		}
	} else {
		ds = YT8531_RGMII_RX_DS_DEFAULT;
	}

	ret = ytphy_modify_ext(phydev,
			       YT8531_PAD_DRIVE_STRENGTH_CFG_REG,
			       YT8531_RGMII_RXC_DS_MASK,
			       FIELD_PREP(YT8531_RGMII_RXC_DS_MASK, ds));
	if (ret < 0)
		return ret;

	/* set rgmii rx data driver strength */
	if (!ofnode_read_u32(phydev->node, "motorcomm,rx-data-drv-microamp", &val)) {
		ds = yt8531_get_ds_map(phydev, val);
		if (ds < 0) {
			pr_warn("No matching current value was found.");
			return -EINVAL;
		}
	} else {
		ds = YT8531_RGMII_RX_DS_DEFAULT;
	}

	ds_field_hi = FIELD_GET(BIT(2), ds);
	ds_field_hi = FIELD_PREP(YT8531_RGMII_RXD_DS_HI_MASK, ds_field_hi);

	ds_field_low = FIELD_GET(GENMASK(1, 0), ds);
	ds_field_low = FIELD_PREP(YT8531_RGMII_RXD_DS_LOW_MASK, ds_field_low);

	ret = ytphy_modify_ext(phydev,
			       YT8531_PAD_DRIVE_STRENGTH_CFG_REG,
			       YT8531_RGMII_RXD_DS_LOW_MASK | YT8531_RGMII_RXD_DS_HI_MASK,
			       ds_field_low | ds_field_hi);
	if (ret < 0)
		return ret;

	return 0;
}

static int yt8531_config(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;
	u16 mask, val;
	int ret;

	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;

	ytphy_dt_parse(phydev);
	switch (priv->clk_out_frequency) {
	case YTPHY_DTS_OUTPUT_CLK_DIS:
		mask = YT8531_SCR_SYNCE_ENABLE;
		val = 0;
		break;
	case YTPHY_DTS_OUTPUT_CLK_25M:
		mask = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_SRC_MASK |
			   YT8531_SCR_CLK_FRE_SEL_125M;
		val = YT8531_SCR_SYNCE_ENABLE |
			  FIELD_PREP(YT8531_SCR_CLK_SRC_MASK,
				     YT8531_SCR_CLK_SRC_REF_25M);
		break;
	case YTPHY_DTS_OUTPUT_CLK_125M:
		mask = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_SRC_MASK |
			   YT8531_SCR_CLK_FRE_SEL_125M;
		val = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_FRE_SEL_125M |
			  FIELD_PREP(YT8531_SCR_CLK_SRC_MASK,
				     YT8531_SCR_CLK_SRC_PLL_125M);
		break;
	default:
		pr_warn("Freq err:%u\n", priv->clk_out_frequency);
		return -EINVAL;
	}

	ret = ytphy_modify_ext(phydev, YTPHY_SYNCE_CFG_REG, mask,
			       val);
	if (ret < 0)
		return ret;

	ret = ytphy_rgmii_clk_delay_config(phydev);
	if (ret < 0)
		return ret;

	if (priv->flag & AUTO_SLEEP_DISABLED) {
		/* disable auto sleep */
		ret = ytphy_modify_ext(phydev,
				       YT8531_EXTREG_SLEEP_CONTROL1_REG,
				       YT8531_ESC1R_SLEEP_SW, 0);
		if (ret < 0)
			return ret;
	}

	if (priv->flag & KEEP_PLL_ENABLED) {
		/* enable RXC clock when no wire plug */
		ret = ytphy_modify_ext(phydev,
				       YT8531_CLOCK_GATING_REG,
				       YT8531_CGR_RX_CLK_EN, 0);
		if (ret < 0)
			return ret;
	}

	ret = yt8531_set_ds(phydev);
	if (ret < 0)
		return ret;

	return 0;
}

static int yt8531_probe(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv;

	priv = calloc(1, sizeof(struct ytphy_plat_priv));
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	return 0;
}

static int ytphy_save_page(struct phy_device *phydev)
{
	int old_page;

	old_page = ytphy_read_ext(phydev, YTPHY_REG_SPACE_SELECT_REG);
	if (old_page < 0)
		return old_page;

	if ((old_page & YTPHY_RSSR_SPACE_MASK) == YTPHY_RSSR_FIBER_SPACE)
		return YTPHY_RSSR_FIBER_SPACE;

	return YTPHY_RSSR_UTP_SPACE;
};

static int ytphy_restore_page(struct phy_device *phydev, int page,
			      int ret)
{
	int mask = YTPHY_RSSR_SPACE_MASK;
	int set;
	int r;

	if ((page & YTPHY_RSSR_SPACE_MASK) == YTPHY_RSSR_FIBER_SPACE)
		set = YTPHY_RSSR_FIBER_SPACE;
	else
		set = YTPHY_RSSR_UTP_SPACE;

	r = ytphy_modify_ext(phydev, YTPHY_REG_SPACE_SELECT_REG, mask,
			     set);
	if (ret >= 0 && r < 0)
		ret = r;

	return ret;
};

static int ytphy_write_ext(struct phy_device *phydev, u16 regnum,
			   u16 val)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE,
			YTPHY_PAGE_SELECT, regnum);
	if (ret < 0)
		return ret;

	return phy_write(phydev, MDIO_DEVAD_NONE, YTPHY_PAGE_DATA, val);
}

static int yt8821_probe(struct phy_device *phydev)
{
	phydev->advertising = PHY_GBIT_FEATURES |
				SUPPORTED_2500baseX_Full |
				SUPPORTED_Pause |
				SUPPORTED_Asym_Pause;
	phydev->supported = phydev->advertising;

	return 0;
}

static int yt8821_serdes_init(struct phy_device *phydev)
{
	int old_page;
	u16 mask;
	u16 set;
	int ret;

	old_page = ytphy_save_page(phydev);
	if (old_page < 0)
		return old_page;

	ret = ytphy_modify_ext(phydev, YTPHY_REG_SPACE_SELECT_REG,
			       YTPHY_RSSR_SPACE_MASK,
			       YTPHY_RSSR_FIBER_SPACE);
	if (ret < 0)
		goto err_restore_page;

	ret = phy_modify(phydev, MDIO_DEVAD_NONE, MII_BMCR,
			 BMCR_ANENABLE, 0);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_SDS_EXT_CSR_VCO_LDO_EN |
		YT8821_SDS_EXT_CSR_VCO_BIAS_LPF_EN;
	set = YT8821_SDS_EXT_CSR_VCO_LDO_EN;
	ret = ytphy_modify_ext(phydev, YT8821_SDS_EXT_CSR_CTRL_REG, mask,
			       set);

err_restore_page:
	return ytphy_restore_page(phydev, old_page, ret);
}

static int yt8821_utp_init(struct phy_device *phydev)
{
	int old_page;
	u16 mask;
	u16 save;
	u16 set;
	int ret;

	old_page = ytphy_save_page(phydev);
	if (old_page < 0)
		return old_page;

	ret = ytphy_modify_ext(phydev, YTPHY_REG_SPACE_SELECT_REG,
			       YTPHY_RSSR_SPACE_MASK,
			       YTPHY_RSSR_UTP_SPACE);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_RPDN_BP_FFE_LNG_2500 |
		YT8821_UTP_EXT_RPDN_BP_FFE_SHT_2500 |
		YT8821_UTP_EXT_RPDN_IPR_SHT_2500;
	set = YT8821_UTP_EXT_RPDN_BP_FFE_LNG_2500 |
		YT8821_UTP_EXT_RPDN_BP_FFE_SHT_2500;
	ret = ytphy_modify_ext(phydev, YT8821_UTP_EXT_RPDN_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_VGA_LPF1_CAP_OTHER |
		YT8821_UTP_EXT_VGA_LPF1_CAP_2500;
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_VGA_LPF1_CAP_CTRL_REG,
			       mask, 0);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_VGA_LPF2_CAP_OTHER |
		YT8821_UTP_EXT_VGA_LPF2_CAP_2500;
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_VGA_LPF2_CAP_CTRL_REG,
			       mask, 0);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_TRACE_LNG_GAIN_THE_2500 |
		YT8821_UTP_EXT_TRACE_MED_GAIN_THE_2500;
	set = FIELD_PREP(YT8821_UTP_EXT_TRACE_LNG_GAIN_THE_2500, 0x5a) |
		FIELD_PREP(YT8821_UTP_EXT_TRACE_MED_GAIN_THE_2500, 0x3c);
	ret = ytphy_modify_ext(phydev, YT8821_UTP_EXT_TRACE_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_IPR_LNG_2500;
	set = FIELD_PREP(YT8821_UTP_EXT_IPR_LNG_2500, 0x6c);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_ALPHA_IPR_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_TRACE_LNG_GAIN_THR_1000;
	set = FIELD_PREP(YT8821_UTP_EXT_TRACE_LNG_GAIN_THR_1000, 0x2a);
	ret = ytphy_modify_ext(phydev, YT8821_UTP_EXT_ECHO_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_TRACE_MED_GAIN_THR_1000;
	set = FIELD_PREP(YT8821_UTP_EXT_TRACE_MED_GAIN_THR_1000, 0x22);
	ret = ytphy_modify_ext(phydev, YT8821_UTP_EXT_GAIN_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_TH_20DB_2500;
	set = FIELD_PREP(YT8821_UTP_EXT_TH_20DB_2500, 0x8000);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_TH_20DB_2500_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_MU_COARSE_FR_F_FFE |
		YT8821_UTP_EXT_MU_COARSE_FR_F_FBE;
	set = FIELD_PREP(YT8821_UTP_EXT_MU_COARSE_FR_F_FFE, 0x7) |
		FIELD_PREP(YT8821_UTP_EXT_MU_COARSE_FR_F_FBE, 0x7);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_MU_COARSE_FR_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_MU_FINE_FR_F_FFE |
		YT8821_UTP_EXT_MU_FINE_FR_F_FBE;
	set = FIELD_PREP(YT8821_UTP_EXT_MU_FINE_FR_F_FFE, 0x2) |
		FIELD_PREP(YT8821_UTP_EXT_MU_FINE_FR_F_FBE, 0x2);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_MU_FINE_FR_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	/* save YT8821_UTP_EXT_PI_CTRL_REG's val for use later */
	ret = ytphy_read_ext(phydev, YT8821_UTP_EXT_PI_CTRL_REG);
	if (ret < 0)
		goto err_restore_page;

	save = ret;

	mask = YT8821_UTP_EXT_PI_TX_CLK_SEL_AFE |
		YT8821_UTP_EXT_PI_RX_CLK_3_SEL_AFE |
		YT8821_UTP_EXT_PI_RX_CLK_2_SEL_AFE |
		YT8821_UTP_EXT_PI_RX_CLK_1_SEL_AFE |
		YT8821_UTP_EXT_PI_RX_CLK_0_SEL_AFE;
	ret = ytphy_modify_ext(phydev, YT8821_UTP_EXT_PI_CTRL_REG,
			       mask, 0);
	if (ret < 0)
		goto err_restore_page;

	/* restore YT8821_UTP_EXT_PI_CTRL_REG's val */
	ret = ytphy_write_ext(phydev, YT8821_UTP_EXT_PI_CTRL_REG, save);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_FECHO_AMP_TH_HUGE;
	set = FIELD_PREP(YT8821_UTP_EXT_FECHO_AMP_TH_HUGE, 0x38);
	ret = ytphy_modify_ext(phydev, YT8821_UTP_EXT_VCT_CFG6_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_NFR_TX_ABILITY;
	set = YT8821_UTP_EXT_NFR_TX_ABILITY;
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_TXGE_NFR_FR_THP_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_PLL_SPARE_CFG;
	set = FIELD_PREP(YT8821_UTP_EXT_PLL_SPARE_CFG, 0xe9);
	ret = ytphy_modify_ext(phydev, YT8821_UTP_EXT_PLL_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_DAC_IMID_CH_3_10_ORG |
		YT8821_UTP_EXT_DAC_IMID_CH_2_10_ORG;
	set = FIELD_PREP(YT8821_UTP_EXT_DAC_IMID_CH_3_10_ORG, 0x64) |
		FIELD_PREP(YT8821_UTP_EXT_DAC_IMID_CH_2_10_ORG, 0x64);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_DAC_IMID_CH_2_3_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_DAC_IMID_CH_1_10_ORG |
		YT8821_UTP_EXT_DAC_IMID_CH_0_10_ORG;
	set = FIELD_PREP(YT8821_UTP_EXT_DAC_IMID_CH_1_10_ORG, 0x64) |
		FIELD_PREP(YT8821_UTP_EXT_DAC_IMID_CH_0_10_ORG, 0x64);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_DAC_IMID_CH_0_1_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_DAC_IMSB_CH_3_10_ORG |
		YT8821_UTP_EXT_DAC_IMSB_CH_2_10_ORG;
	set = FIELD_PREP(YT8821_UTP_EXT_DAC_IMSB_CH_3_10_ORG, 0x64) |
		FIELD_PREP(YT8821_UTP_EXT_DAC_IMSB_CH_2_10_ORG, 0x64);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_DAC_IMSB_CH_2_3_CTRL_REG,
			       mask, set);
	if (ret < 0)
		goto err_restore_page;

	mask = YT8821_UTP_EXT_DAC_IMSB_CH_1_10_ORG |
		YT8821_UTP_EXT_DAC_IMSB_CH_0_10_ORG;
	set = FIELD_PREP(YT8821_UTP_EXT_DAC_IMSB_CH_1_10_ORG, 0x64) |
		FIELD_PREP(YT8821_UTP_EXT_DAC_IMSB_CH_0_10_ORG, 0x64);
	ret = ytphy_modify_ext(phydev,
			       YT8821_UTP_EXT_DAC_IMSB_CH_0_1_CTRL_REG,
			       mask, set);

err_restore_page:
	return ytphy_restore_page(phydev, old_page, ret);
}

static int yt8821_auto_sleep_config(struct phy_device *phydev,
				    bool enable)
{
	int old_page;
	int ret;

	old_page = ytphy_save_page(phydev);
	if (old_page < 0)
		return old_page;

	ret = ytphy_modify_ext(phydev, YTPHY_REG_SPACE_SELECT_REG,
			       YTPHY_RSSR_SPACE_MASK,
			       YTPHY_RSSR_UTP_SPACE);
	if (ret < 0)
		goto err_restore_page;

	ret = ytphy_modify_ext(phydev,
			       YT8531_EXTREG_SLEEP_CONTROL1_REG,
			       YT8531_ESC1R_SLEEP_SW,
			       enable ? 1 : 0);

err_restore_page:
	return ytphy_restore_page(phydev, old_page, ret);
}

static int yt8821_soft_reset(struct phy_device *phydev)
{
	return ytphy_modify_ext(phydev, YT8531_CHIP_CONFIG_REG,
				YT8531_CCR_SW_RST, 0);
}

static int yt8821_config(struct phy_device *phydev)
{
	u8 mode = YT8821_CHIP_MODE_FORCE_BX2500;
	int ret;
	u16 set;

	set = FIELD_PREP(YTPHY_CCR_MODE_SEL_MASK, mode);
	ret = ytphy_modify_ext(phydev,
			       YT8531_CHIP_CONFIG_REG,
			       YTPHY_CCR_MODE_SEL_MASK,
			       set);
	if (ret < 0)
		return ret;

	ret = yt8821_serdes_init(phydev);
	if (ret < 0)
		return ret;

	ret = yt8821_utp_init(phydev);
	if (ret < 0)
		return ret;

	ret = yt8821_auto_sleep_config(phydev, false);
	if (ret < 0)
		return ret;

	return yt8821_soft_reset(phydev);
}

static void yt8821_parse_status(struct phy_device *phydev, int val)
{
	int speed_mode;
	int speed;

	speed_mode = val & YTPHY_SPEED_MASK;
	switch (speed_mode) {
	case YTPHY_SPEED_2500M:
		speed = SPEED_2500;
		break;
	case YTPHY_SPEED_1000M:
		speed = SPEED_1000;
		break;
	case YTPHY_SPEED_100M:
		speed = SPEED_100;
		break;
	case YTPHY_SPEED_10M:
		speed = SPEED_10;
		break;
	}

	phydev->speed = speed;
	phydev->duplex = FIELD_GET(YTPHY_DUPLEX_MASK, val);
}

static int yt8821_startup(struct phy_device *phydev)
{
	u16 val;
	int ret;

	ret = ytphy_modify_ext(phydev, YTPHY_REG_SPACE_SELECT_REG,
			       YTPHY_RSSR_SPACE_MASK,
			       YTPHY_RSSR_UTP_SPACE);
	if (ret)
		return ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	ret = phy_read(phydev, MDIO_DEVAD_NONE,
		       YTPHY_SPECIFIC_STATUS_REG);
	if (ret < 0)
		return ret;

	val = ret;

	if (phydev->link)
		yt8821_parse_status(phydev, val);

	return 0;
}

static int yt8521s_config(struct phy_device *phydev)
{
	struct ytphy_plat_priv *priv = phydev->priv;
	u16 mask, val;
	int ret;

	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;

	ytphy_dt_parse(phydev);
	switch (priv->clk_out_frequency) {
	case YTPHY_DTS_OUTPUT_CLK_DIS:
		mask = YT8521S_SCR_SYNCE_ENABLE;
		val = 0;
		break;
	case YTPHY_DTS_OUTPUT_CLK_25M:
		mask = YT8521S_SCR_SYNCE_ENABLE | YT8521S_SCR_CLK_SRC_MASK |
			   YT8521S_SCR_CLK_FRE_SEL_125M;
		val = YT8521S_SCR_SYNCE_ENABLE |
			  FIELD_PREP(YT8521S_SCR_CLK_SRC_MASK,
				     YT8521S_SCR_CLK_SRC_REF_25M);
		break;
	case YTPHY_DTS_OUTPUT_CLK_125M:
		mask = YT8521S_SCR_SYNCE_ENABLE | YT8521S_SCR_CLK_SRC_MASK |
			   YT8521S_SCR_CLK_FRE_SEL_125M;
		val = YT8521S_SCR_SYNCE_ENABLE | YT8521S_SCR_CLK_FRE_SEL_125M |
			  FIELD_PREP(YT8521S_SCR_CLK_SRC_MASK,
				     YT8521S_SCR_CLK_SRC_PLL_125M);
		break;
	default:
		pr_warn("Freq err:%u\n", priv->clk_out_frequency);
		return -EINVAL;
	}

	ret = ytphy_modify_ext(phydev, YTPHY_SYNCE_CFG_REG, mask,
			       val);
	if (ret < 0)
		return ret;

	ret = ytphy_rgmii_clk_delay_config(phydev);
	if (ret < 0)
		return ret;

	if (priv->flag & AUTO_SLEEP_DISABLED) {
		/* disable auto sleep */
		ret = ytphy_modify_ext(phydev,
				       YT8531_EXTREG_SLEEP_CONTROL1_REG,
				       YT8531_ESC1R_SLEEP_SW, 0);
		if (ret < 0)
			return ret;
	}

	if (priv->flag & KEEP_PLL_ENABLED) {
		/* enable RXC clock when no wire plug */
		ret = ytphy_modify_ext(phydev,
				       YT8531_CLOCK_GATING_REG,
				       YT8531_CGR_RX_CLK_EN, 0);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int yt8531s_config(struct phy_device *phydev)
{
	struct ytphy_plat_priv *priv = phydev->priv;
	u16 mask, val;
	int ret;

	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;

	ytphy_dt_parse(phydev);
	switch (priv->clk_out_frequency) {
	case YTPHY_DTS_OUTPUT_CLK_DIS:
		mask = YT8531_SCR_SYNCE_ENABLE;
		val = 0;
		break;
	case YTPHY_DTS_OUTPUT_CLK_25M:
		mask = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_SRC_MASK |
			   YT8531_SCR_CLK_FRE_SEL_125M;
		val = YT8531_SCR_SYNCE_ENABLE |
			  FIELD_PREP(YT8531_SCR_CLK_SRC_MASK,
				     YT8531_SCR_CLK_SRC_REF_25M);
		break;
	case YTPHY_DTS_OUTPUT_CLK_125M:
		mask = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_SRC_MASK |
			   YT8531_SCR_CLK_FRE_SEL_125M;
		val = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_FRE_SEL_125M |
			  FIELD_PREP(YT8531_SCR_CLK_SRC_MASK,
				     YT8531_SCR_CLK_SRC_PLL_125M);
		break;
	default:
		pr_warn("Freq err:%u\n", priv->clk_out_frequency);
		return -EINVAL;
	}

	ret = ytphy_modify_ext(phydev, YTPHY_SYNCE_CFG_REG, mask,
			       val);
	if (ret < 0)
		return ret;

	ret = ytphy_rgmii_clk_delay_config(phydev);
	if (ret < 0)
		return ret;

	if (priv->flag & AUTO_SLEEP_DISABLED) {
		/* disable auto sleep */
		ret = ytphy_modify_ext(phydev,
				       YT8531_EXTREG_SLEEP_CONTROL1_REG,
				       YT8531_ESC1R_SLEEP_SW, 0);
		if (ret < 0)
			return ret;
	}

	if (priv->flag & KEEP_PLL_ENABLED) {
		/* enable RXC clock when no wire plug */
		ret = ytphy_modify_ext(phydev,
				       YT8531_CLOCK_GATING_REG,
				       YT8531_CGR_RX_CLK_EN, 0);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int yt8531s_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	ret = yt8531_parse_status(phydev);
	if (ret)
		return ret;

	return 0;
}

U_BOOT_PHY_DRIVER(motorcomm8511) = {
	.name          = "YT8511 Gigabit Ethernet",
	.uid           = PHY_ID_YT8511,
	.mask          = PHY_ID_MASK,
	.features      = PHY_GBIT_FEATURES,
	.config        = &yt8511_config,
	.startup       = &genphy_startup,
	.shutdown      = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(motorcomm8531) = {
	.name          = "YT8531 Gigabit Ethernet",
	.uid           = PHY_ID_YT8531,
	.mask          = PHY_ID_MASK,
	.features      = PHY_GBIT_FEATURES,
	.probe	       = &yt8531_probe,
	.config        = &yt8531_config,
	.startup       = &yt8531_startup,
	.shutdown      = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(motorcomm8821) = {
	.name		= "YT8821 2.5G Ethernet",
	.uid		= PHY_ID_YT8821,
	.mask		= PHY_ID_MASK,
	.mmds		= (MDIO_MMD_PMAPMD | MDIO_MMD_PCS | MDIO_MMD_AN),
	.probe		= &yt8821_probe,
	.config		= &yt8821_config,
	.startup	= &yt8821_startup,
	.shutdown	= &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(motorcomm8531S) = {
	.name		= "YT8531S Gigabit Ethernet Transceiver",
	.uid		= PHY_ID_YT8531S,
	.mask		= PHY_ID_MASK,
	.features	= PHY_GBIT_FEATURES,
	.probe		= &yt8531_probe,
	.config		= &yt8531s_config,
	.startup	= &yt8531s_startup,
	.shutdown	= &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(motorcomm8521S) = {
	.name		= "YT8521S Gigabit Ethernet Transceiver",
	.uid		= PHY_ID_YT8521S,
	.mask		= PHY_ID_MASK,
	.features	= PHY_GBIT_FEATURES,
	.probe		= &yt8531_probe,
	.config		= &yt8521s_config,
	.startup	= &yt8531s_startup,
	.shutdown	= &genphy_shutdown,
};
