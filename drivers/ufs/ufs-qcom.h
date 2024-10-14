/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 */

#ifndef UFS_QCOM_H_
#define UFS_QCOM_H_

#include <reset.h>
#include <linux/bitfield.h>

#define MPHY_TX_FSM_STATE       0x41
#define TX_FSM_HIBERN8          0x1
#define DEFAULT_CLK_RATE_HZ     1000000

#define UFS_HW_VER_MAJOR_MASK	GENMASK(31, 28)
#define UFS_HW_VER_MINOR_MASK	GENMASK(27, 16)
#define UFS_HW_VER_STEP_MASK	GENMASK(15, 0)

/* QCOM UFS host controller vendor specific registers */
enum {
	REG_UFS_SYS1CLK_1US                 = 0xC0,
	REG_UFS_TX_SYMBOL_CLK_NS_US         = 0xC4,
	REG_UFS_LOCAL_PORT_ID_REG           = 0xC8,
	REG_UFS_PA_ERR_CODE                 = 0xCC,
	/* On older UFS revisions, this register is called "RETRY_TIMER_REG" */
	REG_UFS_PARAM0                      = 0xD0,
	/* On older UFS revisions, this register is called "REG_UFS_PA_LINK_STARTUP_TIMER" */
	REG_UFS_CFG0                        = 0xD8,
	REG_UFS_CFG1                        = 0xDC,
	REG_UFS_CFG2                        = 0xE0,
	REG_UFS_HW_VERSION                  = 0xE4,

	UFS_TEST_BUS				= 0xE8,
	UFS_TEST_BUS_CTRL_0			= 0xEC,
	UFS_TEST_BUS_CTRL_1			= 0xF0,
	UFS_TEST_BUS_CTRL_2			= 0xF4,
	UFS_UNIPRO_CFG				= 0xF8,

	/*
	 * QCOM UFS host controller vendor specific registers
	 * added in HW Version 3.0.0
	 */
	UFS_AH8_CFG				= 0xFC,

	REG_UFS_CFG3				= 0x271C,
};

/* bit definitions for REG_UFS_CFG0 register */
#define QUNIPRO_G4_SEL		BIT(5)

/* bit definitions for REG_UFS_CFG1 register */
#define QUNIPRO_SEL		BIT(0)
#define UFS_PHY_SOFT_RESET	BIT(1)
#define UTP_DBG_RAMS_EN		BIT(17)
#define TEST_BUS_EN		BIT(18)
#define TEST_BUS_SEL		GENMASK(22, 19)
#define UFS_REG_TEST_BUS_EN	BIT(30)

#define UFS_PHY_RESET_ENABLE	1
#define UFS_PHY_RESET_DISABLE	0

/* bit definitions for REG_UFS_CFG2 register */
#define UAWM_HW_CGC_EN		BIT(0)
#define UARM_HW_CGC_EN		BIT(1)
#define TXUC_HW_CGC_EN		BIT(2)
#define RXUC_HW_CGC_EN		BIT(3)
#define DFC_HW_CGC_EN		BIT(4)
#define TRLUT_HW_CGC_EN		BIT(5)
#define TMRLUT_HW_CGC_EN	BIT(6)
#define OCSC_HW_CGC_EN		BIT(7)

/* bit definitions for REG_UFS_PARAM0 */
#define MAX_HS_GEAR_MASK	GENMASK(6, 4)
#define UFS_QCOM_MAX_GEAR(x)	FIELD_GET(MAX_HS_GEAR_MASK, (x))

/* bit definition for UFS_UFS_TEST_BUS_CTRL_n */
#define TEST_BUS_SUB_SEL_MASK	GENMASK(4, 0)  /* All XXX_SEL fields are 5 bits wide */

#define REG_UFS_CFG2_CGC_EN_ALL (UAWM_HW_CGC_EN | UARM_HW_CGC_EN |\
				 TXUC_HW_CGC_EN | RXUC_HW_CGC_EN |\
				 DFC_HW_CGC_EN | TRLUT_HW_CGC_EN |\
				 TMRLUT_HW_CGC_EN | OCSC_HW_CGC_EN)

/* bit offset */
#define OFFSET_CLK_NS_REG		0xa

/* bit masks */
#define MASK_TX_SYMBOL_CLK_1US_REG	GENMASK(9, 0)
#define MASK_CLK_NS_REG			GENMASK(23, 10)

/* QUniPro Vendor specific attributes */
#define PA_VS_CONFIG_REG1	0x9000
#define DME_VS_CORE_CLK_CTRL	0xD002
/* bit and mask definitions for DME_VS_CORE_CLK_CTRL attribute */
#define CLK_1US_CYCLES_MASK_V4				GENMASK(27, 16)
#define CLK_1US_CYCLES_MASK				GENMASK(7, 0)
#define DME_VS_CORE_CLK_CTRL_CORE_CLK_DIV_EN_BIT	BIT(8)
#define PA_VS_CORE_CLK_40NS_CYCLES			0x9007
#define PA_VS_CORE_CLK_40NS_CYCLES_MASK			GENMASK(6, 0)

/* QCOM UFS host controller core clk frequencies */
#define UNIPRO_CORE_CLK_FREQ_37_5_MHZ          38
#define UNIPRO_CORE_CLK_FREQ_75_MHZ            75
#define UNIPRO_CORE_CLK_FREQ_100_MHZ           100
#define UNIPRO_CORE_CLK_FREQ_150_MHZ           150
#define UNIPRO_CORE_CLK_FREQ_300_MHZ           300
#define UNIPRO_CORE_CLK_FREQ_201_5_MHZ         202
#define UNIPRO_CORE_CLK_FREQ_403_MHZ           403

static inline void
ufs_qcom_get_controller_revision(struct ufs_hba *hba,
				 u8 *major, u16 *minor, u16 *step)
{
	u32 ver = ufshcd_readl(hba, REG_UFS_HW_VERSION);

	*major = FIELD_GET(UFS_HW_VER_MAJOR_MASK, ver);
	*minor = FIELD_GET(UFS_HW_VER_MINOR_MASK, ver);
	*step = FIELD_GET(UFS_HW_VER_STEP_MASK, ver);
};

/* Host controller hardware version: major.minor.step */
struct ufs_hw_version {
	u16 step;
	u16 minor;
	u8 major;
};

struct gpio_desc;

struct ufs_qcom_priv {
	struct phy *generic_phy;
	struct ufs_hba *hba;

	struct clk_bulk clks;
	bool is_clks_enabled;

	struct ufs_hw_version hw_ver;

	/* Reset control of HCI */
	struct reset_ctl core_reset;

	struct gpio_desc reset;

	bool is_dev_ref_clk_enabled;
};

#endif /* UFS_QCOM_H_ */
