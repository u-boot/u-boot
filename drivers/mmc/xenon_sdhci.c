// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Marvell SOC Platform Group Xenon SDHC as a platform device
 *
 * Copyright (C) 2016 Marvell, All Rights Reserved.
 *
 * Author:	Victor Gu <xigu@marvell.com>
 * Date:	2016-8-24
 *
 * Included parts of the Linux driver version which was written by:
 * Hu Ziji <huziji@marvell.com>
 *
 * Ported to from Marvell 2015.01 to mainline U-Boot 2017.01:
 * Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

/* Register Offset of SD Host Controller SOCP self-defined register */
#define SDHC_SYS_CFG_INFO			0x0104
#define SLOT_TYPE_SDIO_SHIFT			24
#define SLOT_TYPE_EMMC_MASK			0xFF
#define SLOT_TYPE_EMMC_SHIFT			16
#define SLOT_TYPE_SD_SDIO_MMC_MASK		0xFF
#define SLOT_TYPE_SD_SDIO_MMC_SHIFT		8
#define NR_SUPPORTED_SLOT_MASK			0x7

#define SDHC_SYS_OP_CTRL			0x0108
#define AUTO_CLKGATE_DISABLE_MASK		BIT(20)
#define SDCLK_IDLEOFF_ENABLE_SHIFT		8
#define SLOT_ENABLE_SHIFT			0

#define SDHC_SYS_EXT_OP_CTRL			0x010C
#define MASK_CMD_CONFLICT_ERROR			BIT(8)

#define SDHC_SLOT_EMMC_CTRL			0x0130
#define ENABLE_DATA_STROBE_SHIFT		24
#define ENABLE_DATA_STROBE			BIT(ENABLE_DATA_STROBE_SHIFT)
#define SET_EMMC_RSTN_SHIFT			16
#define EMMC_VCCQ_MASK				0x3
#define EMMC_VCCQ_1_8V				0x1
#define EMMC_VCCQ_1_2V				0x2
#define	EMMC_VCCQ_3_3V				0x3

#define SDHC_SLOT_RETUNING_REQ_CTRL		0x0144
/* retuning compatible */
#define RETUNING_COMPATIBLE			0x1

/* Xenon specific Mode Select value */
#define XENON_SDHCI_CTRL_HS200			0x5
#define XENON_SDHCI_CTRL_HS400			0x6

#define EMMC_PHY_REG_BASE			0x170
#define EMMC_PHY_TIMING_ADJUST			EMMC_PHY_REG_BASE
#define OUTPUT_QSN_PHASE_SELECT			BIT(17)
#define SAMPL_INV_QSP_PHASE_SELECT		BIT(18)
#define SAMPL_INV_QSP_PHASE_SELECT_SHIFT	18
#define EMMC_PHY_SDIO_MODE			BIT(28)
#define EMMC_PHY_SLOW_MODE			BIT(29)
#define PHY_INITIALIZAION			BIT(31)
#define WAIT_CYCLE_BEFORE_USING_MASK		0xf
#define WAIT_CYCLE_BEFORE_USING_SHIFT		12
#define FC_SYNC_EN_DURATION_MASK		0xf
#define FC_SYNC_EN_DURATION_SHIFT		8
#define FC_SYNC_RST_EN_DURATION_MASK		0xf
#define FC_SYNC_RST_EN_DURATION_SHIFT		4
#define FC_SYNC_RST_DURATION_MASK		0xf
#define FC_SYNC_RST_DURATION_SHIFT		0

#define EMMC_PHY_FUNC_CONTROL			(EMMC_PHY_REG_BASE + 0x4)
#define DQ_ASYNC_MODE				BIT(4)
#define DQ_DDR_MODE_SHIFT			8
#define DQ_DDR_MODE_MASK			0xff
#define CMD_DDR_MODE				BIT(16)

#define EMMC_PHY_PAD_CONTROL			(EMMC_PHY_REG_BASE + 0x8)
#define REC_EN_SHIFT				24
#define REC_EN_MASK				0xf
#define FC_DQ_RECEN				BIT(24)
#define FC_CMD_RECEN				BIT(25)
#define FC_QSP_RECEN				BIT(26)
#define FC_QSN_RECEN				BIT(27)
#define OEN_QSN					BIT(28)
#define AUTO_RECEN_CTRL				BIT(30)
#define FC_ALL_CMOS_RECEIVER			(REC_EN_MASK << REC_EN_SHIFT)

#define EMMC_PHY_PAD_CONTROL1			(EMMC_PHY_REG_BASE + 0xc)
#define EMMC5_1_FC_QSP_PD			BIT(9)
#define EMMC5_1_FC_QSP_PU			BIT(25)
#define EMMC5_1_FC_CMD_PD			BIT(8)
#define EMMC5_1_FC_CMD_PU			BIT(24)
#define EMMC5_1_FC_DQ_PD			0xff
#define EMMC5_1_FC_DQ_PU			(0xff << 16)

#define EMMC_PHY_PAD_CONTROL2			(EMMC_PHY_REG_BASE + 0x10)
#define ZNR_MASK				0x1F
#define ZNR_SHIFT				8
#define ZPR_MASK				0x1F

#define SDHCI_RETUNE_EVT_INTSIG			0x00001000

#define SDHCI_HOST_CONTROL2		0x3E
#define  SDHCI_CTRL_UHS_MASK		0x0007
#define   SDHCI_CTRL_UHS_SDR12		0x0000
#define   SDHCI_CTRL_UHS_SDR25		0x0001
#define   SDHCI_CTRL_UHS_SDR50		0x0002
#define   SDHCI_CTRL_UHS_SDR104		0x0003
#define   SDHCI_CTRL_UHS_DDR50		0x0004
#define   SDHCI_CTRL_HS400		0x0005 /* Non-standard */
#define   SDHCI_CTRL_HS200_ONLY		0x0005 /* Non-standard */
#define   SDHCI_CTRL_HS400_ONLY		0x0006 /* Non-standard */
#define  SDHCI_CTRL_VDD_180		0x0008
#define  SDHCI_CTRL_DRV_TYPE_MASK	0x0030
#define   SDHCI_CTRL_DRV_TYPE_B		0x0000
#define   SDHCI_CTRL_DRV_TYPE_A		0x0010
#define   SDHCI_CTRL_DRV_TYPE_C		0x0020
#define   SDHCI_CTRL_DRV_TYPE_D		0x0030
#define  SDHCI_CTRL_EXEC_TUNING		0x0040
#define  SDHCI_CTRL_TUNED_CLK		0x0080
#define  SDHCI_CTRL_PRESET_VAL_ENABLE	0x8000

/*
 * Config to eMMC PHY to prepare for tuning.
 * Enable HW DLL and set the TUNING_STEP
 */
#define XENON_SLOT_DLL_CUR_DLY_VAL		0x0150

#define XENON_SLOT_OP_STATUS_CTRL		0x0128
#define XENON_TUN_CONSECUTIVE_TIMES_SHIFT	16
#define XENON_TUN_CONSECUTIVE_TIMES_MASK	0x7
#define XENON_TUN_CONSECUTIVE_TIMES		0x4
#define XENON_TUNING_STEP_SHIFT			12
#define XENON_TUNING_STEP_MASK			0xF
#define XENON_TUNING_STEP_DIVIDER		BIT(6)

#define XENON_EMMC_PHY_DLL_CONTROL		(EMMC_PHY_REG_BASE + 0x14)
#define XENON_EMMC_5_0_PHY_DLL_CONTROL		\
	(XENON_EMMC_5_0_PHY_REG_BASE + 0x10)
#define XENON_DLL_ENABLE			BIT(31)
#define XENON_DLL_UPDATE_STROBE_5_0		BIT(30)
#define XENON_DLL_REFCLK_SEL			BIT(30)
#define XENON_DLL_UPDATE			BIT(23)
#define XENON_DLL_PHSEL1_SHIFT			24
#define XENON_DLL_PHSEL0_SHIFT			16
#define XENON_DLL_PHASE_MASK			0x3F
#define XENON_DLL_PHASE_90_DEGREE		0x1F
#define XENON_DLL_FAST_LOCK			BIT(5)
#define XENON_DLL_GAIN2X			BIT(3)
#define XENON_DLL_BYPASS_EN			BIT(0)

#define XENON_SLOT_EXT_PRESENT_STATE		0x014C
#define XENON_DLL_LOCK_STATE			0x1

/* Hyperion only have one slot 0 */
#define XENON_MMC_SLOT_ID_HYPERION		0
#define SLOT_MASK(slot)				BIT(slot)

#define XENON_EMMC_PHY_LOGIC_TIMING_ADJUST	(EMMC_PHY_REG_BASE + 0x18)
#define XENON_LOGIC_TIMING_VALUE		0x00AA8977

#define MMC_TIMING_LEGACY	0
#define MMC_TIMING_MMC_HS	1
#define MMC_TIMING_SD_HS	2
#define MMC_TIMING_UHS_SDR12	3
#define MMC_TIMING_UHS_SDR25	4
#define MMC_TIMING_UHS_SDR50	5
#define MMC_TIMING_UHS_SDR104	6
#define MMC_TIMING_UHS_DDR50	7
#define MMC_TIMING_MMC_DDR52	8
#define MMC_TIMING_MMC_HS200	9
#define MMC_TIMING_MMC_HS400	10

#define XENON_LOWEST_SDCLK_FREQ		100000
#define XENON_DEFAULT_SDCLK_FREQ	400000

#define XENON_MMC_MAX_CLK	400000000
#define XENON_MMC_3V3_UV	3300000
#define XENON_MMC_1V8_UV	1800000

#define UHS_SDR12_MAX_DTR	25000000
#define MMC_HIGH_26_MAX_DTR	26000000
#define MMC_HIGH_DDR_MAX_DTR	52000000
#define MMC_HS200_MAX_DTR	200000000

enum soc_pad_ctrl_type {
	SOC_PAD_SD,
	SOC_PAD_FIXED_1_8V,
};

struct xenon_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct xenon_sdhci_priv {
	struct sdhci_host host;

	u8 timing;

	unsigned int clock;

	void *pad_ctrl_reg;
	int pad_type;

	struct udevice *vqmmc;
};

static int xenon_mmc_phy_init(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	u32 clock = priv->clock;
	u32 time;
	u32 var;

	/* Enable QSP PHASE SELECT */
	var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
	var |= SAMPL_INV_QSP_PHASE_SELECT;
	if ((priv->timing == MMC_TIMING_UHS_SDR50) ||
	    (priv->timing == MMC_TIMING_UHS_SDR25) ||
	    (priv->timing == MMC_TIMING_UHS_SDR12) ||
	    (priv->timing == MMC_TIMING_SD_HS)     ||
	    (priv->timing == MMC_TIMING_MMC_HS)    ||
	    (priv->timing == MMC_TIMING_LEGACY))
		var |= EMMC_PHY_SLOW_MODE;
	else if (priv->timing == MMC_TIMING_MMC_HS400)
		var &= ~EMMC_PHY_SLOW_MODE;

	sdhci_writel(host, var, EMMC_PHY_TIMING_ADJUST);

	/* Init PHY */
	var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
	var |= PHY_INITIALIZAION;
	sdhci_writel(host, var, EMMC_PHY_TIMING_ADJUST);

	if (clock == 0) {
		/* Use the possibly slowest bus frequency value */
		clock = XENON_LOWEST_SDCLK_FREQ;
	}

	/* Poll for host eMMC PHY init to complete */
	/* Wait up to 10ms */
	time = 100;
	while (time--) {
		var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
		var &= PHY_INITIALIZAION;
		if (!var)
			break;

		/* wait for host eMMC PHY init to complete */
		udelay(100);
	}

	if (time <= 0) {
		pr_err("Failed to init MMC PHY in time\n");
		return -ETIMEDOUT;
	}

	return 0;
}

#define ARMADA_3700_SOC_PAD_1_8V	0x1
#define ARMADA_3700_SOC_PAD_3_3V	0x0

static void armada_3700_soc_pad_voltage_set(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;

	if (priv->pad_type == SOC_PAD_FIXED_1_8V)
		writel(ARMADA_3700_SOC_PAD_1_8V, priv->pad_ctrl_reg);
	else if (priv->pad_type == SOC_PAD_SD)
		writel(ARMADA_3700_SOC_PAD_3_3V, priv->pad_ctrl_reg);
}

static int xenon_mmc_start_signal_voltage_switch(struct sdhci_host *host,
						 int *pwr_1v8)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	u8 voltage = host->mmc->signal_voltage;
	int enable_1v8 = 0;
	u32 ctrl;
	int ret = 0;

	/* If there is no vqmmc regulator, return */
	if (!priv->vqmmc)
		return 0;

	if (priv->pad_type == SOC_PAD_FIXED_1_8V) {
		/* Switch to 1.8v */
		ret = regulator_set_value(priv->vqmmc,
					  XENON_MMC_1V8_UV);
		enable_1v8 = 1;
	} else if (voltage == MMC_SIGNAL_VOLTAGE_330) {
		/* Switch to 3.3v */
		ret = regulator_set_value(priv->vqmmc,
					  XENON_MMC_3V3_UV);
	} else if (voltage == MMC_SIGNAL_VOLTAGE_180) {
		/* Switch to 1.8v */
		ret = regulator_set_value(priv->vqmmc,
					  XENON_MMC_1V8_UV);
		enable_1v8 = 1;
	} else {
		return -ENOTSUPP;
	}

	if (ret) {
		printf("Signal voltage switch fail\n");
		return ret;
	}

	/* Set VCCQ, eMMC mode: 1.8V; SD/SDIO mode: 3.3V */
	ctrl = sdhci_readl(host, SDHC_SLOT_EMMC_CTRL);
	if (IS_SD(host->mmc))
		ctrl |= EMMC_VCCQ_3_3V;
	else
		ctrl |= EMMC_VCCQ_1_8V;
	sdhci_writel(host, ctrl, SDHC_SLOT_EMMC_CTRL);

	*pwr_1v8 = enable_1v8;

	return 0;
}

/*
 * Xenon defines different values for HS200 and HS400
 * in Host_Control_2
 */
static void xenon_set_uhs_signaling(struct sdhci_host *host,
				    unsigned int timing)
{
	u16 ctrl_2;

	ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	/* Select Bus Speed Mode for host */
	ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
	if (timing == MMC_TIMING_MMC_HS200)
		ctrl_2 |= SDHCI_CTRL_HS200_ONLY;
	else if (timing == MMC_TIMING_UHS_SDR104)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
	else if (timing == MMC_TIMING_UHS_SDR12)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
	else if (timing == MMC_TIMING_UHS_SDR25)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
	else if (timing == MMC_TIMING_UHS_SDR50)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
	else if ((timing == MMC_TIMING_UHS_DDR50) ||
		 (timing == MMC_TIMING_MMC_DDR52))
		ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
	else if (timing == MMC_TIMING_MMC_HS400)
		ctrl_2 |= SDHCI_CTRL_HS400_ONLY;
	sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
}

static void xenon_emmc_phy_disable_data_strobe(struct sdhci_host *host)
{
	u32 reg;

	/* Disable SDHC Data Strobe */
	reg = sdhci_readl(host, SDHC_SLOT_EMMC_CTRL);
	reg &= ~ENABLE_DATA_STROBE;
	sdhci_writel(host, reg, SDHC_SLOT_EMMC_CTRL);

	reg = sdhci_readl(host, EMMC_PHY_PAD_CONTROL1);
	reg &= ~(EMMC5_1_FC_QSP_PD | EMMC5_1_FC_QSP_PU);
	sdhci_writel(host, reg, EMMC_PHY_PAD_CONTROL1);
}

/*
 * Enable eMMC PHY HW DLL
 * DLL should be enabled and stable before HS200/SDR104 tuning,
 * and before HS400 data strobe setting.
 */
static int xenon_emmc_phy_enable_dll(struct sdhci_host *host)
{
	u32 reg;
	u32 timeout;

	if (host->mmc->tran_speed <= MMC_HIGH_DDR_MAX_DTR)
		return -EINVAL;

	reg = sdhci_readl(host, XENON_EMMC_PHY_DLL_CONTROL);
	if (reg & XENON_DLL_ENABLE)
		return 0;

	/* Enable DLL */
	reg = sdhci_readl(host, XENON_EMMC_PHY_DLL_CONTROL);
	reg |= (XENON_DLL_ENABLE | XENON_DLL_FAST_LOCK);

	/*
	 * Set Phase as 90 degree, which is most common value.
	 * Might set another value if necessary.
	 * The granularity is 1 degree.
	 */
	reg &= ~((XENON_DLL_PHASE_MASK << XENON_DLL_PHSEL0_SHIFT) |
		 (XENON_DLL_PHASE_MASK << XENON_DLL_PHSEL1_SHIFT));
	reg |= ((XENON_DLL_PHASE_90_DEGREE << XENON_DLL_PHSEL0_SHIFT) |
		(XENON_DLL_PHASE_90_DEGREE << XENON_DLL_PHSEL1_SHIFT));

	reg &= ~(XENON_DLL_BYPASS_EN | XENON_DLL_REFCLK_SEL);
	reg |= XENON_DLL_UPDATE;
	sdhci_writel(host, reg, XENON_EMMC_PHY_DLL_CONTROL);

	/* Wait max 32 ms */
	timeout = 32;
	while (!(sdhci_readl(host, XENON_SLOT_EXT_PRESENT_STATE) &
		XENON_DLL_LOCK_STATE)) {
		if (timeout > 32) {
			printf("Wait for DLL Lock time-out\n");
			return -ETIMEDOUT;
		}
		udelay(1000);
		timeout++;
	}
	return 0;
}

/* Set HS400 Data Strobe */
static void xenon_emmc_phy_strobe_delay_adj(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	u32 reg;

	if (WARN_ON(priv->timing != MMC_TIMING_MMC_HS400))
		return;

	if (host->mmc->tran_speed <= MMC_HIGH_DDR_MAX_DTR)
		return;

	debug("starting HS400 strobe delay adjustment\n");

	xenon_emmc_phy_enable_dll(host);

	/* Enable SDHC Data Strobe */
	reg = sdhci_readl(host, SDHC_SLOT_EMMC_CTRL);
	reg |= ENABLE_DATA_STROBE;
	sdhci_writel(host, reg, SDHC_SLOT_EMMC_CTRL);

	/* Set Data Strobe Pull down */
	reg = sdhci_readl(host, EMMC_PHY_PAD_CONTROL1);
	reg |= EMMC5_1_FC_QSP_PD;
	reg &= ~EMMC5_1_FC_QSP_PU;
	sdhci_writel(host, reg, EMMC_PHY_PAD_CONTROL1);
}

static int xenon_emmc_phy_config_tuning(struct sdhci_host *host)
{
	u32 reg, tuning_step;
	int ret;

	if (host->mmc->tran_speed <= MMC_HIGH_DDR_MAX_DTR)
		return -EINVAL;

	ret = xenon_emmc_phy_enable_dll(host);
	if (ret)
		return ret;

	/* Achieve TUNING_STEP with HW DLL help */
	reg = sdhci_readl(host, XENON_SLOT_DLL_CUR_DLY_VAL);
	tuning_step = reg / XENON_TUNING_STEP_DIVIDER;
	if (unlikely(tuning_step > XENON_TUNING_STEP_MASK)) {
		dev_warn(mmc_dev(host->mmc),
			 "HS200 TUNING_STEP %d is larger than MAX value\n",
			 tuning_step);
		tuning_step = XENON_TUNING_STEP_MASK;
	}

	/* Set TUNING_STEP for later tuning */
	reg = sdhci_readl(host, XENON_SLOT_OP_STATUS_CTRL);
	reg &= ~(XENON_TUN_CONSECUTIVE_TIMES_MASK <<
		 XENON_TUN_CONSECUTIVE_TIMES_SHIFT);
	reg |= (XENON_TUN_CONSECUTIVE_TIMES <<
		XENON_TUN_CONSECUTIVE_TIMES_SHIFT);
	reg &= ~(XENON_TUNING_STEP_MASK << XENON_TUNING_STEP_SHIFT);
	reg |= (tuning_step << XENON_TUNING_STEP_SHIFT);
	sdhci_writel(host, reg, XENON_SLOT_OP_STATUS_CTRL);

	return 0;
}

static void xenon_mmc_phy_set(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	u32 var;

	/* Setup pad, set bit[30], bit[28] and bits[26:24] */
	var = sdhci_readl(host, EMMC_PHY_PAD_CONTROL);
	var |= OEN_QSN | FC_QSP_RECEN | FC_CMD_RECEN | FC_DQ_RECEN |
		FC_ALL_CMOS_RECEIVER;
	sdhci_writel(host, var, EMMC_PHY_PAD_CONTROL);

	/* Set CMD and DQ Pull Up */
	var = sdhci_readl(host, EMMC_PHY_PAD_CONTROL1);
	var |= (EMMC5_1_FC_CMD_PU | EMMC5_1_FC_DQ_PU);
	var &= ~(EMMC5_1_FC_CMD_PD | EMMC5_1_FC_DQ_PD);
	sdhci_writel(host, var, EMMC_PHY_PAD_CONTROL1);

	/*
	 * If Timing belongs to high speed, clear bit[17] of
	 * EMMC_PHY_TIMING_ADJUST register
	 */
	var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
	if ((priv->timing == MMC_TIMING_MMC_HS400) ||
	    (priv->timing == MMC_TIMING_MMC_HS200) ||
	    (priv->timing == MMC_TIMING_MMC_DDR52) ||
	    (priv->timing == MMC_TIMING_UHS_SDR50) ||
	    (priv->timing == MMC_TIMING_UHS_SDR104) ||
	    (priv->timing == MMC_TIMING_UHS_DDR50) ||
	    (priv->timing == MMC_TIMING_UHS_SDR25)) {
		var &= ~OUTPUT_QSN_PHASE_SELECT;
		sdhci_writel(host, var, EMMC_PHY_TIMING_ADJUST);
	}

	if (priv->timing == MMC_TIMING_LEGACY)
		goto phy_init;

	/*
	 * If SDIO card, set SDIO Mode
	 * Otherwise, clear SDIO Mode
	 */
	var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
	if (IS_SD(host->mmc))
		var |= EMMC_PHY_SDIO_MODE;
	else
		var &= ~EMMC_PHY_SDIO_MODE;
	sdhci_writel(host, var, EMMC_PHY_TIMING_ADJUST);

	/*
	 * Set preferred ZNR and ZPR value
	 * The ZNR and ZPR value vary between different boards.
	 * Define them both in sdhci-xenon-emmc-phy.h.
	 */
	var = sdhci_readl(host, EMMC_PHY_PAD_CONTROL2);
	var &= ~((ZNR_MASK << ZNR_SHIFT) | ZPR_MASK);
	var |= ((0xf << ZNR_SHIFT) | 0xf);
	sdhci_writel(host, var, EMMC_PHY_PAD_CONTROL2);

	/*
	 * When setting EMMC_PHY_FUNC_CONTROL register,
	 * SD clock should be disabled
	 */
	var = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	var &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, var, SDHCI_CLOCK_CONTROL);

	var = sdhci_readl(host, EMMC_PHY_FUNC_CONTROL);
	switch (priv->timing) {
	case MMC_TIMING_MMC_HS400:
		var |= (DQ_DDR_MODE_MASK << DQ_DDR_MODE_SHIFT) |
		       CMD_DDR_MODE;
		var &= ~DQ_ASYNC_MODE;
		break;
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		var |= (DQ_DDR_MODE_MASK << DQ_DDR_MODE_SHIFT) |
		       CMD_DDR_MODE | DQ_ASYNC_MODE;
		break;
	default:
		var &= ~((DQ_DDR_MODE_MASK << DQ_DDR_MODE_SHIFT) |
			 CMD_DDR_MODE);
		var |= DQ_ASYNC_MODE;
	}
	sdhci_writel(host, var, EMMC_PHY_FUNC_CONTROL);

	/* Enable bus clock */
	var = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	var |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, var, SDHCI_CLOCK_CONTROL);

	udelay(1000);

	/* Quirk, value suggested by hardware team */
	if (priv->timing == MMC_TIMING_MMC_HS400)
		/* Hardware team recommend a value for HS400 */
		sdhci_writel(host, XENON_LOGIC_TIMING_VALUE,
			     XENON_EMMC_PHY_LOGIC_TIMING_ADJUST);
	else
		xenon_emmc_phy_disable_data_strobe(host);

phy_init:

	xenon_set_uhs_signaling(host, priv->timing);
	xenon_mmc_phy_init(host);

	if (priv->timing == MMC_TIMING_MMC_HS200) {
		if (xenon_emmc_phy_config_tuning(host) != 0)
			printf("Error, failed to tune MMC PHY\n");
	}
	if (priv->timing == MMC_TIMING_MMC_HS400)
		xenon_emmc_phy_strobe_delay_adj(host);
}

/* Set SDCLK-off-while-idle */
static void xenon_set_sdclk_off_idle(struct sdhci_host *host,
				     u8 slot, bool enable)
{
	u32 reg;
	u32 mask;

	reg = sdhci_readl(host, SDHC_SYS_OP_CTRL);
	/* Get the bit shift basing on the SDHC index */
	mask = (0x1 << (SDCLK_IDLEOFF_ENABLE_SHIFT + slot));
	if (enable)
		reg |= mask;
	else
		reg &= ~mask;

	sdhci_writel(host, reg, SDHC_SYS_OP_CTRL);
}

/* Enable/Disable the Auto Clock Gating function of this slot */
static void xenon_mmc_set_acg(struct sdhci_host *host, bool enable)
{
	u32 var;

	var = sdhci_readl(host, SDHC_SYS_OP_CTRL);
	if (enable)
		var &= ~AUTO_CLKGATE_DISABLE_MASK;
	else
		var |= AUTO_CLKGATE_DISABLE_MASK;

	sdhci_writel(host, var, SDHC_SYS_OP_CTRL);
}

/* Enable specific slot */
static void xenon_mmc_enable_slot(struct sdhci_host *host, u8 slot)
{
	u32 var;

	var = sdhci_readl(host, SDHC_SYS_OP_CTRL);
	var |= SLOT_MASK(slot) << SLOT_ENABLE_SHIFT;
	sdhci_writel(host, var, SDHC_SYS_OP_CTRL);
}

/* Enable Parallel Transfer Mode */
static void xenon_mmc_enable_parallel_tran(struct sdhci_host *host, u8 slot)
{
	u32 var;

	var = sdhci_readl(host, SDHC_SYS_EXT_OP_CTRL);
	var |= SLOT_MASK(slot);
	sdhci_writel(host, var, SDHC_SYS_EXT_OP_CTRL);
}

static void xenon_mmc_disable_tuning(struct sdhci_host *host, u8 slot)
{
	u32 var;

	/* Clear the Re-Tuning Request functionality */
	var = sdhci_readl(host, SDHC_SLOT_RETUNING_REQ_CTRL);
	var &= ~RETUNING_COMPATIBLE;
	sdhci_writel(host, var, SDHC_SLOT_RETUNING_REQ_CTRL);

	/* Clear the Re-tuning Event Signal Enable */
	var = sdhci_readl(host, SDHCI_SIGNAL_ENABLE);
	var &= ~SDHCI_RETUNE_EVT_INTSIG;
	sdhci_writel(host, var, SDHCI_SIGNAL_ENABLE);
	var = sdhci_readl(host, SDHCI_INT_ENABLE);
	var &= ~SDHCI_RETUNE_EVT_INTSIG;
	sdhci_writel(host, var, SDHCI_INT_ENABLE);
}

/* Mask command conflict error */
static void xenon_mask_cmd_conflict_err(struct sdhci_host *host)
{
	u32  reg;

	reg = sdhci_readl(host, SDHC_SYS_EXT_OP_CTRL);
	reg |= MASK_CMD_CONFLICT_ERROR;
	sdhci_writel(host, reg, SDHC_SYS_EXT_OP_CTRL);
}

/* Platform specific function for post set_ios configuration */
static int xenon_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	uint speed = host->mmc->tran_speed;
	int pwr_18v = 0;
	u32 reg;

	/*
	 * Signal Voltage Switching is only applicable for Host Controllers
	 * v3.00 and above.
	 */
	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
		xenon_mmc_start_signal_voltage_switch(host, &pwr_18v);

	/* Set timing variable according to the configured speed */
	if (IS_SD(host->mmc)) {
		/* SD/SDIO */
		if (pwr_18v) {
			if (host->mmc->ddr_mode)
				priv->timing = MMC_TIMING_UHS_DDR50;
			else if (speed <= UHS_SDR12_MAX_DTR)
				priv->timing = MMC_TIMING_UHS_SDR25;
			else
				priv->timing = MMC_TIMING_UHS_SDR50;
		} else {
			if (speed <= UHS_SDR12_MAX_DTR)
				priv->timing = MMC_TIMING_LEGACY;
			else
				priv->timing = MMC_TIMING_SD_HS;
		}
	} else {
		/* eMMC */
		if (speed == MMC_HS200_MAX_DTR) {
			if (host->mmc->ddr_mode)
				priv->timing = MMC_TIMING_MMC_HS400;
			else
				priv->timing = MMC_TIMING_MMC_HS200;
		} else if (host->mmc->ddr_mode) {
			priv->timing = MMC_TIMING_MMC_DDR52;
		} else if (speed <= MMC_HIGH_26_MAX_DTR) {
			priv->timing = MMC_TIMING_LEGACY;
		} else {
			priv->timing = MMC_TIMING_MMC_HS;
		}
	}

	/*
	 * HS400/HS200/eMMC HS doesn't have Preset Value register.
	 */
	if ((priv->timing == MMC_TIMING_MMC_HS400) ||
	    (priv->timing == MMC_TIMING_MMC_HS200) ||
	    (priv->timing == MMC_TIMING_MMC_HS)) {
		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg &= ~SDHCI_CTRL_PRESET_VAL_ENABLE;
		sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
	}

	/* Re-init the PHY */
	xenon_mmc_phy_set(host);

	if (host->clock > XENON_DEFAULT_SDCLK_FREQ)
		xenon_set_sdclk_off_idle(host, XENON_MMC_SLOT_ID_HYPERION,
					 true);

	return 0;
}

/* Install a driver specific handler for post set_ios configuration */
static const struct sdhci_ops xenon_sdhci_ops = {
	.set_ios_post = xenon_sdhci_set_ios_post
};

static int xenon_sdhci_probe(struct udevice *dev)
{
	struct xenon_sdhci_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct xenon_sdhci_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;
	int len;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	/* Set quirks */
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_32BIT_DMA_ADDR;

	/* Set default timing */
	priv->timing = MMC_TIMING_LEGACY;

	/* Get the vqmmc regulator if there is */
	device_get_supply_regulator(dev, "vqmmc-supply", &priv->vqmmc);
	/* Set the initial voltage value to 3.3V if there is regulator */
	if (priv->vqmmc) {
		ret = regulator_set_value(priv->vqmmc,
					  XENON_MMC_3V3_UV);
		if (ret) {
			printf("Failed to set VQMMC regulator to 3.3V\n");
			return ret;
		}
	}

	host->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_DDR_52MHz;
	switch (fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "bus-width",
		1)) {
	case 8:
		host->host_caps |= MMC_MODE_8BIT;
		break;
	case 4:
		host->host_caps |= MMC_MODE_4BIT;
		break;
	case 1:
		break;
	default:
		printf("Invalid \"bus-width\" value\n");
		return -EINVAL;
	}

	/* Support for High Speed modes */
	if (fdt_getprop(gd->fdt_blob,
			dev_of_offset(dev), "mmc-hs400-1_8v", &len) != NULL) {
		host->host_caps |= (MMC_MODE_HS400 | MMC_MODE_HS200);
	}
	if (fdt_getprop(gd->fdt_blob,
			dev_of_offset(dev), "mmc-hs200-1_8v", &len) != NULL) {
		host->host_caps |= MMC_MODE_HS200;
	}

	if (host->host_caps & MMC_MODE_HS200)
		sdhci_writeb(host,  SDHCI_POWER_180 | SDHCI_POWER_ON,
			     SDHCI_POWER_CONTROL);

	host->ops = &xenon_sdhci_ops;
	host->max_clk = XENON_MMC_MAX_CLK;

	/* Disable auto clock gating during init */
	xenon_mmc_set_acg(host, false);

	/* Enable slot */
	xenon_mmc_enable_slot(host, XENON_MMC_SLOT_ID_HYPERION);

	/*
	 * Set default power on SoC PHY PAD register (currently only
	 * available on the Armada 3700)
	 */
	if (priv->pad_ctrl_reg)
		armada_3700_soc_pad_voltage_set(host);

	/* Enable parallel transfer */
	xenon_mmc_enable_parallel_tran(host, XENON_MMC_SLOT_ID_HYPERION);

	/* Enable auto clock gating after init */
	xenon_mmc_set_acg(host, true);

	/* Disable SDCLK-Off-While-Idle before card init */
	xenon_set_sdclk_off_idle(host, XENON_MMC_SLOT_ID_HYPERION, false);

	/* Disable tuning functionality of this slot */
	xenon_mmc_disable_tuning(host, XENON_MMC_SLOT_ID_HYPERION);

	xenon_mask_cmd_conflict_err(host);

	ret = sdhci_setup_cfg(&plat->cfg, host, XENON_MMC_MAX_CLK, 0);
	if (ret)
		return ret;

	ret = sdhci_probe(dev);

	return ret;
}

static int xenon_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);
	struct xenon_sdhci_priv *priv = dev_get_priv(dev);
	const char *name;

	host->name = dev->name;
	host->ioaddr = (void *)devfdt_get_addr(dev);

	if (!device_is_compatible(dev, "marvell,armada-3700-sdhci"))
		return 0;

	priv->pad_ctrl_reg = (void *)devfdt_get_addr_index(dev, 1);

	name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "marvell,pad-type",
			   NULL);
	if (name) {
		if (0 == strncmp(name, "sd", 2)) {
			priv->pad_type = SOC_PAD_SD;
		} else if (0 == strncmp(name, "fixed-1-8v", 10)) {
			priv->pad_type = SOC_PAD_FIXED_1_8V;
		} else {
			printf("Unsupported SOC PHY PAD ctrl type %s\n", name);
			return -EINVAL;
		}
	}

	return 0;
}

static int xenon_sdhci_bind(struct udevice *dev)
{
	struct xenon_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id xenon_sdhci_ids[] = {
	{ .compatible = "marvell,armada-8k-sdhci",},
	{ .compatible = "marvell,armada-3700-sdhci",},
	{ }
};

U_BOOT_DRIVER(xenon_sdhci_drv) = {
	.name		= "xenon_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= xenon_sdhci_ids,
	.ofdata_to_platdata = xenon_sdhci_ofdata_to_platdata,
	.ops		= &sdhci_ops,
	.bind		= xenon_sdhci_bind,
	.probe		= xenon_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct xenon_sdhci_priv),
	.platdata_auto_alloc_size = sizeof(struct xenon_sdhci_plat),
};
