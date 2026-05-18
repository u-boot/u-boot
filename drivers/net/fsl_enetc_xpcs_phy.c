// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2024 NXP
 */

#define XPCS_PHY_GLOBAL		0x0
#define XPCS_PHY_MPLLA		0x1
#define XPCS_PHY_MPLLB		0x2
#define XPCS_PHY_LANE		0x3
#define XPCS_PHY_MAC_ADAPTER	0x1f

#define XPCS_PHY_REG(x) (((x) & 0x1fffe) >> 1)

/* MAC ADAPTER */
#define MAC_ADAPTER_LOCK_PHY	0x200
#define MAC_ADAPTER_LOCK_MPLLA	0x204
#define MAC_ADAPTER_LOCK_MPLLB	0x208
#define MAC_ADAPTER_LOCK_ROM	0x20c
#define MAC_ADAPTER_LOCK_RAM	0x210
#define MAC_ADAPTER_LOCK_EVENT	0x214

#define MAC_ADAPTER_LOCK_LOCK	BIT(7)

/* PMA */
#define PMA_RX_LSTS					0x10040
#define PMA_RX_LSTS_RX_VALID_0			BIT(12)
#define PMA_MP_12G_16G_25G_TX_GENCTRL0			0x10060
#define PMA_TX_GENCTRL0_TX_RST_0		BIT(8)
#define PMA_TX_GENCTRL0_TX_DT_EN_0		BIT(12)
#define PMA_MP_12G_16G_25G_TX_GENCTRL1			0x10062
#define PMA_TX_GENCTRL1_VBOOST_EN_0		BIT(4)
#define PMA_TX_GENCTRL1_VBOOST_LVL_MASK		GENMASK(10, 8)
#define PMA_TX_GENCTRL1_VBOOST_LVL(x)		(((x) << 8) & GENMASK(10, 8))
#define PMA_TX_GENCTRL1_TX_CLK_RDY_0		BIT(12)
#define PMA_MP_12G_16G_TX_GENCTRL2			0x10064
#define PMA_TX_GENCTRL2_TX_REQ_0		BIT(0)
#define PMA_TX_GENCTRL2_TX0_WIDTH_MASK		GENMASK(9, 8)
#define PMA_TX_GENCTRL2_TX0_WIDTH(x)		(((x) << 8) & GENMASK(9, 8))
#define PMA_MP_12G_16G_25G_TX_BOOST_CTRL		0x10066
#define PMA_TX_BOOST_CTRL_TX0_IBOOST_MASK	GENMASK(3, 0)
#define PMA_TX_BOOST_CTRL_TX0_IBOOST(x)		((x) & GENMASK(3, 0))
#define PMA_MP_12G_16G_25G_TX_RATE_CTRL			0x10068
#define PMA_TX_RATE_CTRL_TX0_RATE_MASK		GENMASK(2, 0)
#define PMA_TX_RATE_CTRL_TX0_RATE(x)		((x) & GENMASK(2, 0))
#define PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL		0x1006A
#define PMA_POWER_STATE_CTRL_TX0_PSTATE_MASK	GENMASK(1, 0)
#define PMA_POWER_STATE_CTRL_TX0_PSTATE(x)	((x) & GENMASK(1, 0))
#define PMA_POWER_STATE_CTRL_TX_DISABLE_0	BIT(8)
#define PMA_MP_12G_16G_25G_TX_EQ_CTRL0			0x1006C
#define PMA_TX_EQ_CTRL0_TX_EQ_PRE_MASK		GENMASK(5, 0)
#define PMA_TX_EQ_CTRL0_TX_EQ_PRE(x)		((x) & GENMASK(5, 0))
#define PMA_TX_EQ_CTRL0_TX_EQ_MAIN_MASK		GENMASK(13, 8)
#define PMA_TX_EQ_CTRL0_TX_EQ_MAIN(x)		(((x) << 8) & GENMASK(13, 8))
#define PMA_MP_12G_16G_25G_TX_EQ_CTRL1			0x1006E
#define PMA_TX_EQ_CTRL1_TX_EQ_POST_MASK		GENMASK(5, 0)
#define PMA_TX_EQ_CTRL1_TX_EQ_POST(x)		((x) & GENMASK(5, 0))
#define PMA_MP_16G_25G_TX_MISC_CTRL0			0x1007C
#define PMA_TX_MISC_CTRL0_TX0_MISC_MASK		GENMASK(7, 0)
#define PMA_TX_MISC_CTRL0_TX0_MISC(x)		((x) & GENMASK(7, 0))
#define PMA_MP_12G_16G_25G_RX_GENCTRL0			0x100A0
#define PMA_RX_GENCTRL0_RX_DT_EN_0		BIT(8)
#define PMA_MP_12G_16G_25G_RX_GENCTRL1			0x100A2
#define PMA_RX_GENCTRL1_RX_RST_0		BIT(4)
#define PMA_RX_GENCTRL1_RX_TERM_ACDC_0		BIT(8)
#define PMA_RX_GENCTRL1_RX_DIV16P5_CLK_EN_0	BIT(12)
#define PMA_MP_12G_16G_RX_GENCTRL2			0x100A4
#define PMA_RX_GENCTRL2_RX_REQ_0		BIT(0)
#define PMA_RX_GENCTRL2_RX0_WIDTH_MASK		GENMASK(9, 8)
#define PMA_RX_GENCTRL2_RX0_WIDTH(x)		(((x) << 8) & GENMASK(9, 8))
#define PMA_MP_12G_16G_RX_GENCTRL3			0x100A6
#define PMA_RX_GENCTRL3_LOS_TRSHLD_0_MASK	GENMASK(2, 0)
#define PMA_RX_GENCTRL3_LOS_TRSHLD_0(x)		((x) & GENMASK(2, 0))
#define PMA_RX_GENCTRL3_LOS_LFPS_EN_0		BIT(12)
#define PMA_MP_12G_16G_25G_RX_RATE_CTRL			0x100A8
#define PMA_RX_RATE_CTRL_RX0_RATE_MASK		GENMASK(1, 0)
#define PMA_RX_RATE_CTRL_RX0_RATE(x)		((x) & GENMASK(1, 0))
#define PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL		0x100AA
#define PMA_RX_POWER_STATE_CTRL_RX0_PSTATE_MASK	GENMASK(1, 0)
#define PMA_RX_POWER_STATE_CTRL_RX0_PSTATE(x)	((x) & GENMASK(1, 0))
#define PMA_RX_POWER_STATE_CTRL_RX_DISABLE_0	BIT(8)
#define PMA_MP_12G_16G_25G_RX_CDR_CTRL			0x100AC
#define PMA_RX_CDR_CTRL_CDR_SSC_EN_0		BIT(4)
#define PMA_MP_12G_16G_25G_RX_ATTN_CTRL			0x100AE
#define PMA_RX_ATTN_CTRL_RX0_EQ_ATT_LVL_MASK	GENMASK(2, 0)
#define PMA_RX_ATTN_CTRL_RX0_EQ_ATT_LVL(x)	((x) & GENMASK(2, 0))
#define PMA_MP_16G_25G_RX_EQ_CTRL0			0x100B0
#define PMA_RX_EQ_CTRL0_CTLE_BOOST_0_MASK	GENMASK(4, 0)
#define PMA_RX_EQ_CTRL0_CTLE_BOOST_0(x)		((x) & GENMASK(4, 0))
#define PMA_RX_EQ_CTRL0_CTLE_POLE_0_MASK	GENMASK(6, 5)
#define PMA_RX_EQ_CTRL0_CTLE_POLE_0(x)		(((x) << 5) & GENMASK(6, 5))
#define PMA_RX_EQ_CTRL0_VGA2_GAIN_0_MASK	GENMASK(10, 8)
#define PMA_RX_EQ_CTRL0_VGA2_GAIN_0(x)		(((x) << 8) & GENMASK(10, 8))
#define PMA_RX_EQ_CTRL0_VGA1_GAIN_0_MASK	GENMASK(14, 12)
#define PMA_RX_EQ_CTRL0_VGA1_GAIN_0(x)		(((x) << 12) & GENMASK(14, 12))
#define PMA_MP_12G_16G_25G_RX_EQ_CTRL4			0x100B8
#define PMA_RX_EQ_CTRL4_CONT_ADAPT_0		BIT(0)
#define PMA_RX_EQ_CTRL4_RX_AD_REQ		BIT(12)
#define PMA_MP_16G_25G_RX_EQ_CTRL5			0x100BA
#define PMA_RX_EQ_CTRL5_RX_ADPT_SEL_0		BIT(0)
#define PMA_RX_EQ_CTRL5_RX0_ADPT_MODE_MASK	GENMASK(5, 4)
#define PMA_RX_EQ_CTRL5_RX0_ADPT_MODE(x)	(((x) << 4) & GENMASK(5, 4))
#define PMA_MP_12G_16G_25G_DFE_TAP_CTRL0		0x100BC
#define PMA_DFE_TAP_CTRL0_DFE_TAP1_0_MASK	GENMASK(7, 0)
#define PMA_DFE_TAP_CTRL0_DFE_TAP1_0(x)		((x) & GENMASK(7, 0))
#define PMA_MP_16G_RX_CDR_CTRL1				0x100C8
#define PMA_RX_CDR_CTRL1_VCO_TEMP_COMP_EN_0	BIT(0)
#define PMA_RX_CDR_CTRL1_VCO_STEP_CTRL_0	BIT(4)
#define PMA_RX_CDR_CTRL1_VCO_FRQBAND_0_MASK	GENMASK(9, 8)
#define PMA_RX_CDR_CTRL1_VCO_FRQBAND_0(x)	(((x) << 8) & GENMASK(9, 8))
#define PMA_MP_16G_25G_RX_PPM_CTRL0			0x100CA
#define PMA_RX_PPM_CTRL0_RX0_CDR_PPM_MAX_MASK	GENMASK(4, 0)
#define PMA_RX_PPM_CTRL0_RX0_CDR_PPM_MAX(x)	((x) & GENMASK(4, 0))
#define PMA_MP_16G_25G_RX_GENCTRL4			0x100D0
#define PMA_RX_GENCTRL4_RX_DFE_BYP_0		BIT(8)
#define PMA_MP_16G_25G_RX_MISC_CTRL0			0x100D2
#define PMA_RX_MISC_CTRL0_RX0_MISC_MASK		GENMASK(7, 0)
#define PMA_RX_MISC_CTRL0_RX0_MISC(x)		((x) & GENMASK(7, 0))
#define PMA_MP_16G_25G_RX_IQ_CTRL0			0x100D6
#define PMA_RX_IQ_CTRL0_RX0_MARGIN_IQ_MASK	GENMASK(6, 0)
#define PMA_RX_IQ_CTRL0_RX0_MARGIN_IQ(x)	((x) & GENMASK(6, 0))
#define PMA_RX_IQ_CTRL0_RX0_DELTA_IQ_MASK	GENMASK(11, 8)
#define PMA_RX_IQ_CTRL0_RX0_DELTA_IQ(x)		(((x) << 8) & GENMASK(11, 8))
#define PMA_MP_12G_16G_25G_MPLL_CMN_CTRL		0x100E0
#define PMA_MPLL_CMN_CTRL_MPLL_EN_0		BIT(0)
#define PMA_MPLL_CMN_CTRL_MPLLB_SEL_0		BIT(4)
#define PMA_MP_12G_16G_MPLLA_CTRL0			0x100E2
#define PMA_MPLLA_CTRL0_MPLLA_MULTIPLIER_MASK	GENMASK(7, 0)
#define PMA_MPLLA_CTRL0_MPLLA_MULTIPLIER(x)	((x) & GENMASK(7, 0))
#define PMA_MP_16G_MPLLA_CTRL1				0x100E4
#define PMA_MPLLA_CTRL1_MPLLA_SSC_EN		BIT(0)
#define PMA_MPLLA_CTRL1_MPLLA_SSC_CLK_SEL	BIT(4)
#define PMA_MPLLA_CTRL1_MPLLA_FRACN_CTRL_MASK	GENMASK(15, 5)
#define PMA_MPLLA_CTRL1_MPLLA_FRACN_CTRL(x)	(((x) << 5) & GENMASK(15, 5))
#define PMA_MP_12G_16G_MPLLA_CTRL2			0x100E6
#define PMA_MPLLA_CTRL2_MPLLA_DIV_MULT_MASK	GENMASK(6, 0)
#define PMA_MPLLA_CTRL2_MPLLA_DIV_MULT(x)	((x) & GENMASK(6, 0))
#define PMA_MPLLA_CTRL2_MPLLA_DIV_CLK_EN	BIT(7)
#define PMA_MPLLA_CTRL2_MPLLA_DIV8_CLK_EN	BIT(8)
#define PMA_MPLLA_CTRL2_MPLLA_DIV10_CLK_EN	BIT(9)
#define PMA_MPLLA_CTRL2_MPLLA_DIV16P5_CLK_EN	BIT(10)
#define PMA_MPLLA_CTRL2_MPLLA_TX_CLK_DIV_MASK	GENMASK(12, 11)
#define PMA_MPLLA_CTRL2_MPLLA_TX_CLK_DIV(x)	(((x) << 11) & GENMASK(12, 11))
#define PMA_MP_16G_MPLLA_CTRL3				0x100EE
#define PMA_MPLLA_CTRL3_MPLLA_BANDWIDTH_MASK	GENMASK(15, 0)
#define PMA_MPLLA_CTRL3_MPLLA_BANDWIDTH(x)	((x) & GENMASK(15, 0))
#define PMA_MP_16G_MPLLA_CTRL4				0x100F2
#define PMA_MPLLA_CTRL4_MPLLA_SSC_FRQ_CNT_INT_MASK GENMASK(11, 0)
#define PMA_MPLLA_CTRL4_MPLLA_SSC_FRQ_CNT_INT(x) ((x) & GENMASK(11, 0))
#define PMA_MP_16G_MPLLA_CTRL5				0x100F4
#define PMA_MPLLA_CTRL5_MPLLA_SSC_FRQ_CNT_PK_MASK GENMASK(7, 0)
#define PMA_MPLLA_CTRL5_MPLLA_SSC_FRQ_CNT_PK(x)	((x) & GENMASK(7, 0))
#define PMA_MPLLA_CTRL5_MPLLA_SSC_SPD_EN	BIT(8)
#define PMA_MP_12G_16G_25G_MISC_CTRL0			0x10120
#define PMA_MISC_CTRL0_RX_VREF_CTRL_MASK	GENMASK(12, 8)
#define PMA_MISC_CTRL0_RX_VREF_CTRL(x)		(((x) << 8) & GENMASK(12, 8))
#define PMA_MP_12G_16G_25G_REF_CLK_CTRL			0x10122
#define PMA_REF_CLK_CTRL_REF_CLK_DIV2		BIT(2)
#define PMA_REF_CLK_CTRL_REF_RANGE_MASK		GENMASK(5, 3)
#define PMA_REF_CLK_CTRL_REF_RANGE(x)		(((x) << 3) & GENMASK(5, 3))
#define PMA_REF_CLK_CTRL_REF_MPLLA_DIV2		BIT(6)
#define PMA_MP_12G_16G_25G_VCO_CAL_LD0			0x10124
#define PMA_VCO_CAL_LD0_VCO_LD_VAL_0_MASK	GENMASK(12, 0)
#define PMA_VCO_CAL_LD0_VCO_LD_VAL_0(x)		((x) & GENMASK(12, 0))
#define PMA_MP_16G_25G_VCO_CAL_REF0			0x1012C
#define PMA_VCO_CAL_REF0_VCO_REF_LD_0_MASK	GENMASK(6, 0)
#define PMA_VCO_CAL_REF0_VCO_REF_LD_0(x)	((x) & GENMASK(6, 0))
#define PMA_MP_12G_16G_25G_MISC_STS			0x10130
#define PMA_MISC_STS_RX_ADPT_ACK		BIT(12)
#define PMA_MP_12G_16G_25G_SRAM				0x10136
#define PMA_SRAM_INIT_DN			BIT(0)
#define PMA_SRAM_EXT_LD_DN			BIT(1)
#define PMA_MP_16G_25G_MISC_CTRL2			0x10138
#define PMA_MISC_CTRL2_SUP_MISC_MASK		GENMASK(7, 0)
#define PMA_MISC_CTRL2_SUP_MISC(x)		((x) & GENMASK(7, 0))

/* PCS */
#define PCS_CTRL1				0x0
#define PCS_CTRL1_RESET			BIT(15)
#define PCS_CTRL2				0xE
#define PCS_CTRL2_PCS_TYPE_SEL_MASK	GENMASK(3, 0)
#define PCS_CTRL2_PCS_TYPE_SEL(x)	((x) & GENMASK(3, 0))
#define PCS_DIG_CTRL1				0x10000
#define PCS_DIG_CTRL1_USXG_EN		BIT(9)
#define PCS_DIG_CTRL1_USRA_RST		BIT(10)
#define PCS_DIG_CTRL1_VR_RST		BIT(15)
#define PCS_DEBUG_CTRL				0x1000A
#define PCS_DEBUG_CTRL_SUPRESS_LOS_DET	BIT(4)
#define PCS_DEBUG_CTRL_RX_DT_EN_CTL	BIT(6)
#define PCS_DEBUG_CTRL_TX_PMBL_CTL	BIT(8)
#define PCS_KR_CTRL1				0x1000E
#define PCS_KR_CTRL1_USXG_MODE_MASK	GENMASK(12, 10)
#define PCS_KR_CTRL1_USXG_MODE(x)	(((x) << 10) & GENMASK(12, 10))

/* VS MII MMD */
#define MII_CTRL					0x0
#define MII_CTRL_SS5				BIT(5)
#define MII_CTRL_SS6				BIT(6)
#define MII_CTRL_AN_ENABLE			BIT(12)
#define MII_CTRL_SS13				BIT(13)
#define MII_DIG_CTRL1					0x10000
#define MII_DIG_CTRL1_CL37_TMR_OVR_RIDE		BIT(3)
#define MII_AN_CTRL					0x10002
#define MII_AN_CTRL_MII_AN_INTR_EN		BIT(0)
#define MII_AN_CTRL_TX_CONFIG			BIT(3)
#define MII_AN_INTR_STS					0x10004
#define MII_AN_INTR_STS_CL37_ANCMPLT_INTR	BIT(0)
#define MII_LINK_TIMER_CTRL				0x10014
#define MII_LINK_TIMER_CTRL_CL37_LINK_TIME_MASK		GENMASK(15, 0)
#define MII_LINK_TIMER_CTRL_CL37_LINK_TIME(x)		((x) & GENMASK(15, 0))

/* E16 MEM MAP */
#define IDCODE_LO						0x0
#define IDCODE_HI						0x4
#define GLOBAL_CTRL_EX_0					0x114
#define GLOBAL_CTRL_EX_0_PHY_SRAM_BYPASS		BIT(0)
#define L0_RX_VCO_OVRD_OUT_0					0x20c
#define L0_RX_VCO_OVRD_OUT_0_RX_ANA_CDR_FREQ_TUNE_MASK	GENMASK(12, 3)
#define L0_RX_VCO_OVRD_OUT_0_RX_ANA_CDR_FREQ_TUNE(x)	(((x) << 3) & GENMASK(12, 3))
#define L0_RX_VCO_OVRD_OUT_0_RX_CDR_FREQ_TUNE_OVRD_EN	BIT(15)
#define L0_RX_VCO_OVRD_OUT_2					0x214
#define L0_RX_VCO_OVRD_OUT_2_RX_ANA_CDR_FREQ_TUNE_CLK	BIT(0)

static int enetc_mdio_read(struct mii_dev *bus, int addr, int devad, int reg);
static int enetc_mdio_write(struct mii_dev *bus, int addr, int devad, int reg, u16 val);

int xpcs_read(struct udevice *dev, int devaddr, u32 reg)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	return enetc_mdio_read(&priv->imdio, ENETC_PCS_PHY_ADDR, devaddr, reg);
}

int xpcs_write(struct udevice *dev, int devaddr, u32 reg, u16 val)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	return enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, devaddr, reg, val);
}

int xpcs_phy_read(struct udevice *dev, int devaddr, u32 reg)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	return enetc_mdio_read(&priv->imdio, ENETC_NON_PCS_PHY_ADDR, devaddr, reg);
}

int xpcs_phy_write(struct udevice *dev, int devaddr, u32 reg, u16 val)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	return enetc_mdio_write(&priv->imdio, ENETC_NON_PCS_PHY_ADDR, devaddr, reg, val);
}

int xpcs_phy_read_pma(struct udevice *dev, u32 reg)
{
	return xpcs_read(dev, MDIO_MMD_PMAPMD, XPCS_PHY_REG(reg));
}

int xpcs_phy_write_pma(struct udevice *dev, int reg, u16 val)
{
	return xpcs_write(dev, MDIO_MMD_PMAPMD, XPCS_PHY_REG(reg), val);
}

int xpcs_phy_usxgmii_init_seq_2(struct udevice *dev)
{
	ulong begin;
	u16 val;

	/* Seq 2.1 Keep preamble data */
	val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DEBUG_CTRL));
	val |= PCS_DEBUG_CTRL_TX_PMBL_CTL;
	xpcs_write(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DEBUG_CTRL), val);

	/* Seq 2.2 Power up MPLLA to P1 state */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL);
	val = u16_replace_bits(val, 2, PMA_POWER_STATE_CTRL_TX0_PSTATE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL);
	val |= PMA_MPLL_CMN_CTRL_MPLL_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL, val);

	/* Seq 2.3 Assert request of transmitand receive */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
	val |= PMA_TX_GENCTRL2_TX_REQ_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
	val |= PMA_RX_GENCTRL2_RX_REQ_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2, val);

	/* Seq 2.4 Poll for acknowledge */
	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (val & PMA_TX_GENCTRL2_TX_REQ_0);

	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (val & PMA_RX_GENCTRL2_RX_REQ_0);

	/* Seq 2.5 Turn transmit to P0 state */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL);
	val = u16_replace_bits(val, 0, PMA_POWER_STATE_CTRL_TX0_PSTATE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0);
	val &= ~PMA_TX_GENCTRL0_TX_RST_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL);
	val &= ~PMA_POWER_STATE_CTRL_TX_DISABLE_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL);
	val |= PMA_MPLL_CMN_CTRL_MPLL_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL, val);

	/* Seq 2.6 Turn receive to P0 state */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1);
	val &= ~PMA_RX_GENCTRL1_RX_RST_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL);
	val &= ~PMA_RX_POWER_STATE_CTRL_RX0_PSTATE_MASK;
	val &= ~PMA_RX_POWER_STATE_CTRL_RX_DISABLE_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL);
	val &= ~PMA_RX_POWER_STATE_CTRL_RX0_PSTATE_MASK;
	val &= ~PMA_RX_POWER_STATE_CTRL_RX_DISABLE_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL, val);

	/* Seq 2.7 Enable transmitter output driver in the PHY */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0);
	val |= PMA_TX_GENCTRL0_TX_DT_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0, val);

	/* Seq 2.8 Enable receiver data output from PHY */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL0);
	val |= PMA_RX_GENCTRL0_RX_DT_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL0, val);

	/* Seq 2.9 Assert request of transmit and receive */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
	val |= PMA_TX_GENCTRL2_TX_REQ_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
	val |= PMA_RX_GENCTRL2_RX_REQ_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2, val);

	/* Seq 2.10 Poll for acknowledge */
	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
		schedule();
	} while (val & PMA_TX_GENCTRL2_TX_REQ_0);

	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
		schedule();
	} while (val & PMA_RX_GENCTRL2_RX_REQ_0);

	return 0;

timeout:
	return -ETIMEDOUT;
}

void xpcs_phy_reg_lock(struct udevice *dev)
{
	u16 val;
	ulong begin;

	if (xpcs_phy_read(dev, XPCS_PHY_MAC_ADAPTER, XPCS_PHY_REG(MAC_ADAPTER_LOCK_PHY)) & MAC_ADAPTER_LOCK_LOCK)
		return;

	xpcs_phy_write(dev, XPCS_PHY_MAC_ADAPTER, XPCS_PHY_REG(MAC_ADAPTER_LOCK_PHY), MAC_ADAPTER_LOCK_LOCK);
	xpcs_phy_write(dev, XPCS_PHY_MAC_ADAPTER, XPCS_PHY_REG(MAC_ADAPTER_LOCK_MPLLA), MAC_ADAPTER_LOCK_LOCK);
	xpcs_phy_write(dev, XPCS_PHY_MAC_ADAPTER, XPCS_PHY_REG(MAC_ADAPTER_LOCK_MPLLB), MAC_ADAPTER_LOCK_LOCK);
	xpcs_phy_write(dev, XPCS_PHY_MAC_ADAPTER, XPCS_PHY_REG(MAC_ADAPTER_LOCK_ROM), MAC_ADAPTER_LOCK_LOCK);
	xpcs_phy_write(dev, XPCS_PHY_MAC_ADAPTER, XPCS_PHY_REG(MAC_ADAPTER_LOCK_RAM), MAC_ADAPTER_LOCK_LOCK);

	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_SRAM);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (!(val & PMA_SRAM_INIT_DN));

	/* Work around */
	// xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_SRAM, PMA_SRAM_EXT_LD_DN);
	xpcs_phy_write(dev, XPCS_PHY_GLOBAL, XPCS_PHY_REG(GLOBAL_CTRL_EX_0), GLOBAL_CTRL_EX_0_PHY_SRAM_BYPASS);

	begin = get_timer(0);
	do {
		val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_CTRL1));
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (val & PCS_CTRL1_RESET);

	mdelay(1);

timeout:
	return;
}

int xpcs_phy_usxgmii_pma_config(struct udevice *dev)
{
	ulong begin;
	u16 val;

	xpcs_phy_reg_lock(dev);

	/* 1.6 Turn off C37 auto-negotiation */
	val = xpcs_read(dev, MDIO_MMD_VEND2, XPCS_PHY_REG(MII_CTRL));
	val &= ~MII_CTRL_AN_ENABLE;
	xpcs_write(dev, MDIO_MMD_VEND2, XPCS_PHY_REG(MII_CTRL), val);

	/* 1.7 Assert tx_reset and rx_reset*/
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0);
	val |= PMA_TX_GENCTRL0_TX_RST_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1);
	val |= PMA_RX_GENCTRL1_RX_RST_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1, val);

	/* 1.8 Wait for more than 1us */
	udelay(5);

	/* 1.9 Deassert tx_reset and rx_reset*/
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0);
	val &= ~PMA_TX_GENCTRL0_TX_RST_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1);
	val &= ~PMA_RX_GENCTRL1_RX_RST_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1, val);

	/* 1.10 Power down MPLLA */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL);
	val = u16_replace_bits(val, 3, PMA_POWER_STATE_CTRL_TX0_PSTATE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL);
	val &= ~PMA_MPLL_CMN_CTRL_MPLL_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0);
	val &= ~PMA_TX_GENCTRL0_TX_DT_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL0, val);

	/* 1.11 Change RX0 power state to P2 */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL0);
	val &= ~PMA_RX_GENCTRL0_RX_DT_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL0, val);

	/* TODO: check if it is needed */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL);
	val = u16_replace_bits(val, 1, PMA_RX_POWER_STATE_CTRL_RX0_PSTATE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL);
	val = u16_replace_bits(val, 3, PMA_RX_POWER_STATE_CTRL_RX0_PSTATE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL, val);

	/* 1.12 Assert request of transmit and receive */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
	val |= PMA_TX_GENCTRL2_TX_REQ_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
	val |= PMA_RX_GENCTRL2_RX_REQ_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2, val);

	/* 1.13 Poll for acknlowledge */
	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (val & PMA_TX_GENCTRL2_TX_REQ_0);

	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (val & PMA_RX_GENCTRL2_RX_REQ_0);

	/* 2 Config MPLL for 10G XGMII */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_REF_CLK_CTRL);
	val = u16_replace_bits(val, 6, PMA_REF_CLK_CTRL_REF_RANGE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_REF_CLK_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_REF_CLK_CTRL);
	val &= ~PMA_REF_CLK_CTRL_REF_CLK_DIV2;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_REF_CLK_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_REF_CLK_CTRL);
	val |= PMA_REF_CLK_CTRL_REF_MPLLA_DIV2;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_REF_CLK_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val &= ~PMA_MPLLA_CTRL2_MPLLA_DIV8_CLK_EN;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val |= PMA_MPLLA_CTRL2_MPLLA_DIV10_CLK_EN;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val |= PMA_MPLLA_CTRL2_MPLLA_DIV16P5_CLK_EN;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val &= ~PMA_MPLLA_CTRL2_MPLLA_TX_CLK_DIV_MASK;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val |= PMA_MPLLA_CTRL2_MPLLA_DIV_CLK_EN;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val = u16_replace_bits(val, 5, PMA_MPLLA_CTRL2_MPLLA_DIV_MULT_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_MPLLA_CTRL1);
	val &= ~PMA_MPLLA_CTRL1_MPLLA_SSC_EN;
	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_MPLLA_CTRL1);
	val &= ~PMA_MPLLA_CTRL1_MPLLA_SSC_CLK_SEL;
	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_MPLLA_CTRL5);
	val &= ~PMA_MPLLA_CTRL5_MPLLA_SSC_FRQ_CNT_PK_MASK;
	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL5, val);

	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL4, 0);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_MPLLA_CTRL5);
	val &= ~PMA_MPLLA_CTRL5_MPLLA_SSC_SPD_EN;
	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL5, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_MPLLA_CTRL1);
	val &= ~PMA_MPLLA_CTRL1_MPLLA_FRACN_CTRL_MASK;
	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL0);
	val = u16_replace_bits(val, 33, PMA_MPLLA_CTRL0_MPLLA_MULTIPLIER_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1);
	val = u16_replace_bits(val, 5, PMA_TX_GENCTRL1_VBOOST_LVL_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1, val);

	val = PMA_MPLLA_CTRL3_MPLLA_BANDWIDTH(0xA016);
	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL3, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_MISC_CTRL0);
	val = u16_replace_bits(val, 0x11, PMA_MISC_CTRL0_RX_VREF_CTRL_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_MISC_CTRL0, val);

	val = PMA_MISC_CTRL2_SUP_MISC(1);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_MISC_CTRL2, val);

	val = PMA_VCO_CAL_REF0_VCO_REF_LD_0(0x29);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_VCO_CAL_REF0, val);

	val = PMA_VCO_CAL_LD0_VCO_LD_VAL_0(0x549);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_VCO_CAL_LD0, val);

	val = PMA_RX_PPM_CTRL0_RX0_CDR_PPM_MAX(0x12);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_PPM_CTRL0, val);

	/* 3 Configure LANE0 for 10G XGMII */
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_TX_MISC_CTRL0, 0x0);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_RATE_CTRL, 0x0);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL);
	val &= ~PMA_MPLL_CMN_CTRL_MPLLB_SEL_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_MPLL_CMN_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
	val = u16_replace_bits(val, 3, PMA_TX_GENCTRL2_TX0_WIDTH_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1);
	val |= PMA_TX_GENCTRL1_VBOOST_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1, val);

	val = PMA_TX_BOOST_CTRL_TX0_IBOOST(0xf);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_BOOST_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_EQ_CTRL0);
	val = u16_replace_bits(val, 0, PMA_TX_EQ_CTRL0_TX_EQ_PRE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_EQ_CTRL0, val);

	val = PMA_TX_EQ_CTRL1_TX_EQ_POST(0x20);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_EQ_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_EQ_CTRL0);
	val = u16_replace_bits(val, 0x20, PMA_TX_EQ_CTRL0_TX_EQ_MAIN_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_EQ_CTRL0, val);

	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_RATE_CTRL, 0x0);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0);
	val = u16_replace_bits(val, 0x2, PMA_RX_EQ_CTRL0_CTLE_POLE_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0);
	val = u16_replace_bits(val, 0x10, PMA_RX_EQ_CTRL0_CTLE_BOOST_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL3);
	val = u16_replace_bits(val, 0x7, PMA_RX_GENCTRL3_LOS_TRSHLD_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL3, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_RX_CDR_CTRL1);
	val |= PMA_RX_CDR_CTRL1_VCO_STEP_CTRL_0;
	xpcs_phy_write_pma(dev, PMA_MP_16G_RX_CDR_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_RX_CDR_CTRL1);
	val |= PMA_RX_CDR_CTRL1_VCO_TEMP_COMP_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_16G_RX_CDR_CTRL1, val);

	val = PMA_RX_MISC_CTRL0_RX0_MISC(0x12);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_MISC_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
	val = u16_replace_bits(val, 0x3, PMA_RX_GENCTRL2_RX0_WIDTH_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1);
	val |= PMA_RX_GENCTRL1_RX_DIV16P5_CLK_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1, val);

	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_CDR_CTRL, 0x0);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL3);
	val &= ~PMA_RX_GENCTRL3_LOS_LFPS_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL3, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_GENCTRL4);
	val &= ~PMA_RX_GENCTRL4_RX_DFE_BYP_0;
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_GENCTRL4, val);

	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_ATTN_CTRL, 0x0);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0);
	val = u16_replace_bits(val, 0x5, PMA_RX_EQ_CTRL0_VGA1_GAIN_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0);
	val = u16_replace_bits(val, 0x5, PMA_RX_EQ_CTRL0_VGA2_GAIN_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0, val);

	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_DFE_TAP_CTRL0, 0x0);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_RX_CDR_CTRL1);
	val = u16_replace_bits(val, 0x1, PMA_RX_CDR_CTRL1_VCO_FRQBAND_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_RX_CDR_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1);
	val |= PMA_RX_GENCTRL1_RX_TERM_ACDC_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_IQ_CTRL0);
	val = u16_replace_bits(val, 0x0, PMA_RX_IQ_CTRL0_RX0_DELTA_IQ_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_IQ_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL5);
	val &= ~PMA_RX_EQ_CTRL5_RX_ADPT_SEL_0;
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL5, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL5);
	val = u16_replace_bits(val, 0x3, PMA_RX_EQ_CTRL5_RX0_ADPT_MODE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL5, val);

	/* 4 Configure XPCS for 10G XGMII */
	xpcs_write(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_CTRL2), 0x0);

	val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DIG_CTRL1));
	val |= PCS_DIG_CTRL1_USXG_EN;
	xpcs_write(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DIG_CTRL1), val);

	val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_KR_CTRL1));
	val = u16_replace_bits(val, 0x0, PCS_KR_CTRL1_USXG_MODE_MASK);
	xpcs_write(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_KR_CTRL1), val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL0);
	val = u16_replace_bits(val, 0x21, PMA_MPLLA_CTRL0_MPLLA_MULTIPLIER_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL0, val);

	val = PMA_MPLLA_CTRL3_MPLLA_BANDWIDTH(0xA016);
	xpcs_phy_write_pma(dev, PMA_MP_16G_MPLLA_CTRL3, val);

	val = PMA_VCO_CAL_LD0_VCO_LD_VAL_0(0x549);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_VCO_CAL_LD0, val);

	val = PMA_VCO_CAL_REF0_VCO_REF_LD_0(0x29);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_VCO_CAL_REF0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_EQ_CTRL4);
	val |= PMA_RX_EQ_CTRL4_CONT_ADAPT_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_EQ_CTRL4, val);

	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_RATE_CTRL, 0x0);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_RATE_CTRL, 0x0);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2);
	val = u16_replace_bits(val, 0x3, PMA_TX_GENCTRL2_TX0_WIDTH_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_TX_GENCTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
	val = u16_replace_bits(val, 0x3, PMA_RX_GENCTRL2_RX0_WIDTH_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val |= PMA_MPLLA_CTRL2_MPLLA_DIV16P5_CLK_EN;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val |= PMA_MPLLA_CTRL2_MPLLA_DIV10_CLK_EN;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2);
	val &= ~PMA_MPLLA_CTRL2_MPLLA_DIV8_CLK_EN;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_MPLLA_CTRL2, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1);
	val |= PMA_TX_GENCTRL1_VBOOST_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0);
	val = u16_replace_bits(val, 0x10, PMA_RX_EQ_CTRL0_CTLE_BOOST_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_RX_CDR_CTRL1);
	val |= PMA_RX_CDR_CTRL1_VCO_STEP_CTRL_0;
	xpcs_phy_write_pma(dev, PMA_MP_16G_RX_CDR_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_RX_CDR_CTRL1);
	val |= PMA_RX_CDR_CTRL1_VCO_TEMP_COMP_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_16G_RX_CDR_CTRL1, val);

	val = PMA_RX_MISC_CTRL0_RX0_MISC(0x12);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_MISC_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_GENCTRL4);
	val &= ~PMA_RX_GENCTRL4_RX_DFE_BYP_0;
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_GENCTRL4, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_RX_CDR_CTRL1);
	val = u16_replace_bits(val, 0x1, PMA_RX_CDR_CTRL1_VCO_FRQBAND_0_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_RX_CDR_CTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_IQ_CTRL0);
	val = u16_replace_bits(val, 0x0, PMA_RX_IQ_CTRL0_RX0_DELTA_IQ_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_IQ_CTRL0, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL5);
	val = u16_replace_bits(val, 0x3, PMA_RX_EQ_CTRL5_RX0_ADPT_MODE_MASK);
	xpcs_phy_write_pma(dev, PMA_MP_16G_25G_RX_EQ_CTRL5, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1);
	val &= ~PMA_TX_GENCTRL1_TX_CLK_RDY_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1, val);

	/* 5 Assert soft reset */
	val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DIG_CTRL1));
	val |= PCS_DIG_CTRL1_VR_RST;
	xpcs_write(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DIG_CTRL1), val);

	/* 6 Poll for SRAM initialization done */
	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_SRAM);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (!(val & PMA_SRAM_INIT_DN));

	/* 7 Assert SRAM external loading done */
	/* Workaround */
	// xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_SRAM, PMA_SRAM_EXT_LD_DN);
	xpcs_phy_write(dev, XPCS_PHY_GLOBAL, XPCS_PHY_REG(GLOBAL_CTRL_EX_0), GLOBAL_CTRL_EX_0_PHY_SRAM_BYPASS);

	/* 8 Poll for vendor-specific soft reset */
	begin = get_timer(0);
	do {
		val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DIG_CTRL1));
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (val & PCS_DIG_CTRL1_VR_RST);

	/* 9 Turn receive to P0 state */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1);
	val &= ~PMA_RX_GENCTRL1_RX_RST_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL1, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL);
	val &= ~PMA_RX_POWER_STATE_CTRL_RX_DISABLE_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL, val);

	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL);
	val &= ~PMA_RX_POWER_STATE_CTRL_RX0_PSTATE_MASK;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL, val);

	/* 10 Enable receiver data output from PHY */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL0);
	val |= PMA_RX_GENCTRL0_RX_DT_EN_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_GENCTRL0, val);

	/* 11 Assert request of receive */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
	val |= PMA_RX_GENCTRL2_RX_REQ_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2, val);

	/* 11.1 Poll for acknowledge */
	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_RX_GENCTRL2);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (val & PMA_RX_GENCTRL2_RX_REQ_0);

	/* 12 Assert TX0 clock is active and stable */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1);
	val |= PMA_TX_GENCTRL1_TX_CLK_RDY_0;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_TX_GENCTRL1, val);

	/*
	 * 13.1 Configure XPCS to consider Loss-of-Signal indicated by the
	 * PHY while evaluating the receive link status
	 */
	val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DEBUG_CTRL));
	val |= PCS_DEBUG_CTRL_SUPRESS_LOS_DET;
	xpcs_write(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DEBUG_CTRL), val);
	/*
	 * 13.2 Configure XPCS to deassert "receiver data enable" on
	 * detecting of Loss-of-Signal
	 */
	val = xpcs_read(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DEBUG_CTRL));
	val |= PCS_DEBUG_CTRL_RX_DT_EN_CTL;
	xpcs_write(dev, MDIO_MMD_PCS, XPCS_PHY_REG(PCS_DEBUG_CTRL), val);

	/* 14 Poll for DPLL lock status for Lane 0 */
	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_RX_LSTS);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (!(val & PMA_RX_LSTS_RX_VALID_0));

	/* 15 Assert request of receive adaptation */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_EQ_CTRL4);
	val |= PMA_RX_EQ_CTRL4_RX_AD_REQ;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_EQ_CTRL4, val);

	/* 16 Poll for acknowledge */
	begin = get_timer(0);
	do {
		val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_MISC_STS);
		if (get_timer(begin) > 500) {
			dev_err(dev, "Polling timeout, line: %d\n", __LINE__);
			goto timeout;
		}
		mdelay(10);
	} while (!(val & PMA_MISC_STS_RX_ADPT_ACK));

	/* 17 Deassert request of receive adaptation */
	val = xpcs_phy_read_pma(dev, PMA_MP_12G_16G_25G_RX_EQ_CTRL4);
	val &= ~PMA_RX_EQ_CTRL4_RX_AD_REQ;
	xpcs_phy_write_pma(dev, PMA_MP_12G_16G_25G_RX_EQ_CTRL4, val);

	/* 18 Set the value of Config_Reg to 0 for Clause 37 autonegotiation. */
	val = xpcs_read(dev, MDIO_MMD_VEND2, XPCS_PHY_REG(MII_AN_CTRL));
	val &= ~MII_AN_CTRL_TX_CONFIG;
	xpcs_write(dev, MDIO_MMD_VEND2, XPCS_PHY_REG(MII_AN_CTRL), val);

	/* 19 Select XGMII speed */
	val = xpcs_read(dev, MDIO_MMD_VEND2, XPCS_PHY_REG(MII_CTRL));
	val &= ~MII_CTRL_SS5;
	val |= MII_CTRL_SS6 | MII_CTRL_SS13;
	xpcs_write(dev, MDIO_MMD_VEND2, XPCS_PHY_REG(MII_CTRL), val);

	val = xpcs_phy_usxgmii_init_seq_2(dev);
	if (val)
		return val;

	return 0;

timeout:
	return -ETIMEDOUT;
}

u32 xpcs_phy_get_id(struct udevice *dev)
{
	int ret;
	u32 id;

	/* First, search C73 PCS using PCS MMD */
	ret = xpcs_phy_read(dev, XPCS_PHY_GLOBAL, XPCS_PHY_REG(IDCODE_HI));
	if (ret < 0)
		return 0xffffffff;

	id = ret << 16;

	ret = xpcs_phy_read(dev, XPCS_PHY_GLOBAL, XPCS_PHY_REG(IDCODE_LO));
	if (ret < 0)
		return 0xffffffff;

	/* If Device IDs are not all zeros or all ones,
	 * we found C73 AN-type device
	 */
	if ((id | ret) && (id | ret) != 0xffffffff)
		return id | ret;

	return 0xffffffff;
}
