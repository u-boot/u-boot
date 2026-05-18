// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <generic-phy.h>
#include <reset.h>
#include <power/regulator.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/err.h>

#include "phy-qcom-qmp-common.h"

#include "phy-qcom-qmp.h"
#include "phy-qcom-qmp-pcs-misc-v3.h"
#include "phy-qcom-qmp-pcs-usb-v4.h"
#include "phy-qcom-qmp-dp-com-v3.h"

/* QPHY_V3_DP_COM_RESET_OVRD_CTRL register bits */
/* DP PHY soft reset */
#define SW_DPPHY_RESET                          BIT(0)
/* mux to select DP PHY reset control, 0:HW control, 1: software reset */
#define SW_DPPHY_RESET_MUX                      BIT(1)
/* USB3 PHY soft reset */
#define SW_USB3PHY_RESET                        BIT(2)
/* mux to select USB3 PHY reset control, 0:HW control, 1: software reset */
#define SW_USB3PHY_RESET_MUX                    BIT(3)

/* QPHY_V3_DP_COM_PHY_MODE_CTRL register bits */
#define USB3_MODE                               BIT(0) /* enables USB3 mode */
#define DP_MODE                                 BIT(1) /* enables DP mode */

/* QPHY_V3_DP_COM_TYPEC_CTRL register bits */
#define SW_PORTSELECT_MUX                       BIT(1)

/* PHY slot identifiers for device tree phandle arguments */
#define QMP_USB43DP_USB3_PHY    0
#define QMP_USB43DP_DP_PHY      1

#define PHY_INIT_COMPLETE_TIMEOUT		10000

struct qmp_combo_offsets {
	u16 com;
	u16 txa;
	u16 rxa;
	u16 txb;
	u16 rxb;
	u16 usb3_serdes;
	u16 usb3_pcs_misc;
	u16 usb3_pcs;
	u16 usb3_pcs_usb;
};

/*
 * Initialisation tables
 */

static const struct qmp_combo_offsets qmp_combo_offsets_v3 = {
	.com		= 0x0000,
	.txa		= 0x1200,
	.rxa		= 0x1400,
	.txb		= 0x1600,
	.rxb		= 0x1800,
	.usb3_serdes	= 0x1000,
	.usb3_pcs_misc	= 0x1a00,
	.usb3_pcs	= 0x1c00,
	.usb3_pcs_usb	= 0x1f00,
};

static const struct qmp_phy_init_tbl sm8150_usb3_serdes_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SSC_EN_CENTER, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SSC_PER1, 0x31),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SSC_PER2, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SSC_STEP_SIZE1_MODE0, 0xde),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SSC_STEP_SIZE2_MODE0, 0x07),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SSC_STEP_SIZE1_MODE1, 0xde),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SSC_STEP_SIZE2_MODE1, 0x07),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SYSCLK_BUF_ENABLE, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_CMN_IPTRIM, 0x20),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_CP_CTRL_MODE0, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_CP_CTRL_MODE1, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_PLL_RCTRL_MODE0, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_PLL_RCTRL_MODE1, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_PLL_CCTRL_MODE0, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_PLL_CCTRL_MODE1, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_SYSCLK_EN_SEL, 0x1a),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_LOCK_CMP_EN, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_LOCK_CMP1_MODE0, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_LOCK_CMP2_MODE0, 0x34),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_LOCK_CMP1_MODE1, 0x34),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_LOCK_CMP2_MODE1, 0x82),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DEC_START_MODE0, 0x82),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DEC_START_MODE1, 0x82),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DIV_FRAC_START1_MODE0, 0xab),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DIV_FRAC_START2_MODE0, 0xea),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DIV_FRAC_START3_MODE0, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_VCO_TUNE_MAP, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DIV_FRAC_START1_MODE1, 0xab),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DIV_FRAC_START2_MODE1, 0xea),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_DIV_FRAC_START3_MODE1, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_VCO_TUNE1_MODE0, 0x24),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_VCO_TUNE1_MODE1, 0x24),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_VCO_TUNE2_MODE1, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_HSCLK_SEL, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_CORECLK_DIV_MODE1, 0x08),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_BIN_VCOCAL_CMP_CODE1_MODE0, 0xca),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_BIN_VCOCAL_CMP_CODE2_MODE0, 0x1e),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_BIN_VCOCAL_CMP_CODE1_MODE1, 0xca),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_BIN_VCOCAL_CMP_CODE2_MODE1, 0x1e),
	QMP_PHY_INIT_CFG(QSERDES_V4_COM_BIN_VCOCAL_HSCLK_SEL, 0x11),
};

static const struct qmp_phy_init_tbl sm8250_usb3_tx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V4_TX_RES_CODE_LANE_TX, 0x60),
	QMP_PHY_INIT_CFG(QSERDES_V4_TX_RES_CODE_LANE_RX, 0x60),
	QMP_PHY_INIT_CFG(QSERDES_V4_TX_RES_CODE_LANE_OFFSET_TX, 0x11),
	QMP_PHY_INIT_CFG(QSERDES_V4_TX_RES_CODE_LANE_OFFSET_RX, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V4_TX_LANE_MODE_1, 0xd5),
	QMP_PHY_INIT_CFG(QSERDES_V4_TX_RCV_DETECT_LVL_2, 0x12),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V4_TX_PI_QEC_CTRL, 0x40, 1),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V4_TX_PI_QEC_CTRL, 0x54, 2),
};

static const struct qmp_phy_init_tbl sm8250_usb3_rx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_SO_GAIN, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_FASTLOCK_FO_GAIN, 0x2f),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_SO_SATURATION_AND_ENABLE, 0x7f),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_FASTLOCK_COUNT_LOW, 0xff),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_FASTLOCK_COUNT_HIGH, 0x0f),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_PI_CONTROLS, 0x99),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_SB2_THRESH1, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_SB2_THRESH2, 0x08),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_SB2_GAIN1, 0x05),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_UCDR_SB2_GAIN2, 0x05),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_VGA_CAL_CNTRL1, 0x54),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_VGA_CAL_CNTRL2, 0x0c),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_EQU_ADAPTOR_CNTRL2, 0x0f),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_EQU_ADAPTOR_CNTRL3, 0x4a),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_EQU_ADAPTOR_CNTRL4, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_IDAC_TSETTLE_LOW, 0xc0),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_IDAC_TSETTLE_HIGH, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1, 0x77),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_SIGDET_CNTRL, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_SIGDET_DEGLITCH_CNTRL, 0x0e),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V4_RX_RX_MODE_00_LOW, 0xff, 1),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V4_RX_RX_MODE_00_LOW, 0x7f, 2),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V4_RX_RX_MODE_00_HIGH, 0x7f, 1),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V4_RX_RX_MODE_00_HIGH, 0xff, 2),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_00_HIGH2, 0x7f),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_00_HIGH3, 0x7f),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_00_HIGH4, 0x97),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_01_LOW, 0xdc),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_01_HIGH, 0xdc),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_01_HIGH2, 0x5c),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_01_HIGH3, 0x7b),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_RX_MODE_01_HIGH4, 0xb4),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_DFE_EN_TIMER, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_DFE_CTLE_POST_CAL_OFFSET, 0x38),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_AUX_DATA_TCOARSE_TFINE, 0xa0),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_DCC_CTRL1, 0x0c),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_GM_CAL, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V4_RX_VTH_CODE, 0x10),
};

static const struct qmp_phy_init_tbl sm8250_usb3_pcs_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_LOCK_DETECT_CONFIG1, 0xd0),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_LOCK_DETECT_CONFIG2, 0x07),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_LOCK_DETECT_CONFIG3, 0x20),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_LOCK_DETECT_CONFIG6, 0x13),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_REFGEN_REQ_CONFIG1, 0x21),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_RX_SIGDET_LVL, 0xa9),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_CDR_RESET_TIME, 0x0a),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_ALIGN_DETECT_CONFIG1, 0x88),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_ALIGN_DETECT_CONFIG2, 0x13),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_PCS_TX_RX_CONFIG, 0x0c),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_EQ_CONFIG1, 0x4b),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_EQ_CONFIG5, 0x10),
};

static const struct qmp_phy_init_tbl sm8250_usb3_pcs_usb_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_USB3_LFPS_DET_HIGH_COUNT_VAL, 0xf8),
	QMP_PHY_INIT_CFG(QPHY_V4_PCS_USB3_RXEQTRAINING_DFE_TIME_S2, 0x07),
};

struct qmp_phy_cfg {
	const struct qmp_combo_offsets *offsets;
	const struct qmp_phy_init_tbl *serdes_tbl;
	int serdes_tbl_num;
	const struct qmp_phy_init_tbl *tx_tbl;
	int tx_tbl_num;
	const struct qmp_phy_init_tbl *rx_tbl;
	int rx_tbl_num;
	const struct qmp_phy_init_tbl *pcs_tbl;
	int pcs_tbl_num;
	const struct qmp_phy_init_tbl *pcs_usb_tbl;
	int pcs_usb_tbl_num;
	const char * const *vreg_list;
	int num_vregs;
	/* true, if PHY needs delay after POWER_DOWN */
	bool has_pwrdn_delay;
};

/* list of clocks required by phy */
static const char * const qmp_combo_phy_clk_l[] = {
	"aux", "com_aux",
};

/* list of regulators */
static const char * const qmp_phy_vreg_l[] = {
	"vdda-phy-supply",
	"vdda-pll-supply",
};

struct qmp_combo {
	struct udevice *dev;
	void __iomem *com;
	void __iomem *serdes;
	void __iomem *tx;
	void __iomem *rx;
	void __iomem *tx2;
	void __iomem *rx2;
	void __iomem *pcs;
	void __iomem *pcs_usb;
	void __iomem *pcs_misc;
	struct clk *clks;
	struct clk *pipe_clk;
	int num_clks;
	struct reset_ctl_bulk resets;
	int num_resets;
	struct udevice **vregs;
	int num_vregs;
	const struct qmp_phy_cfg *cfg;
};

static inline void qphy_setbits(void __iomem *base, u32 offset, u32 val)
{
	u32 reg;

	reg = readl(base + offset);
	reg |= val;
	writel(reg, base + offset);

	/* ensure that above write is through */
	readl(base + offset);
}

static inline void qphy_clrbits(void __iomem *base, u32 offset, u32 val)
{
	u32 reg;

	reg = readl(base + offset);
	reg &= ~val;
	writel(reg, base + offset);

	/* ensure that above write is through */
	readl(base + offset);
}

static int qmp_combo_com_exit(struct qmp_combo *qmp)
{
	int i, ret;

	for (i = 0; i < qmp->num_clks; i++)
		clk_disable(&qmp->clks[i]);

	reset_assert_bulk(&qmp->resets);

	for (i = qmp->num_vregs - 1; i >= 0; i--) {
		ret = regulator_set_enable(qmp->vregs[i], false);
		if (ret)
			dev_warn(qmp->dev, "failed to disable %s: %d\n",
				 qmp->cfg->vreg_list[i], ret);
	}

	return 0;
}

static int qmp_combo_com_init(struct qmp_combo *qmp)
{
	void __iomem *com = qmp->com;
	void __iomem *pcs = qmp->pcs;
	u32 val;
	int ret, i;

	ret = reset_assert_bulk(&qmp->resets);
	if (ret) {
		printf("Failed to assert resets: %d\n", ret);
		return ret;
	}

	ret = reset_deassert_bulk(&qmp->resets);
	if (ret) {
		printf("Failed to deassert resets: %d\n", ret);
		return ret;
	}

	for (i = 0; i < qmp->num_vregs; i++) {
		ret = regulator_set_enable(qmp->vregs[i], true);
		if (ret) {
			dev_err(qmp->dev, "Failed to enable regulator %d: %d\n", i, ret);
			while (--i >= 0)
				regulator_set_enable(qmp->vregs[i], false);
			reset_assert_bulk(&qmp->resets);
			return ret;
		}
	}

	for (i = 0; i < qmp->num_clks; i++) {
		ret = clk_enable(&qmp->clks[i]);
		if (ret) {
			printf("Failed to enable clock %d: %d\n", i, ret);
			while (--i >= 0)
				clk_disable(&qmp->clks[i]);
			for (i = qmp->num_vregs - 1; i >= 0; i--)
				regulator_set_enable(qmp->vregs[i], false);
			reset_assert_bulk(&qmp->resets);
			return ret;
		}
	}

	/* Common block register writes */
	qphy_setbits(com, QPHY_V3_DP_COM_POWER_DOWN_CTRL, SW_PWRDN);
	qphy_setbits(com, QPHY_V3_DP_COM_RESET_OVRD_CTRL,
		     SW_DPPHY_RESET_MUX | SW_DPPHY_RESET |
		     SW_USB3PHY_RESET_MUX | SW_USB3PHY_RESET);

	val = SW_PORTSELECT_MUX;
	writel(val, com + QPHY_V3_DP_COM_TYPEC_CTRL);

	writel(USB3_MODE | DP_MODE, com + QPHY_V3_DP_COM_PHY_MODE_CTRL);

	qphy_clrbits(com, QPHY_V3_DP_COM_RESET_OVRD_CTRL,
		     SW_DPPHY_RESET_MUX | SW_DPPHY_RESET |
		     SW_USB3PHY_RESET_MUX | SW_USB3PHY_RESET);

	qphy_clrbits(com, QPHY_V3_DP_COM_SWI_CTRL, 0x03);

	qphy_clrbits(com, QPHY_V3_DP_COM_SW_RESET, SW_RESET);

	qphy_setbits(pcs, QPHY_V4_PCS_POWER_DOWN_CONTROL, SW_PWRDN);

	return 0;
}

static int qmp_combo_usb_power_on(struct qmp_combo *qmp)
{
	const struct qmp_phy_cfg *cfg = qmp->cfg;
	void __iomem *serdes = qmp->serdes;
	void __iomem *tx = qmp->tx;
	void __iomem *rx = qmp->rx;
	void __iomem *tx2 = qmp->tx2;
	void __iomem *rx2 = qmp->rx2;
	void __iomem *pcs = qmp->pcs;
	void __iomem *pcs_usb = qmp->pcs_usb;
	u32 val;
	int ret;

	/* Serdes configuration */
	qmp_configure(qmp->dev, serdes, cfg->serdes_tbl, cfg->serdes_tbl_num);

	ret = clk_prepare_enable(qmp->pipe_clk);
	if (ret) {
		dev_err(qmp->dev, "pipe_clk enable failed err=%d\n", ret);
		return ret;
	}

	/* Tx, Rx configurations */
	qmp_configure_lane(qmp->dev, tx, cfg->tx_tbl, cfg->tx_tbl_num, 1);
	qmp_configure_lane(qmp->dev, tx2, cfg->tx_tbl, cfg->tx_tbl_num, 2);

	qmp_configure_lane(qmp->dev, rx, cfg->rx_tbl, cfg->rx_tbl_num, 1);
	qmp_configure_lane(qmp->dev, rx2, cfg->rx_tbl, cfg->rx_tbl_num, 2);

	/* PCS configuration */
	qmp_configure(qmp->dev, pcs, cfg->pcs_tbl, cfg->pcs_tbl_num);

	if (pcs_usb) {
		qmp_configure(qmp->dev, pcs_usb,
			      cfg->pcs_usb_tbl,
			      cfg->pcs_usb_tbl_num);
	}

	if (cfg->has_pwrdn_delay)
		udelay(20);

	/* Pull PHY out of reset */
	qphy_clrbits(pcs, QPHY_V4_PCS_SW_RESET, SW_RESET);

	/* Start SerDes and Phy-Coding-Sublayer */
	qphy_setbits(pcs, QPHY_V4_PCS_START_CONTROL,
		     SERDES_START | PCS_START);

	/* Wait for PHY initialization */
	ret = readl_poll_timeout(pcs + QPHY_V4_PCS_PCS_STATUS1, val,
				 !(val & PHYSTATUS), PHY_INIT_COMPLETE_TIMEOUT);

	if (ret) {
		printf("QMP USB3 PHY initialization timeout\n");
		clk_disable(qmp->pipe_clk);
		return ret;
	}

	return 0;
}

static int qmp_combo_power_on(struct phy *phy)
{
	struct qmp_combo *qmp = dev_get_priv(phy->dev);
	int ret;

	/* Initialize common block */
	ret = qmp_combo_com_init(qmp);
	if (ret)
		return ret;

	/* Initialize USB3-specific configuration */
	ret = qmp_combo_usb_power_on(qmp);
	if (ret) {
		qmp_combo_com_exit(qmp);
		return ret;
	}

	return 0;
}

static int qmp_combo_power_off(struct phy *phy)
{
	struct qmp_combo *qmp = dev_get_priv(phy->dev);
	void __iomem *com = qmp->com;

	clk_disable(qmp->pipe_clk);

	/* PHY reset */
	qphy_setbits(qmp->pcs, QPHY_V4_PCS_SW_RESET, SW_RESET);

	/* Stop SerDes and Phy-Coding-Sublayer */
	qphy_clrbits(qmp->pcs, QPHY_V4_PCS_START_CONTROL,
		     SERDES_START | PCS_START);

	/* Put PHY into POWER DOWN state: active low */
	qphy_clrbits(qmp->pcs, QPHY_V4_PCS_POWER_DOWN_CONTROL, SW_PWRDN);

	/* Power down common block */
	qphy_clrbits(com, QPHY_V3_DP_COM_POWER_DOWN_CTRL, SW_PWRDN);

	return qmp_combo_com_exit(qmp);
}

static int qmp_combo_reset_init(struct qmp_combo *qmp)
{
	struct udevice *dev = qmp->dev;
	int ret;

	ret = reset_get_bulk(dev, &qmp->resets);
	if (ret) {
		printf("Failed to get resets: %d\n", ret);
		return ret;
	}

	qmp->num_resets = qmp->resets.count;

	return 0;
}

static int qmp_combo_clk_init(struct qmp_combo *qmp)
{
	struct udevice *dev = qmp->dev;
	int num = ARRAY_SIZE(qmp_combo_phy_clk_l);
	int i, ret;

	qmp->clks = devm_kcalloc(dev, num, sizeof(*qmp->clks), GFP_KERNEL);
	if (!qmp->clks)
		return -ENOMEM;

	for (i = 0; i < num; i++) {
		ret = clk_get_by_name(dev, qmp_combo_phy_clk_l[i], &qmp->clks[i]);
		if (ret) {
			dev_err(dev, "failed to get %s clock: %d\n",
				qmp_combo_phy_clk_l[i], ret);
			return ret;
		}
	}

	qmp->num_clks = num;
	return 0;
}

static int qmp_combo_vreg_init(struct qmp_combo *qmp)
{
	const struct qmp_phy_cfg *cfg = qmp->cfg;
	struct udevice *dev = qmp->dev;
	int num = cfg->num_vregs;
	int i, ret;

	if (!num)
		return 0;

	qmp->vregs = devm_kcalloc(dev, num, sizeof(*qmp->vregs), GFP_KERNEL);
	if (!qmp->vregs)
		return -ENOMEM;

	for (i = 0; i < num; i++) {
		ret = device_get_supply_regulator(dev, cfg->vreg_list[i],
						  &qmp->vregs[i]);
		if (ret) {
			dev_err(dev, "failed to get regulator %s: %d\n",
				cfg->vreg_list[i], ret);
			return ret;
		}
	}

	qmp->num_vregs = num;
	return 0;
}

static int qmp_combo_parse_dt(struct qmp_combo *qmp)
{
	const struct qmp_phy_cfg *cfg = qmp->cfg;
	const struct qmp_combo_offsets *offs = cfg->offsets;
	struct udevice *dev = qmp->dev;
	void __iomem *base;
	int ret;

	if (!offs)
		return -EINVAL;

	base = (void __iomem *)dev_read_addr(dev);
	if (IS_ERR(base))
		return PTR_ERR(base);

	qmp->com = base + offs->com;
	qmp->serdes = base + offs->usb3_serdes;
	qmp->tx = base + offs->txa;
	qmp->rx = base + offs->rxa;
	qmp->tx2 = base + offs->txb;
	qmp->rx2 = base + offs->rxb;
	qmp->pcs = base + offs->usb3_pcs;
	qmp->pcs_usb = base + offs->usb3_pcs_usb;
	qmp->pcs_misc = base + offs->usb3_pcs_misc;

	ret = qmp_combo_clk_init(qmp);
	if (ret)
		return ret;

	qmp->pipe_clk = devm_clk_get(dev, "usb3_pipe");
	if (IS_ERR(qmp->pipe_clk)) {
		dev_err(dev, "failed to get pipe clock (%ld)\n",
			PTR_ERR(qmp->pipe_clk));
		return ret;
	}

	ret = qmp_combo_reset_init(qmp);
	if (ret)
		return ret;

	ret = qmp_combo_vreg_init(qmp);
	if (ret)
		return ret;

	return 0;
}

static int qmp_combo_probe(struct udevice *dev)
{
	struct qmp_combo *qmp = dev_get_priv(dev);
	int ret;

	qmp->dev = dev;
	qmp->cfg = (const struct qmp_phy_cfg *)dev_get_driver_data(dev);
	if (!qmp->cfg) {
		printf("Failed to get PHY configuration\n");
		return -EINVAL;
	}

	ret = qmp_combo_parse_dt(qmp);

	return ret;
}

static const struct qmp_phy_cfg sc7280_usb3dpphy_cfg = {
	.offsets		= &qmp_combo_offsets_v3,
	.serdes_tbl		= sm8150_usb3_serdes_tbl,
	.serdes_tbl_num		= ARRAY_SIZE(sm8150_usb3_serdes_tbl),
	.tx_tbl			= sm8250_usb3_tx_tbl,
	.tx_tbl_num		= ARRAY_SIZE(sm8250_usb3_tx_tbl),
	.rx_tbl			= sm8250_usb3_rx_tbl,
	.rx_tbl_num		= ARRAY_SIZE(sm8250_usb3_rx_tbl),
	.pcs_tbl		= sm8250_usb3_pcs_tbl,
	.pcs_tbl_num		= ARRAY_SIZE(sm8250_usb3_pcs_tbl),
	.pcs_usb_tbl		= sm8250_usb3_pcs_usb_tbl,
	.pcs_usb_tbl_num	= ARRAY_SIZE(sm8250_usb3_pcs_usb_tbl),
	.vreg_list		= qmp_phy_vreg_l,
	.num_vregs		= ARRAY_SIZE(qmp_phy_vreg_l),

	.has_pwrdn_delay	= true,
};

static int qmp_combo_xlate(struct phy *phy, struct ofnode_phandle_args *args)
{
	if (args->args_count != 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	/* We only support the USB3 phy at slot 0 */
	if (args->args[0] == QMP_USB43DP_DP_PHY)
		return -EINVAL;

	phy->id = QMP_USB43DP_USB3_PHY;

	return 0;
}

static struct phy_ops qmp_combo_ops = {
	.init = qmp_combo_power_on,
	.exit = qmp_combo_power_off,
	.of_xlate = qmp_combo_xlate,
};

static const struct udevice_id qmp_combo_ids[] = {
	{
		.compatible = "qcom,sc7280-qmp-usb3-dp-phy",
		.data = (ulong)&sc7280_usb3dpphy_cfg,
	},
	{ }
};

U_BOOT_DRIVER(qmp_combo) = {
	.name = "qcom-qmp-usb3-dp-phy",
	.id = UCLASS_PHY,
	.of_match = qmp_combo_ids,
	.ops = &qmp_combo_ops,
	.probe = qmp_combo_probe,
	.priv_auto = sizeof(struct qmp_combo),
};
