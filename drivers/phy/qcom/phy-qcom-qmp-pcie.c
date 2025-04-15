// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Bhupesh Sharma <bhupesh.sharma@linaro.org>
 *
 * Based on Linux driver
 */

#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <generic-phy.h>
#include <malloc.h>
#include <reset.h>
#include <power/regulator.h>

#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>

#include "phy-qcom-qmp.h"
#include "phy-qcom-qmp-pcs-misc-v3.h"
#include "phy-qcom-qmp-pcs-v5.h"
#include "phy-qcom-qmp-pcs-v6.h"
#include "phy-qcom-qmp-pcs-v6_20.h"
#include "phy-qcom-qmp-pcs-pcie-v6.h"
#include "phy-qcom-qmp-pcs-pcie-v6_20.h"
#include "phy-qcom-qmp-pcie-qhp.h"
#include "phy-qcom-qmp-qserdes-com-v6.h"
#include "phy-qcom-qmp-qserdes-txrx-v6.h"
#include "phy-qcom-qmp-qserdes-txrx-v6_20.h"
#include "phy-qcom-qmp-qserdes-ln-shrd-v6.h"

/* QPHY_SW_RESET bit */
#define SW_RESET				BIT(0)
/* QPHY_POWER_DOWN_CONTROL */
#define SW_PWRDN				BIT(0)
#define REFCLK_DRV_DSBL				BIT(1)
/* QPHY_START_CONTROL bits */
#define SERDES_START				BIT(0)
#define PCS_START				BIT(1)
/* QPHY_PCS_READY_STATUS bit */
#define PCS_READY				BIT(0)

/* QPHY_PCS_STATUS bit */
#define PHYSTATUS				BIT(6)
#define PHYSTATUS_4_20				BIT(7)

#define PHY_INIT_COMPLETE_TIMEOUT		(200 * 10000)

#define NUM_SUPPLIES	3

struct qmp_pcie_init_tbl {
	unsigned int offset;
	unsigned int val;
	/*
	 * mask of lanes for which this register is written
	 * for cases when second lane needs different values
	 */
	u8 lane_mask;
};

#define QMP_PHY_INIT_CFG(o, v)		\
	{				\
		.offset = o,		\
		.val = v,		\
		.lane_mask = 0xff,	\
	}

#define QMP_PHY_INIT_CFG_LANE(o, v, l)	\
	{				\
		.offset = o,		\
		.val = v,		\
		.lane_mask = l,		\
	}

/* set of registers with offsets different per-PHY */
enum qphy_reg_layout {
	/* PCS registers */
	QPHY_SW_RESET,
	QPHY_START_CTRL,
	QPHY_PCS_STATUS,
	QPHY_PCS_POWER_DOWN_CONTROL,
	/* Keep last to ensure regs_layout arrays are properly initialized */
	QPHY_LAYOUT_SIZE
};

static const unsigned int pciephy_v5_regs_layout[QPHY_LAYOUT_SIZE] = {
	[QPHY_SW_RESET]			= QPHY_V5_PCS_SW_RESET,
	[QPHY_START_CTRL]		= QPHY_V5_PCS_START_CONTROL,
	[QPHY_PCS_STATUS]		= QPHY_V5_PCS_PCS_STATUS1,
	[QPHY_PCS_POWER_DOWN_CONTROL]	= QPHY_V5_PCS_POWER_DOWN_CONTROL,
};

static const unsigned int pciephy_v6_regs_layout[QPHY_LAYOUT_SIZE] = {
	[QPHY_SW_RESET]			= QPHY_V6_PCS_SW_RESET,
	[QPHY_START_CTRL]		= QPHY_V6_PCS_START_CONTROL,
	[QPHY_PCS_STATUS]		= QPHY_V6_PCS_PCS_STATUS1,
	[QPHY_PCS_POWER_DOWN_CONTROL]	= QPHY_V6_PCS_POWER_DOWN_CONTROL,
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen3x2_pcie_serdes_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_EN_CENTER, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_PER1, 0x62),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_PER2, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE1_MODE0, 0xf8),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE2_MODE0, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE1_MODE1, 0x93),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE2_MODE1, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CLK_ENABLE1, 0x90),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SYS_CLK_CTRL, 0x82),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_IVCO, 0x07),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CP_CTRL_MODE0, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CP_CTRL_MODE1, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_RCTRL_MODE0, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_RCTRL_MODE1, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CCTRL_MODE0, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CCTRL_MODE1, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SYSCLK_EN_SEL, 0x08),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_BG_TIMER, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP_EN, 0x42),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP1_MODE0, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP2_MODE0, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP1_MODE1, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP2_MODE1, 0x1a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DEC_START_MODE0, 0x41),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DEC_START_MODE1, 0x34),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START1_MODE0, 0xab),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START2_MODE0, 0xaa),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START3_MODE0, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START1_MODE1, 0x55),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START2_MODE1, 0x55),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START3_MODE1, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_VCO_TUNE_MAP, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CLK_SELECT, 0x34),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_HSCLK_SEL_1, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CORECLK_DIV_MODE1, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CMN_CONFIG_1, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_ADDITIONAL_MISC_3, 0x0f),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CORE_CLK_EN, 0xa0),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen3x2_pcie_tx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_TX_LANE_MODE_1, 0x15),
	QMP_PHY_INIT_CFG(QSERDES_V6_TX_LANE_MODE_4, 0x3f),
	QMP_PHY_INIT_CFG(QSERDES_V6_TX_PI_QEC_CTRL, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_TX_RES_CODE_LANE_OFFSET_RX, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_TX_RES_CODE_LANE_OFFSET_TX, 0x18),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen3x2_pcie_rx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_DFE_CTLE_POST_CAL_OFFSET, 0x38),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_GM_CAL, 0x11),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_00_HIGH, 0xbf),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_00_HIGH2, 0xbf),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_00_HIGH3, 0xb7),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_00_HIGH4, 0xea),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_00_LOW, 0x3f),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_01_HIGH, 0x5c),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_01_HIGH2, 0x9c),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_01_HIGH3, 0x1a),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_01_HIGH4, 0x89),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_01_LOW, 0xdc),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_10_HIGH, 0x94),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_10_HIGH2, 0x5b),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_10_HIGH3, 0x1a),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_MODE_10_HIGH4, 0x89),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_TX_ADAPT_POST_THRESH, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_UCDR_FO_GAIN, 0x09),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_UCDR_SO_GAIN, 0x05),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_UCDR_SB2_THRESH1, 0x08),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_UCDR_SB2_THRESH2, 0x08),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_VGA_CAL_CNTRL2, 0x0f),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_SIDGET_ENABLES, 0x1c),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_RX_IDAC_TSETTLE_LOW, 0x07),
	QMP_PHY_INIT_CFG(QSERDES_V6_RX_SIGDET_CAL_TRIM, 0x08),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen3x2_pcie_pcs_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_V6_PCS_REFGEN_REQ_CONFIG1, 0x05),
	QMP_PHY_INIT_CFG(QPHY_V6_PCS_RX_SIGDET_LVL, 0x77),
	QMP_PHY_INIT_CFG(QPHY_V6_PCS_RATE_SLEW_CNTRL1, 0x0b),
	QMP_PHY_INIT_CFG(QPHY_V6_PCS_EQ_CONFIG2, 0x0f),
	QMP_PHY_INIT_CFG(QPHY_V6_PCS_PCS_TX_RX_CONFIG, 0x8c),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen3x2_pcie_pcs_misc_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_PCS_PCIE_EQ_CONFIG1, 0x1e),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_PCS_PCIE_RXEQEVAL_TIME, 0x27),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_PCS_PCIE_POWER_STATE_CONFIG2, 0x1d),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_PCS_PCIE_POWER_STATE_CONFIG4, 0x07),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_PCS_PCIE_ENDPOINT_REFCLK_DRIVE, 0xc1),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_PCS_PCIE_OSC_DTCT_ACTIONS, 0x00),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen4x2_pcie_serdes_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE1_MODE1, 0x26),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE2_MODE1, 0x03),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CP_CTRL_MODE1, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_RCTRL_MODE1, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CCTRL_MODE1, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CORECLK_DIV_MODE1, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP1_MODE1, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP2_MODE1, 0x1a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DEC_START_MODE1, 0x68),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START1_MODE1, 0xab),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START2_MODE1, 0xaa),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START3_MODE1, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_HSCLK_SEL_1, 0x12),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE1_MODE0, 0xf8),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE2_MODE0, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CP_CTRL_MODE0, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_RCTRL_MODE0, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CCTRL_MODE0, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CORE_CLK_DIV_MODE0, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP1_MODE0, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP2_MODE0, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DEC_START_MODE0, 0x41),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START1_MODE0, 0xab),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START2_MODE0, 0xaa),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START3_MODE0, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_HSCLK_HS_SWITCH_SEL_1, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_BG_TIMER, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_EN_CENTER, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_PER1, 0x62),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_PER2, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_POST_DIV_MUX, 0x40),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_BIAS_EN_CLK_BUFLR_EN, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CLK_ENABLE1, 0x90),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SYS_CLK_CTRL, 0x82),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_IVCO, 0x0f),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SYSCLK_EN_SEL, 0x08),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP_EN, 0x46),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP_CFG, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_VCO_TUNE_MAP, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CLK_SELECT, 0x34),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CORE_CLK_EN, 0xa0),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CMN_CONFIG_1, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CMN_MISC_1, 0x88),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CMN_MODE, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_VCO_DC_LEVEL_CTRL, 0x0f),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen4x2_pcie_ln_shrd_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RXCLK_DIV2_CTRL, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_DFE_DAC_ENABLE1, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_TX_ADAPT_POST_THRESH1, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_TX_ADAPT_POST_THRESH2, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B0, 0x12),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B1, 0x12),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B2, 0xdb),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B3, 0x9a),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B4, 0x38),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B5, 0xb6),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B6, 0x64),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH1_RATE210, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH1_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH2_RATE210, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH2_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH3_RATE210, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH3_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH4_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH5_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH6_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_SUMMER_CAL_SPD_MODE, 0x5b),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen4x2_pcie_tx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_RES_CODE_LANE_OFFSET_TX, 0x1d),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_RES_CODE_LANE_OFFSET_RX, 0x03),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_LANE_MODE_1, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_LANE_MODE_2, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_LANE_MODE_3, 0x51),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_TRAN_DRVR_EMP_EN, 0x34),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen4x2_pcie_rx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_FO_GAIN_RATE_2, 0x0c),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_FO_GAIN_RATE_3, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_SO_GAIN_RATE_2, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_PI_CONTROLS, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_SO_ACC_DEFAULT_VAL_RATE3, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_IVCM_CAL_CTRL2, 0x80),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_IVCM_POSTCAL_OFFSET, 0x7c),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_DFE_3, 0x05),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_TX_ADPT_CTRL, 0x10),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_VGA_CAL_MAN_VAL, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_GM_CAL, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_EQU_ADAPTOR_CNTRL4, 0x0b),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_SIGDET_ENABLES, 0x1c),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_PHPRE_CTRL, 0x20),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_DFE_CTLE_POST_CAL_OFFSET, 0x30),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_Q_PI_INTRINSIC_BIAS_RATE32, 0x09),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B0, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B1, 0xb3),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B2, 0x58),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B3, 0x9a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B4, 0x26),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B5, 0xb6),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B6, 0xee),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B0, 0xdb),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B1, 0xdb),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B2, 0xa0),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B3, 0xdf),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B4, 0x78),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B5, 0x76),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B6, 0xff),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_VGA_CAL_CNTRL1, 0x00),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen4x2_pcie_pcs_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_G12S1_TXDEEMPH_M6DB, 0x17),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_G3S2_PRE_GAIN, 0x2e),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_RX_SIGDET_LVL, 0xcc),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_EQ_CONFIG4, 0x00),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_EQ_CONFIG5, 0x22),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_TX_RX_CONFIG1, 0x04),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_TX_RX_CONFIG2, 0x02),
};

static const struct qmp_pcie_init_tbl sm8550_qmp_gen4x2_pcie_pcs_misc_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_ENDPOINT_REFCLK_DRIVE, 0xc1),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_OSC_DTCT_ATCIONS, 0x00),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_EQ_CONFIG1, 0x16),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G3_RXEQEVAL_TIME, 0x27),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G4_RXEQEVAL_TIME, 0x27),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_EQ_CONFIG5, 0x02),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G4_PRE_GAIN, 0x2e),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_RX_MARGINING_CONFIG1, 0x03),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_RX_MARGINING_CONFIG3, 0x28),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_TX_RX_CONFIG, 0xc0),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_POWER_STATE_CONFIG2, 0x1d),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_RX_MARGINING_CONFIG5, 0x0f),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G3_FOM_EQ_CONFIG5, 0xf2),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G4_FOM_EQ_CONFIG5, 0xf2),
};

static const struct qmp_pcie_init_tbl sm8650_qmp_gen4x2_pcie_rx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_FO_GAIN_RATE_2, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_FO_GAIN_RATE_3, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_PI_CONTROLS, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_SO_ACC_DEFAULT_VAL_RATE3, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_IVCM_CAL_CTRL2, 0x82),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_DFE_3, 0x05),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_VGA_CAL_MAN_VAL, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_GM_CAL, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_EQU_ADAPTOR_CNTRL4, 0x0b),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_SIGDET_ENABLES, 0x1c),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_PHPRE_CTRL, 0x20),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_DFE_CTLE_POST_CAL_OFFSET, 0x38),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B0, 0xd3),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B1, 0xd3),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B2, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B3, 0x9a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B4, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B5, 0xb6),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B6, 0xee),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B0, 0x23),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B1, 0x9b),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B2, 0x60),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B3, 0xdf),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B4, 0x43),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B5, 0x76),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B6, 0xff),
};

static const struct qmp_pcie_init_tbl x1e80100_qmp_gen4x2_pcie_serdes_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE1_MODE1, 0x26),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE2_MODE1, 0x03),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CP_CTRL_MODE1, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_RCTRL_MODE1, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CCTRL_MODE1, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CORECLK_DIV_MODE1, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP1_MODE1, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP2_MODE1, 0x1a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DEC_START_MODE1, 0x68),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START1_MODE1, 0xab),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START2_MODE1, 0xaa),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START3_MODE1, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_HSCLK_SEL_1, 0x12),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE1_MODE0, 0xf8),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_STEP_SIZE2_MODE0, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CP_CTRL_MODE0, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_RCTRL_MODE0, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CCTRL_MODE0, 0x36),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_CORE_CLK_DIV_MODE0, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP1_MODE0, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP2_MODE0, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DEC_START_MODE0, 0x41),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START1_MODE0, 0xab),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START2_MODE0, 0xaa),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_DIV_FRAC_START3_MODE0, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_HSCLK_HS_SWITCH_SEL_1, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_BG_TIMER, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_EN_CENTER, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_PER1, 0x62),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SSC_PER2, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_POST_DIV_MUX, 0x40),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_BIAS_EN_CLK_BUFLR_EN, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CLK_ENABLE1, 0x90),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SYS_CLK_CTRL, 0x82),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_IVCO, 0x0f),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_SYSCLK_EN_SEL, 0x08),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP_EN, 0x46),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_LOCK_CMP_CFG, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_VCO_TUNE_MAP, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CLK_SELECT, 0x34),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CORE_CLK_EN, 0xa0),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CMN_CONFIG_1, 0x06),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CMN_MISC_1, 0x88),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_CMN_MODE, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_COM_PLL_VCO_DC_LEVEL_CTRL, 0x0f),
};

static const struct qmp_pcie_init_tbl x1e80100_qmp_gen4x2_pcie_ln_shrd_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RXCLK_DIV2_CTRL, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_DFE_DAC_ENABLE1, 0x88),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_TX_ADAPT_POST_THRESH1, 0x02),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_TX_ADAPT_POST_THRESH2, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B0, 0xd4),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B1, 0x12),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B2, 0xdb),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B3, 0x9a),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B4, 0x32),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B5, 0xb6),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MODE_RATE_0_1_B6, 0x64),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH1_RATE210, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH1_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH2_RATE210, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH2_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH3_RATE210, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH3_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH4_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH5_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_MARG_COARSE_THRESH6_RATE3, 0x1f),
	QMP_PHY_INIT_CFG(QSERDES_V6_LN_SHRD_RX_SUMMER_CAL_SPD_MODE, 0x5b),
};

static const struct qmp_pcie_init_tbl x1e80100_qmp_gen4x2_pcie_tx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_RES_CODE_LANE_OFFSET_TX, 0x1d),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_RES_CODE_LANE_OFFSET_RX, 0x03),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_LANE_MODE_1, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_LANE_MODE_2, 0x10),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_LANE_MODE_3, 0x51),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_TX_TRAN_DRVR_EMP_EN, 0x34),
};

static const struct qmp_pcie_init_tbl x1e80100_qmp_gen4x2_pcie_rx_tbl[] = {
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_FO_GAIN_RATE_2, 0x0c),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_SO_GAIN_RATE_2, 0x04),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_FO_GAIN_RATE_3, 0x0a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_PI_CONTROLS, 0x16),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_UCDR_SO_ACC_DEFAULT_VAL_RATE3, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_IVCM_CAL_CTRL2, 0x80),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_IVCM_POSTCAL_OFFSET, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_BKUP_CTRL1, 0x15),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_DFE_1, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_DFE_2, 0x01),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_DFE_3, 0x45),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V6_20_RX_VGA_CAL_MAN_VAL, 0x0a, 1),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V6_20_RX_VGA_CAL_MAN_VAL, 0x0b, 2),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_VGA_CAL_CNTRL1, 0x00),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_GM_CAL, 0x0d),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_EQU_ADAPTOR_CNTRL4, 0x0b),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_SIGDET_ENABLES, 0x1c),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_PHPRE_CTRL, 0x20),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V6_20_RX_DFE_CTLE_POST_CAL_OFFSET, 0x3a, 1),
	QMP_PHY_INIT_CFG_LANE(QSERDES_V6_20_RX_DFE_CTLE_POST_CAL_OFFSET, 0x38, 2),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_Q_PI_INTRINSIC_BIAS_RATE32, 0x39),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B0, 0x14),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B1, 0xb3),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B2, 0x58),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B3, 0x9a),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B4, 0x26),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B5, 0xb6),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE2_B6, 0xee),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B0, 0xe4),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B1, 0xa4),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B2, 0x60),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B3, 0xdf),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B4, 0x4b),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B5, 0x76),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_MODE_RATE3_B6, 0xff),
	QMP_PHY_INIT_CFG(QSERDES_V6_20_RX_TX_ADPT_CTRL, 0x10),
};

static const struct qmp_pcie_init_tbl x1e80100_qmp_gen4x2_pcie_pcs_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_G3S2_PRE_GAIN, 0x2e),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_RX_SIGDET_LVL, 0xcc),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_EQ_CONFIG4, 0x00),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_EQ_CONFIG5, 0x22),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_TX_RX_CONFIG1, 0x04),
	QMP_PHY_INIT_CFG(QPHY_V6_20_PCS_TX_RX_CONFIG2, 0x02),
};

static const struct qmp_pcie_init_tbl x1e80100_qmp_gen4x2_pcie_pcs_misc_tbl[] = {
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_ENDPOINT_REFCLK_DRIVE, 0xc1),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_OSC_DTCT_ATCIONS, 0x00),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_EQ_CONFIG1, 0x16),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_EQ_CONFIG5, 0x02),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G4_PRE_GAIN, 0x2e),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_RX_MARGINING_CONFIG1, 0x03),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_RX_MARGINING_CONFIG3, 0x28),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G3_RXEQEVAL_TIME, 0x27),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G4_RXEQEVAL_TIME, 0x27),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_TX_RX_CONFIG, 0xc0),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_POWER_STATE_CONFIG2, 0x1d),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_RX_MARGINING_CONFIG5, 0x18),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G3_FOM_EQ_CONFIG5, 0x7a),
	QMP_PHY_INIT_CFG(QPHY_PCIE_V6_20_PCS_G4_FOM_EQ_CONFIG5, 0x8a),
};

struct qmp_pcie_offsets {
	u16 serdes;
	u16 pcs;
	u16 pcs_misc;
	u16 tx;
	u16 rx;
	u16 tx2;
	u16 rx2;
	u16 ln_shrd;
};

struct qmp_pcie_cfg_tbls {
	/* Init sequence for PHY blocks - serdes, tx, rx, pcs */
	const struct qmp_pcie_init_tbl *serdes;
	int serdes_num;
	const struct qmp_pcie_init_tbl *tx;
	int tx_num;
	const struct qmp_pcie_init_tbl *rx;
	int rx_num;
	const struct qmp_pcie_init_tbl *pcs;
	int pcs_num;
	const struct qmp_pcie_init_tbl *pcs_misc;
	int pcs_misc_num;
	const struct qmp_pcie_init_tbl *ln_shrd;
	int ln_shrd_num;
};

/* struct qmp_pcie_cfg - per-PHY initialization config */
struct qmp_pcie_cfg {
	int lanes;

	const struct qmp_pcie_offsets *offsets;

	/* Main init sequence for PHY blocks - serdes, tx, rx, pcs */
	const struct qmp_pcie_cfg_tbls tbls;

	/* regulators to be requested */
	const char * const *vreg_list;
	int num_vregs;
	/* resets to be requested */
	const char * const *reset_list;
	int num_resets;

	/* array of registers with different offsets */
	const unsigned int *regs;

	unsigned int pwrdn_ctrl;
	/* bit offset of PHYSTATUS in QPHY_PCS_STATUS register */
	unsigned int phy_status;

	bool has_nocsr_reset;
};

struct qmp_pcie_priv {
	struct phy *phy;

	void __iomem *serdes;
	void __iomem *pcs;
	void __iomem *pcs_misc;
	void __iomem *tx;
	void __iomem *rx;
	void __iomem *tx2;
	void __iomem *rx2;
	void __iomem *ln_shrd;

	struct clk *clks;
	unsigned int clk_count;

	struct clk pipe_clk;

	struct reset_ctl *resets;
	unsigned int reset_count;

	struct reset_ctl nocsr_reset;

	struct udevice *vregs[NUM_SUPPLIES];
	unsigned int vreg_count;

	const struct qmp_pcie_cfg *cfg;
	struct udevice *dev;
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

/* list of clocks required by phy */
static const char * const qmp_pciephy_clk_l[] = {
	"aux", "cfg_ahb", "ref", "rchng",
};

/* list of regulators */
static const char * const qmp_phy_vreg_l[] = {
	"vdda-phy-supply", "vdda-pll-supply",
};

static const char * const sm8550_qmp_phy_vreg_l[] = {
	"vdda-phy-supply", "vdda-pll-supply", "vdda-qref-supply",
};

/* list of resets */
static const char * const sdm845_pciephy_reset_l[] = {
	"phy",
};

static const struct qmp_pcie_offsets qmp_pcie_offsets_v5 = {
	.serdes		= 0,
	.pcs		= 0x0200,
	.pcs_misc	= 0x0600,
	.tx		= 0x0e00,
	.rx		= 0x1000,
	.tx2		= 0x1600,
	.rx2		= 0x1800,
};

static const struct qmp_pcie_offsets qmp_pcie_offsets_v6_20 = {
	.serdes		= 0x1000,
	.pcs		= 0x1200,
	.pcs_misc	= 0x1400,
	.tx		= 0x0000,
	.rx		= 0x0200,
	.tx2		= 0x0800,
	.rx2		= 0x0a00,
	.ln_shrd	= 0x0e00,
};

static const struct qmp_pcie_cfg sm8550_qmp_gen3x2_pciephy_cfg = {
	.lanes = 2,

	.offsets		= &qmp_pcie_offsets_v5,

	.tbls = {
		.serdes		= sm8550_qmp_gen3x2_pcie_serdes_tbl,
		.serdes_num	= ARRAY_SIZE(sm8550_qmp_gen3x2_pcie_serdes_tbl),
		.tx		= sm8550_qmp_gen3x2_pcie_tx_tbl,
		.tx_num		= ARRAY_SIZE(sm8550_qmp_gen3x2_pcie_tx_tbl),
		.rx		= sm8550_qmp_gen3x2_pcie_rx_tbl,
		.rx_num		= ARRAY_SIZE(sm8550_qmp_gen3x2_pcie_rx_tbl),
		.pcs		= sm8550_qmp_gen3x2_pcie_pcs_tbl,
		.pcs_num	= ARRAY_SIZE(sm8550_qmp_gen3x2_pcie_pcs_tbl),
		.pcs_misc	= sm8550_qmp_gen3x2_pcie_pcs_misc_tbl,
		.pcs_misc_num	= ARRAY_SIZE(sm8550_qmp_gen3x2_pcie_pcs_misc_tbl),
	},
	.reset_list		= sdm845_pciephy_reset_l,
	.num_resets		= ARRAY_SIZE(sdm845_pciephy_reset_l),
	.vreg_list		= qmp_phy_vreg_l,
	.num_vregs		= ARRAY_SIZE(qmp_phy_vreg_l),
	.regs			= pciephy_v5_regs_layout,

	.pwrdn_ctrl		= SW_PWRDN | REFCLK_DRV_DSBL,
	.phy_status		= PHYSTATUS,
};

static const struct qmp_pcie_cfg sm8550_qmp_gen4x2_pciephy_cfg = {
	.lanes = 2,

	.offsets		= &qmp_pcie_offsets_v6_20,

	.tbls = {
		.serdes			= sm8550_qmp_gen4x2_pcie_serdes_tbl,
		.serdes_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_serdes_tbl),
		.tx			= sm8550_qmp_gen4x2_pcie_tx_tbl,
		.tx_num			= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_tx_tbl),
		.rx			= sm8550_qmp_gen4x2_pcie_rx_tbl,
		.rx_num			= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_rx_tbl),
		.pcs			= sm8550_qmp_gen4x2_pcie_pcs_tbl,
		.pcs_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_pcs_tbl),
		.pcs_misc		= sm8550_qmp_gen4x2_pcie_pcs_misc_tbl,
		.pcs_misc_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_pcs_misc_tbl),
		.ln_shrd		= sm8550_qmp_gen4x2_pcie_ln_shrd_tbl,
		.ln_shrd_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_ln_shrd_tbl),
	},
	.reset_list		= sdm845_pciephy_reset_l,
	.num_resets		= ARRAY_SIZE(sdm845_pciephy_reset_l),
	.vreg_list		= sm8550_qmp_phy_vreg_l,
	.num_vregs		= ARRAY_SIZE(sm8550_qmp_phy_vreg_l),
	.regs			= pciephy_v6_regs_layout,

	.pwrdn_ctrl		= SW_PWRDN | REFCLK_DRV_DSBL,
	.phy_status		= PHYSTATUS_4_20,
	.has_nocsr_reset	= true,
};

static const struct qmp_pcie_cfg sm8650_qmp_gen4x2_pciephy_cfg = {
	.lanes = 2,

	.offsets		= &qmp_pcie_offsets_v6_20,

	.tbls = {
		.serdes			= sm8550_qmp_gen4x2_pcie_serdes_tbl,
		.serdes_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_serdes_tbl),
		.tx			= sm8550_qmp_gen4x2_pcie_tx_tbl,
		.tx_num			= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_tx_tbl),
		.rx			= sm8650_qmp_gen4x2_pcie_rx_tbl,
		.rx_num			= ARRAY_SIZE(sm8650_qmp_gen4x2_pcie_rx_tbl),
		.pcs			= sm8550_qmp_gen4x2_pcie_pcs_tbl,
		.pcs_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_pcs_tbl),
		.pcs_misc		= sm8550_qmp_gen4x2_pcie_pcs_misc_tbl,
		.pcs_misc_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_pcs_misc_tbl),
		.ln_shrd		= sm8550_qmp_gen4x2_pcie_ln_shrd_tbl,
		.ln_shrd_num		= ARRAY_SIZE(sm8550_qmp_gen4x2_pcie_ln_shrd_tbl),
	},
	.reset_list		= sdm845_pciephy_reset_l,
	.num_resets		= ARRAY_SIZE(sdm845_pciephy_reset_l),
	.vreg_list		= sm8550_qmp_phy_vreg_l,
	.num_vregs		= ARRAY_SIZE(sm8550_qmp_phy_vreg_l),
	.regs			= pciephy_v6_regs_layout,

	.pwrdn_ctrl		= SW_PWRDN | REFCLK_DRV_DSBL,
	.phy_status		= PHYSTATUS_4_20,
	.has_nocsr_reset	= true,
};

static const struct qmp_pcie_cfg x1e80100_qmp_gen4x2_pciephy_cfg = {
	.lanes = 2,

	.offsets		= &qmp_pcie_offsets_v6_20,

	.tbls = {
		.serdes			= x1e80100_qmp_gen4x2_pcie_serdes_tbl,
		.serdes_num		= ARRAY_SIZE(x1e80100_qmp_gen4x2_pcie_serdes_tbl),
		.tx			= x1e80100_qmp_gen4x2_pcie_tx_tbl,
		.tx_num			= ARRAY_SIZE(x1e80100_qmp_gen4x2_pcie_tx_tbl),
		.rx			= x1e80100_qmp_gen4x2_pcie_rx_tbl,
		.rx_num			= ARRAY_SIZE(x1e80100_qmp_gen4x2_pcie_rx_tbl),
		.pcs			= x1e80100_qmp_gen4x2_pcie_pcs_tbl,
		.pcs_num		= ARRAY_SIZE(x1e80100_qmp_gen4x2_pcie_pcs_tbl),
		.pcs_misc		= x1e80100_qmp_gen4x2_pcie_pcs_misc_tbl,
		.pcs_misc_num		= ARRAY_SIZE(x1e80100_qmp_gen4x2_pcie_pcs_misc_tbl),
		.ln_shrd		= x1e80100_qmp_gen4x2_pcie_ln_shrd_tbl,
		.ln_shrd_num		= ARRAY_SIZE(x1e80100_qmp_gen4x2_pcie_ln_shrd_tbl),
	},
	.reset_list		= sdm845_pciephy_reset_l,
	.num_resets		= ARRAY_SIZE(sdm845_pciephy_reset_l),
	.vreg_list		= qmp_phy_vreg_l,
	.num_vregs		= ARRAY_SIZE(qmp_phy_vreg_l),
	.regs			= pciephy_v6_regs_layout,

	.pwrdn_ctrl		= SW_PWRDN | REFCLK_DRV_DSBL,
	.phy_status		= PHYSTATUS_4_20,
	.has_nocsr_reset	= true,
};

static void qmp_pcie_configure_lane(void __iomem *base,
				    const struct qmp_pcie_init_tbl tbl[],
				    int num, u8 lane_mask)
{
	int i;
	const struct qmp_pcie_init_tbl *t = tbl;

	if (!t)
		return;

	for (i = 0; i < num; i++, t++) {
		if (!(t->lane_mask & lane_mask))
			continue;

		writel(t->val, base + t->offset);
	}
}

static void qmp_pcie_configure(void __iomem *base,
			       const struct qmp_pcie_init_tbl tbl[],
			       int num)
{
	qmp_pcie_configure_lane(base, tbl, num, 0xff);
}

static void qmp_pcie_init_registers(struct qmp_pcie_priv *qmp, const struct qmp_pcie_cfg *cfg)
{
	const struct qmp_pcie_cfg_tbls *tbls = &cfg->tbls;
	void __iomem *serdes = qmp->serdes;
	void __iomem *tx = qmp->tx;
	void __iomem *rx = qmp->rx;
	void __iomem *tx2 = qmp->tx2;
	void __iomem *rx2 = qmp->rx2;
	void __iomem *pcs = qmp->pcs;
	void __iomem *pcs_misc = qmp->pcs_misc;
	void __iomem *ln_shrd = qmp->ln_shrd;

	qmp_pcie_configure(serdes, tbls->serdes, tbls->serdes_num);

	qmp_pcie_configure_lane(tx, tbls->tx, tbls->tx_num, 1);
	qmp_pcie_configure_lane(rx, tbls->rx, tbls->rx_num, 1);

	if (cfg->lanes >= 2) {
		qmp_pcie_configure_lane(tx2, tbls->tx, tbls->tx_num, 2);
		qmp_pcie_configure_lane(rx2, tbls->rx, tbls->rx_num, 2);
	}

	qmp_pcie_configure(pcs, tbls->pcs, tbls->pcs_num);
	qmp_pcie_configure(pcs_misc, tbls->pcs_misc, tbls->pcs_misc_num);

	qmp_pcie_configure(ln_shrd, tbls->ln_shrd, tbls->ln_shrd_num);
}

static int qmp_pcie_do_reset(struct qmp_pcie_priv *qmp)
{
	const struct qmp_pcie_cfg *cfg = qmp->cfg;
	int i, ret;

	for (i = 0; i < qmp->reset_count; i++) {
		ret = reset_assert(&qmp->resets[i]);
		if (ret)
			return ret;
	}

	if (cfg->has_nocsr_reset)
		reset_assert(&qmp->nocsr_reset);

	udelay(10);

	for (i = 0; i < qmp->reset_count; i++) {
		ret = reset_deassert(&qmp->resets[i]);
		if (ret)
			return ret;
	}

	udelay(50);

	return 0;
}

static int qmp_pcie_power_on(struct phy *phy)
{
	struct qmp_pcie_priv *qmp = dev_get_priv(phy->dev);
	const struct qmp_pcie_cfg *cfg = qmp->cfg;
	void __iomem *pcs = qmp->pcs;
	void __iomem *status;
	unsigned int mask, val;
	int ret, i;

	for (i = 0; i < qmp->vreg_count; i++) {
		ret = regulator_set_enable(qmp->vregs[i], true);
		if (ret && ret != -ENOSYS)
			dev_err(phy->dev, "failed to enable regulator %d (%d)\n", i, ret);
	}

	ret = qmp_pcie_do_reset(qmp);
	if (ret)
		return ret;

	for (i = 0; i < qmp->clk_count; i++) {
		ret = clk_enable(&qmp->clks[i]);
		if (ret && ret != -ENOSYS) {
			dev_err(phy->dev, "failed to enable clock %d\n", i);
			return ret;
		}
	}

	/* Power down PHY */
	qphy_setbits(pcs, cfg->regs[QPHY_PCS_POWER_DOWN_CONTROL], cfg->pwrdn_ctrl);

	qmp_pcie_init_registers(qmp, cfg);

	clk_enable(&qmp->pipe_clk);

	if (cfg->has_nocsr_reset)
		reset_deassert(&qmp->nocsr_reset);

	/* Pull PHY out of reset state */
	qphy_clrbits(pcs, cfg->regs[QPHY_SW_RESET], SW_RESET);

	/* start SerDes */
	qphy_setbits(pcs, cfg->regs[QPHY_START_CTRL], SERDES_START | PCS_START);

	status = pcs + cfg->regs[QPHY_PCS_STATUS];
	mask = cfg->phy_status;
	ret = readl_poll_timeout(status, val, !(val & mask), PHY_INIT_COMPLETE_TIMEOUT);
	if (ret) {
		dev_err(phy->dev, "phy initialization timed-out\n");
		return ret;
	}

	return 0;
}

static int qmp_pcie_power_off(struct phy *phy)
{
	struct qmp_pcie_priv *qmp = dev_get_priv(phy->dev);
	const struct qmp_pcie_cfg *cfg = qmp->cfg;

	clk_disable(&qmp->pipe_clk);

	/* PHY reset */
	qphy_setbits(qmp->pcs, cfg->regs[QPHY_SW_RESET], SW_RESET);

	/* stop SerDes and Phy-Coding-Sublayer */
	qphy_clrbits(qmp->pcs, cfg->regs[QPHY_START_CTRL],
		     SERDES_START | PCS_START);

	/* Put PHY into POWER DOWN state: active low */
	qphy_clrbits(qmp->pcs, cfg->regs[QPHY_PCS_POWER_DOWN_CONTROL],
		     cfg->pwrdn_ctrl);

	return 0;
}

static int qmp_pcie_vreg_init(struct udevice *dev, struct qmp_pcie_priv *qmp)
{
	const struct qmp_pcie_cfg *cfg = qmp->cfg;
	unsigned int vreg;
	int ret;

	qmp->vreg_count = cfg->num_vregs;

	for (vreg = 0; vreg < NUM_SUPPLIES && vreg < qmp->vreg_count; ++vreg) {
		ret = device_get_supply_regulator(dev, cfg->vreg_list[vreg], &qmp->vregs[vreg]);
		if (ret)
			dev_warn(dev, "failed to get regulator %d (%d)\n", vreg, ret);

		regulator_set_enable(qmp->vregs[vreg], true);
	}

	return 0;
}

static int qmp_pcie_reset_init(struct udevice *dev, struct qmp_pcie_priv *qmp)
{
	const struct qmp_pcie_cfg *cfg = qmp->cfg;
	int num = cfg->num_resets;
	int i, ret;

	qmp->reset_count = 0;
	qmp->resets = devm_kcalloc(dev, num, sizeof(*qmp->resets), GFP_KERNEL);
	if (!qmp->resets)
		return -ENOMEM;

	for (i = 0; i < num; i++) {
		ret = reset_get_by_name(dev, cfg->reset_list[i], &qmp->resets[i]);
		if (ret) {
			dev_err(dev, "failed to get reset %d\n", i);
			goto reset_get_err;
		}

		++qmp->reset_count;
	}

	if (cfg->has_nocsr_reset) {
		ret = reset_get_by_name(dev, "phy_nocsr", &qmp->nocsr_reset);
		if (ret)
			dev_warn(dev, "failed to get nocsr reset\n");
	}

	return 0;

reset_get_err:
	reset_release_all(qmp->resets, qmp->reset_count);

	return ret;
}

static int qmp_pcie_clk_init(struct udevice *dev, struct qmp_pcie_priv *qmp)
{
	int num = ARRAY_SIZE(qmp_pciephy_clk_l);
	int i, ret;

	qmp->clk_count = 0;
	qmp->clks = devm_kcalloc(dev, num, sizeof(*qmp->clks), GFP_KERNEL);
	if (!qmp->clks)
		return -ENOMEM;

	for (i = 0; i < num; i++) {
		ret = clk_get_by_name(dev, qmp_pciephy_clk_l[i], &qmp->clks[i]);
		/* Ignore failure to get ref clock */
		if (ret && strcmp(qmp_pciephy_clk_l[i], "ref") != 0) {
			dev_err(dev, "failed to get clock %d\n", i);
			goto clk_get_err;
		}

		++qmp->clk_count;
	}

	ret = clk_get_by_name(dev, "pipe", &qmp->pipe_clk);
	if (ret)
		dev_warn(dev, "failed to get pipe clock\n");

	return 0;

clk_get_err:
	clk_release_all(qmp->clks, qmp->clk_count);

	return ret;
}

static int qmp_pcie_parse_dt(struct udevice *dev, struct qmp_pcie_priv *qmp)
{
	const struct qmp_pcie_offsets *offs = qmp->cfg->offsets;
	const struct qmp_pcie_cfg *cfg = qmp->cfg;
	struct resource res;
	int ret;

	if (!qmp->cfg->offsets) {
		dev_err(dev, "missing PCIE offsets\n");
		return -EINVAL;
	}

	ret = ofnode_read_resource(dev_ofnode(dev), 0, &res);
	if (ret) {
		dev_err(dev, "can't get reg property\n");
		return ret;
	}

	qmp->serdes = (void __iomem *)res.start + offs->serdes;
	qmp->pcs = (void __iomem *)res.start + offs->pcs;
	qmp->pcs_misc = (void __iomem *)res.start + offs->pcs_misc;
	qmp->tx = (void __iomem *)res.start + offs->tx;
	qmp->rx = (void __iomem *)res.start + offs->rx;

	if (qmp->cfg->lanes >= 2) {
		qmp->tx2 = (void __iomem *)res.start + offs->tx2;
		qmp->rx2 = (void __iomem *)res.start + offs->rx2;
	}

	if (cfg->tbls.ln_shrd)
		qmp->ln_shrd = (void __iomem *)res.start + offs->ln_shrd;

	return 0;
}

static int qmp_pcie_probe(struct udevice *dev)
{
	struct qmp_pcie_priv *qmp = dev_get_priv(dev);
	int ret;

	qmp->serdes = (void __iomem *)dev_read_addr(dev);
	if (IS_ERR(qmp->serdes))
		return PTR_ERR(qmp->serdes);

	qmp->cfg = (const struct qmp_pcie_cfg *)dev_get_driver_data(dev);
	if (!qmp->cfg)
		return -EINVAL;

	ret = qmp_pcie_clk_init(dev, qmp);
	if (ret) {
		dev_err(dev, "failed to get PCIE clks\n");
		return ret;
	}

	ret = qmp_pcie_vreg_init(dev, qmp);
	if (ret) {
		dev_err(dev, "failed to get PCIE voltage regulators\n");
		return ret;
	}

	ret = qmp_pcie_reset_init(dev, qmp);
	if (ret) {
		dev_err(dev, "failed to get PCIE resets\n");
		return ret;
	}

	qmp->dev = dev;

	return qmp_pcie_parse_dt(dev, qmp);
}

static struct phy_ops qmp_pcie_ops = {
	.power_on = qmp_pcie_power_on,
	.power_off = qmp_pcie_power_off,
};

static const struct udevice_id qmp_pcie_ids[] = {
	{
		.compatible = "qcom,sm8550-qmp-gen3x2-pcie-phy",
		.data = (ulong)&sm8550_qmp_gen3x2_pciephy_cfg,
	}, {
		.compatible = "qcom,sm8550-qmp-gen4x2-pcie-phy",
		.data = (ulong)&sm8550_qmp_gen4x2_pciephy_cfg
	}, {
		.compatible = "qcom,sm8650-qmp-gen3x2-pcie-phy",
		.data = (ulong)&sm8550_qmp_gen3x2_pciephy_cfg,
	}, {
		.compatible = "qcom,sm8650-qmp-gen4x2-pcie-phy",
		.data = (ulong)&sm8650_qmp_gen4x2_pciephy_cfg
	}, {
		.compatible = "qcom,x1e80100-qmp-gen3x2-pcie-phy",
		.data = (ulong)&sm8550_qmp_gen3x2_pciephy_cfg
	}, {
		.compatible = "qcom,x1e80100-qmp-gen4x2-pcie-phy",
		.data = (ulong)&x1e80100_qmp_gen4x2_pciephy_cfg
	}, {
		.compatible = "qcom,x1e80100-qmp-gen4x4-pcie-phy",
		.data = (ulong)&x1e80100_qmp_gen4x2_pciephy_cfg
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(qcom_qmp_pcie) = {
	.name		= "qcom-qmp-pcie",
	.id		= UCLASS_PHY,
	.of_match	= qmp_pcie_ids,
	.ops		= &qmp_pcie_ops,
	.probe		= qmp_pcie_probe,
	.priv_auto	= sizeof(struct qmp_pcie_priv),
};
