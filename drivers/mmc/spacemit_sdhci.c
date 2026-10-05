// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Spacemit K1 Mobile Storage Host Controller
 *
 * Copyright (C) 2023-2026 Spacemit Inc.
 * Copyright (C) 2026 RISCstar Ltd.
 */

#define LOG_CATEGORY UCLASS_MMC

#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <sdhci.h>
#include <reset-uclass.h>
#include <mapmem.h>
#include <dm/pinctrl.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <linux/libfdt.h>
#include <linux/delay.h>
#include <power/regulator.h>

/* SDH register definitions */
#define SPACEMIT_SDHC_OP_EXT_REG	0x108
#define  SDHC_OVRRD_CLK_OEN		BIT(11)
#define  SDHC_FORCE_CLK_ON		BIT(12)

#define SPACEMIT_SDHC_LEGACY_CTRL_REG	0x10C
#define  SDHC_GEN_PAD_CLK_ON		BIT(6)

#define SPACEMIT_SDHC_MMC_CTRL_REG	0x114
#define  SDHC_MISC_INT_EN		BIT(1)
#define  SDHC_MISC_INT			BIT(2)
#define  SDHC_ENHANCE_STROBE_EN		BIT(8)
#define  SDHC_MMC_HS400			BIT(9)
#define  SDHC_MMC_HS200			BIT(10)
#define  SDHC_MMC_CARD_MODE		BIT(12)

#define SPACEMIT_SDHC_TX_CFG_REG	0x11C
#define  SDHC_TX_INT_CLK_SEL		BIT(30)
#define  SDHC_TX_MUX_SEL		BIT(31)

#define SPACEMIT_SDHC_PHY_CTRL_REG	0x160
#define  SDHC_PHY_FUNC_EN		BIT(0)
#define  SDHC_PHY_PLL_LOCK		BIT(1)
#define  SDHC_HOST_LEGACY_MODE		BIT(31)

#define SPACEMIT_SDHC_PHY_FUNC_REG	0x164
#define  SDHC_PHY_TEST_EN		BIT(7)
#define  SDHC_HS200_USE_RFIFO		BIT(15)

#define SPACEMIT_SDHC_PHY_DLLCFG	0x168
#define  SDHC_DLL_PREDLY_NUM		GENMASK(3, 2)
#define  SDHC_DLL_FULLDLY_RANGE		GENMASK(5, 4)
#define  SDHC_DLL_VREG_CTRL		GENMASK(7, 6)
#define  SDHC_DLL_ENABLE		BIT(31)

#define SPACEMIT_SDHC_PHY_DLLCFG1	0x16C
#define  SDHC_DLL_REG1_CTRL		GENMASK(7, 0)
#define  SDHC_DLL_REG2_CTRL		GENMASK(15, 8)
#define  SDHC_DLL_REG3_CTRL		GENMASK(23, 16)
#define  SDHC_DLL_REG4_CTRL		GENMASK(31, 24)
#define  SDHC_DLL_REG1_HS400_VAL	0x92

#define SPACEMIT_SDHC_PHY_DLLSTS	0x170
#define  SDHC_DLL_LOCK_STATE		BIT(0)

#define SPACEMIT_SDHC_PHY_PADCFG_REG	0x178
#define  SDHC_PHY_DRIVE_SEL		GENMASK(2, 0)
#define  SDHC_RX_BIAS_CTRL		BIT(5)

#define SPACEMIT_SDHC_RX_CFG_REG	0x118
#define  SDHC_RX_SDCLK_SEL0_MASK	GENMASK(1, 0)
#define  SDHC_RX_SDCLK_SEL1_MASK	GENMASK(3, 2)
#define  SDHC_RX_SDCLK_SEL1		FIELD_PREP(SDHC_RX_SDCLK_SEL1_MASK, 1)

#define SPACEMIT_SDHC_DLINE_CTRL_REG	0x130
#define  SDHC_DLINE_PU			BIT(0)
#define  SDHC_RX_DLINE_CODE_MASK	GENMASK(23, 16)
#define  SDHC_TX_DLINE_CODE_MASK	GENMASK(31, 24)

#define SPACEMIT_SDHC_DLINE_CFG_REG	0x134
#define  SDHC_RX_DLINE_REG_MASK		GENMASK(7, 0)
#define  SDHC_RX_DLINE_GAIN		BIT(8)
#define  SDHC_TX_DLINE_REG_MASK		GENMASK(23, 16)

#define SPACEMIT_RX_DLINE_REG		0
#define SPACEMIT_RX_TUNE_DELAY_MIN	0x0
#define SPACEMIT_RX_TUNE_DELAY_MAX	0xFF

#define SPACEMIT_TX_TUNING_DLINE_REG	0x00
#define SPACEMIT_TX_TUNING_DELAYCODE	127

struct spacemit_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	struct reset_ctl_bulk resets;
	struct clk_bulk clks;
};

struct spacemit_sdhci_priv {
	struct sdhci_host host;
};

/* All helper functions will update clr/set while preserve rest bits */
static inline void spacemit_sdhci_setbits(struct sdhci_host *host, u32 val,
					  int reg)
{
	sdhci_writel(host, sdhci_readl(host, reg) | val, reg);
}

static inline void spacemit_sdhci_clrbits(struct sdhci_host *host, u32 val,
					  int reg)
{
	sdhci_writel(host, sdhci_readl(host, reg) & ~val, reg);
}

static inline void spacemit_sdhci_clrsetbits(struct sdhci_host *host, u32 clr,
					     u32 set, int reg)
{
	u32 val = sdhci_readl(host, reg);

	val = (val & ~clr) | set;
	sdhci_writel(host, val, reg);
}

#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
static void spacemit_sdhci_set_rx_delay(struct sdhci_host *host, u8 delay)
{
	spacemit_sdhci_clrsetbits(host, SDHC_RX_DLINE_CODE_MASK,
				  FIELD_PREP(SDHC_RX_DLINE_CODE_MASK, delay),
				  SPACEMIT_SDHC_DLINE_CTRL_REG);
}

static void spacemit_sdhci_set_tx_delay(struct sdhci_host *host, u8 delay)
{
	spacemit_sdhci_clrsetbits(host, SDHC_TX_DLINE_CODE_MASK,
				  FIELD_PREP(SDHC_TX_DLINE_CODE_MASK, delay),
				  SPACEMIT_SDHC_DLINE_CTRL_REG);
}

static void spacemit_sdhci_set_tx_dline_reg(struct sdhci_host *host,
					    u8 dline_reg)
{
	spacemit_sdhci_clrsetbits(host, SDHC_TX_DLINE_REG_MASK,
				  FIELD_PREP(SDHC_TX_DLINE_REG_MASK, dline_reg),
				  SPACEMIT_SDHC_DLINE_CFG_REG);
}

static void spacemit_sdhci_tx_tuning_prepare(struct sdhci_host *host)
{
	spacemit_sdhci_setbits(host, SDHC_TX_MUX_SEL, SPACEMIT_SDHC_TX_CFG_REG);
	spacemit_sdhci_setbits(host, SDHC_DLINE_PU,
			       SPACEMIT_SDHC_DLINE_CTRL_REG);
	udelay(5);
}

static void spacemit_sdhci_prepare_tuning(struct sdhci_host *host)
{
	spacemit_sdhci_clrsetbits(host, SDHC_RX_DLINE_REG_MASK,
				  FIELD_PREP(SDHC_RX_DLINE_REG_MASK, SPACEMIT_RX_DLINE_REG),
				  SPACEMIT_SDHC_DLINE_CFG_REG);

	spacemit_sdhci_setbits(host, SDHC_DLINE_PU,
			       SPACEMIT_SDHC_DLINE_CTRL_REG);
	udelay(5);

	spacemit_sdhci_clrsetbits(host, SDHC_RX_SDCLK_SEL1_MASK,
				  SDHC_RX_SDCLK_SEL1,
				  SPACEMIT_SDHC_RX_CFG_REG);

	if (host->mmc->selected_mode == MMC_HS_200)
		spacemit_sdhci_setbits(host, SDHC_HS200_USE_RFIFO,
				       SPACEMIT_SDHC_PHY_FUNC_REG);
}
#endif /* MMC_SUPPORTS_TUNING */

/*
 * Reference: PMU_SDH0_CLK_RES_CTRL (0x054), SDH0_CLK_SEL=0x0,
 * SDH0_CLK_DIV=0x1. The default clock source is 204.8 MHz
 * (pll1_d6_409p6Mhz / 2).
 *
 * During start-up, use a 200 kHz frequency.
 */
#define SDHC_MIN_CLOCK (200 * 1000)

static void spacemit_sdhci_phy_init(struct udevice *dev,
				    struct sdhci_host *host)
{
	u32 reg = 0;

	if (dev_read_bool(dev, "no-sd") && dev_read_bool(dev, "no-sdio")) {
		/* MMC card mode */
		reg = sdhci_readl(host, SPACEMIT_SDHC_MMC_CTRL_REG);
		reg |= SDHC_MMC_CARD_MODE;
		sdhci_writel(host, reg, SPACEMIT_SDHC_MMC_CTRL_REG);

		/* Use PHY functional mode */
		reg = sdhci_readl(host, SPACEMIT_SDHC_PHY_CTRL_REG);
		reg |= (SDHC_PHY_FUNC_EN | SDHC_PHY_PLL_LOCK);
		sdhci_writel(host, reg, SPACEMIT_SDHC_PHY_CTRL_REG);

		reg = sdhci_readl(host, SPACEMIT_SDHC_PHY_PADCFG_REG);
		reg |= SDHC_RX_BIAS_CTRL;
		sdhci_writel(host, reg, SPACEMIT_SDHC_PHY_PADCFG_REG);
	} else {
		reg = sdhci_readl(host, SPACEMIT_SDHC_TX_CFG_REG);
		reg |= SDHC_TX_INT_CLK_SEL;
		sdhci_writel(host, reg, SPACEMIT_SDHC_TX_CFG_REG);
	}

	reg = sdhci_readl(host, SPACEMIT_SDHC_MMC_CTRL_REG);
	reg &= ~SDHC_ENHANCE_STROBE_EN;
	sdhci_writel(host, reg, SPACEMIT_SDHC_MMC_CTRL_REG);
}

static int spacemit_sdhci_set_vqmmc_voltage(struct mmc *mmc, int voltage)
{
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	int ret;

	if (!mmc->vqmmc_supply)
		return 0;

	ret = regulator_set_value(mmc->vqmmc_supply, voltage);
	if (ret)
		return ret;
	ret = regulator_set_enable_if_allowed(mmc->vqmmc_supply, true);
	if (ret)
		return ret;
#endif
	return 0;
}

static void spacemit_sdhci_set_voltage(struct sdhci_host *host)
{
	if (IS_ENABLED(CONFIG_MMC_IO_VOLTAGE)) {
		struct mmc *mmc = host->mmc;
		u32 ctrl;

		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

		switch (mmc->signal_voltage) {
		case MMC_SIGNAL_VOLTAGE_330:
		case MMC_SIGNAL_VOLTAGE_180: {
			bool to_180 = mmc->signal_voltage ==
				      MMC_SIGNAL_VOLTAGE_180;
			bool ok;
			int voltage_mv = to_180 ? 1800000 : 3300000;

			if (spacemit_sdhci_set_vqmmc_voltage(mmc, voltage_mv))
				return;
			if (!IS_SD(mmc))
				return;
			if (to_180)
				ctrl |= SDHCI_CTRL_VDD_180;
			else
				ctrl &= ~SDHCI_CTRL_VDD_180;
			sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

			mdelay(5);

			ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
			ok = !!(ctrl & SDHCI_CTRL_VDD_180) == to_180;
			if (ok)
				return;

			log_err("%d.%dV regulator output not stable\n",
				voltage_mv / 1000000,
				(voltage_mv / 100000) % 10);
			break;
		}
		default:
			/* No signal voltage switch required */
			return;
		}
	}
}

static void spacemit_sdhci_set_clk_gate(struct sdhci_host *host, int auto_gate)
{
	u32 reg;

	reg = sdhci_readl(host, SPACEMIT_SDHC_OP_EXT_REG);
	if (auto_gate)
		reg &= ~(SDHC_OVRRD_CLK_OEN | SDHC_FORCE_CLK_ON);
	else
		reg |= (SDHC_OVRRD_CLK_OEN | SDHC_FORCE_CLK_ON);
	sdhci_writel(host, reg, SPACEMIT_SDHC_OP_EXT_REG);
}

static bool spacemit_sdhci_is_voltage_switch_cmd(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 cmd;

	if (!IS_SD(mmc))
		return false;

	cmd = SDHCI_GET_CMD(sdhci_readw(host, SDHCI_COMMAND));
	return cmd == SD_CMD_SWITCH_UHS18V &&
	       mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180;
}

static int spacemit_sdhci_wait_dat0(struct udevice *dev, int state,
				    int timeout_us)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct sdhci_host *host = mmc->priv;
	u32 tmp;
	int ret;

	ret = read_poll_timeout(sdhci_readl, tmp,
				!!(tmp & SDHCI_DATA_0_LVL_MASK) == !!state,
				0, timeout_us, host, SDHCI_PRESENT_STATE);
	if (ret)
		return ret;

	if (spacemit_sdhci_is_voltage_switch_cmd(host))
		spacemit_sdhci_set_clk_gate(host, 1);
	return 0;
}

static int spacemit_sdhci_get_b_max(struct udevice *dev, void *dst,
				    lbaint_t blkcnt)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	if (IS_ENABLED(CONFIG_SPL_BUILD) && mmc && IS_SD(mmc))
		return 1;
	return mmc->cfg->b_max;
}

static void spacemit_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 reg;

	spacemit_sdhci_set_voltage(host);

	if (spacemit_sdhci_is_voltage_switch_cmd(host))
		spacemit_sdhci_set_clk_gate(host, 0);

	/*
	 * Set TX_INT_CLK_SEL to guarantee hold time at default speed,
	 * HS, SDR12/SDR25/SDR50 modes. See SDHC_TX_CFG_REG (0x11c).
	 */
	reg = sdhci_readl(host, SPACEMIT_SDHC_TX_CFG_REG);
	if (mmc->selected_mode == MMC_LEGACY ||
	    mmc->selected_mode == MMC_HS ||
	    mmc->selected_mode == SD_HS ||
	    mmc->selected_mode == UHS_SDR12 ||
	    mmc->selected_mode == UHS_SDR25 ||
	    mmc->selected_mode == UHS_SDR50) {
		reg |= SDHC_TX_INT_CLK_SEL;
	} else {
		reg &= ~SDHC_TX_INT_CLK_SEL;
	}
	sdhci_writel(host, reg, SPACEMIT_SDHC_TX_CFG_REG);

	/* Set pinctrl state based on selected timing mode */
	if (IS_ENABLED(CONFIG_PINCTRL)) {
		switch (mmc->selected_mode) {
		case UHS_SDR12:
		case UHS_SDR25:
		case UHS_SDR50:
		case UHS_SDR104:
			pinctrl_select_state(mmc->dev, "uhs");
			break;
		default:
			pinctrl_select_state(mmc->dev, "default");
			break;
		}
	}

	if (mmc->selected_mode == MMC_HS_200 ||
	    mmc->selected_mode == MMC_HS_400 ||
	    mmc->selected_mode == MMC_HS_400_ES) {
		reg = sdhci_readw(host, SPACEMIT_SDHC_MMC_CTRL_REG);
		if (mmc->selected_mode == MMC_HS_200)
			reg |= SDHC_MMC_HS200;
		else
			reg |= SDHC_MMC_HS400;
		sdhci_writew(host, reg, SPACEMIT_SDHC_MMC_CTRL_REG);
	} else {
		reg = sdhci_readw(host, SPACEMIT_SDHC_MMC_CTRL_REG);
		reg &= ~(SDHC_MMC_HS200 | SDHC_MMC_HS400 | SDHC_ENHANCE_STROBE_EN);
		sdhci_writew(host, reg, SPACEMIT_SDHC_MMC_CTRL_REG);
	}

	sdhci_set_uhs_timing(host);
}

#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
static int spacemit_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct sdhci_host *host = mmc->priv;
	int current_len = 0, current_start = 0;
	int max_pass_len = 0, max_pass_start = 0;
	u8 final_delay;
	int ret = 0;
	int i;

	/*
	 * Tuning is required for SDR50/SDR104, HS200/HS400 cards and
	 * if clock frequency is greater than 100MHz in these modes.
	 */
	if (host->clock < 100 * 1000 * 1000 ||
	    !(mmc->selected_mode == MMC_HS_200 ||
	      mmc->selected_mode == UHS_SDR50 ||
	      mmc->selected_mode == UHS_SDR104))
		return 0;

	if (IS_SD(host->mmc)) {
		spacemit_sdhci_set_tx_dline_reg(host, SPACEMIT_TX_TUNING_DLINE_REG);
		spacemit_sdhci_set_tx_delay(host, SPACEMIT_TX_TUNING_DELAYCODE);
		spacemit_sdhci_tx_tuning_prepare(host);

		log_debug("TX tuning: dline_reg=%d, delaycode=%d\n",
			  SPACEMIT_TX_TUNING_DLINE_REG, SPACEMIT_TX_TUNING_DELAYCODE);
	}

	spacemit_sdhci_prepare_tuning(host);

	for (i = SPACEMIT_RX_TUNE_DELAY_MIN; i <= SPACEMIT_RX_TUNE_DELAY_MAX; i++) {
		spacemit_sdhci_set_rx_delay(host, i);
		ret = mmc_send_tuning(host->mmc, opcode);

		log_debug("RX delay %d: %s\n",
			  i, ret == 0 ? "pass" : "fail");

		if (ret == 0) {
			/* Test passed - extend current window */
			if (current_len == 0)
				current_start = i;
			current_len++;
		} else {
			/* Test failed - check if current window is best so far */
			if (current_len > max_pass_len) {
				max_pass_len = current_len;
				max_pass_start = current_start;
			}
			current_len = 0;
		}
	}

	if (current_len > max_pass_len) {
		max_pass_len = current_len;
		max_pass_start = current_start;
	}

	if (max_pass_len < 50) {
		log_err("Tuning failed: no stable window found\n");
		return -EIO;
	}

	final_delay = max_pass_start + max_pass_len / 2;
	spacemit_sdhci_set_rx_delay(host, final_delay);
	ret = mmc_send_tuning(host->mmc, opcode);
	if (ret) {
		u8 retry_delays[] = {
			max_pass_start + max_pass_len / 4,
			max_pass_start + (3 * max_pass_len) / 4,
			max_pass_start,
			max_pass_start + max_pass_len - 1
		};
		int retry_count = ARRAY_SIZE(retry_delays);

		log_warning("Primary delay %d failed, trying alternatives\n",
			    final_delay);

		for (i = 0; i < retry_count; i++) {
			if (retry_delays[i] >= SPACEMIT_RX_TUNE_DELAY_MIN &&
			    retry_delays[i] <= SPACEMIT_RX_TUNE_DELAY_MAX) {
				spacemit_sdhci_set_rx_delay(host, retry_delays[i]);
				ret = mmc_send_tuning(host->mmc, opcode);
				if (!ret) {
					final_delay = retry_delays[i];
					log_info("Retry successful with delay %d\n",
						 final_delay);
					break;
				}
			}
		}

		if (ret) {
			log_err("All retry attempts failed\n");
			return -EIO;
		}
	}

	log_debug("Tuning successful: window %d-%d, using delay %d\n",
		  max_pass_start, max_pass_start + max_pass_len - 1, final_delay);

	return 0;
}
#endif /* MMC_SUPPORTS_TUNING */

#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
static int spacemit_sdhci_phy_dll_init(struct sdhci_host *host)
{
	u32 reg, val;
	int ret;

	/* Configure DLL predly, fulldly, and vreg */
	spacemit_sdhci_clrsetbits(host, SDHC_DLL_PREDLY_NUM |
				  SDHC_DLL_FULLDLY_RANGE |
				  SDHC_DLL_VREG_CTRL,
				  FIELD_PREP(SDHC_DLL_PREDLY_NUM, 1) |
				  FIELD_PREP(SDHC_DLL_FULLDLY_RANGE, 1) |
				  FIELD_PREP(SDHC_DLL_VREG_CTRL, 1),
				  SPACEMIT_SDHC_PHY_DLLCFG);

	reg = sdhci_readl(host, SPACEMIT_SDHC_PHY_DLLCFG1);
	reg |= FIELD_PREP(SDHC_DLL_REG1_CTRL, SDHC_DLL_REG1_HS400_VAL);
	sdhci_writel(host, reg, SPACEMIT_SDHC_PHY_DLLCFG1);

	/* Enable DLL */
	reg = sdhci_readl(host, SPACEMIT_SDHC_PHY_DLLCFG);
	reg |= SDHC_DLL_ENABLE;
	sdhci_writel(host, reg, SPACEMIT_SDHC_PHY_DLLCFG);

	/* Wait for DLL lock */
	ret = read_poll_timeout(sdhci_readl, val, val & SDHC_DLL_LOCK_STATE,
				10, 1000, host, SPACEMIT_SDHC_PHY_DLLSTS);
	if (ret) {
		log_err("%s: phy dll lock timeout\n", host->name);
		return -ETIMEDOUT;
	}

	return 0;
}

static int spacemit_sdhci_hs400_enhanced_strobe(struct sdhci_host *host)
{
	u32 reg;

	reg = sdhci_readl(host, SPACEMIT_SDHC_MMC_CTRL_REG);
	reg |= SDHC_ENHANCE_STROBE_EN;
	sdhci_writel(host, reg, SPACEMIT_SDHC_MMC_CTRL_REG);

	return spacemit_sdhci_phy_dll_init(host);
}
#endif

const struct sdhci_ops spacemit_sdhci_ops = {
	.set_control_reg		= spacemit_sdhci_set_control_reg,
#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
	.platform_execute_tuning	= spacemit_sdhci_execute_tuning,
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
	.set_enhanced_strobe		= spacemit_sdhci_hs400_enhanced_strobe,
#endif
};

static struct dm_mmc_ops spacemit_mmc_ops;

static int spacemit_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct spacemit_sdhci_priv *priv = dev_get_priv(dev);
	struct spacemit_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = &priv->host;
	struct clk clk;
	int ret = 0;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	spacemit_mmc_ops = sdhci_ops;
	spacemit_mmc_ops.wait_dat0 = spacemit_sdhci_wait_dat0;
	spacemit_mmc_ops.get_b_max = spacemit_sdhci_get_b_max;

	ret = clk_get_bulk(dev, &plat->clks);
	if (ret) {
		log_err("Can't get clk: %d\n", ret);
		return ret;
	}

	ret = clk_enable_bulk(&plat->clks);
	if (ret) {
		log_err("Failed to enable clk: %d\n", ret);
		goto err_clk;
	}

	ret = reset_get_bulk(dev, &plat->resets);
	if (ret) {
		log_err("Can't get reset: %d\n", ret);
		goto err_clk;
	}

	ret = reset_deassert_bulk(&plat->resets);
	if (ret) {
		log_err("Failed to reset: %d\n", ret);
		goto err_reset;
	}

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret) {
		log_err("Can't get io clk: %d\n", ret);
		goto err_reset;
	}

	ret = clk_set_rate(&clk, plat->cfg.f_max);
	if (ret) {
		log_err("Failed to set io clk: %d\n", ret);
		goto err_reset;
	}

	/* Set quirks */
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_32BIT_DMA_ADDR;
	host->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz;
	host->max_clk = plat->cfg.f_max;

	plat->cfg.f_min = SDHC_MIN_CLOCK;
	host->ops = &spacemit_sdhci_ops;

	ret = sdhci_setup_cfg(&plat->cfg, host, plat->cfg.f_max,
			      SDHC_MIN_CLOCK);
	if (ret)
		goto err_reset;

	ret = sdhci_probe(dev);
	if (ret)
		goto err_reset;

	spacemit_sdhci_phy_init(dev, host);
	return 0;

err_reset:
	reset_release_bulk(&plat->resets);
err_clk:
	clk_release_bulk(&plat->clks);
	return ret;
}

static int spacemit_sdhci_of_to_plat(struct udevice *dev)
{
	struct spacemit_sdhci_plat *plat = dev_get_plat(dev);
	struct spacemit_sdhci_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host = &priv->host;
	int ret = 0;

	host->name = dev->name;
	host->ioaddr = (void *)dev_read_addr(dev);

	ret = mmc_of_parse(dev, &plat->cfg);

	return ret;
}

static int spacemit_sdhci_bind(struct udevice *dev)
{
	struct spacemit_sdhci_plat *drv_data;
	struct spacemit_sdhci_plat *plat = dev_get_plat(dev);

	drv_data = (struct spacemit_sdhci_plat *)dev_get_driver_data(dev);
	if (drv_data)
		memcpy(plat, drv_data, sizeof(struct spacemit_sdhci_plat));
	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id spacemit_sdhci_ids[] = {
	{ .compatible = "spacemit,k1-sdhci" },
	{ }
};

U_BOOT_DRIVER(spacemit_sdhci_drv) = {
	.name		= "spacemit_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= spacemit_sdhci_ids,
	.of_to_plat	= spacemit_sdhci_of_to_plat,
	.ops		= &spacemit_mmc_ops,
	.bind		= spacemit_sdhci_bind,
	.probe		= spacemit_sdhci_probe,
	.priv_auto	= sizeof(struct spacemit_sdhci_priv),
	.plat_auto	= sizeof(struct spacemit_sdhci_plat),
};
