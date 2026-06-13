// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Spacemit K1x Mobile Storage Host Controller
 *
 * Copyright (C) 2023 Spacemit Inc.
 * Copyright (C) 2026 RISCstar Ltd.
 */

#define LOG_CATEGORY UCLASS_MMC

#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <linux/delay.h>
#include <log.h>
#include <malloc.h>
#include <sdhci.h>
#include <reset-uclass.h>
#include <power/regulator.h>
#include <mapmem.h>

/* SDH register definitions */
#define SDHC_OP_EXT_REG			0x108
#define OVRRD_CLK_OEN			0x0800
#define FORCE_CLK_ON			0x1000

#define SDHC_LEGACY_CTRL_REG		0x10C
#define GEN_PAD_CLK_ON			0x0040

#define SDHC_MMC_CTRL_REG		0x114
#define MISC_INT_EN			0x0002
#define MISC_INT			0x0004
#define ENHANCE_STROBE_EN		0x0100
#define MMC_HS400			0x0200
#define MMC_HS200			0x0400
#define MMC_CARD_MODE			0x1000

#define SDHC_RX_CFG_REG			0x118
#define RX_SDCLK_SEL0_MASK		0x03
#define RX_SDCLK_SEL0_SHIFT		0x00
#define RX_SDCLK_SEL0			0x02
#define RX_SDCLK_SEL1_MASK		0x03
#define RX_SDCLK_SEL1_SHIFT		0x02
#define RX_SDCLK_SEL1			0x01

#define SDHC_TX_CFG_REG			0x11C
#define TX_INT_CLK_SEL			0x40000000
#define TX_MUX_SEL			0x80000000

#define SDHC_DLINE_CTRL_REG		0x130
#define DLINE_PU			0x01
#define RX_DLINE_CODE_MASK		0xFF
#define RX_DLINE_CODE_SHIFT		0x10
#define TX_DLINE_CODE_MASK		0xFF
#define TX_DLINE_CODE_SHIFT		0x18

#define SDHC_DLINE_CFG_REG		0x134
#define RX_DLINE_REG_MASK		0xFF
#define RX_DLINE_REG_SHIFT		0x00
#define RX_DLINE_GAIN_MASK		0x1
#define RX_DLINE_GAIN_SHIFT		0x8
#define RX_DLINE_GAIN			0x1
#define TX_DLINE_REG_MASK		0xFF
#define TX_DLINE_REG_SHIFT		0x10

#define SDHC_PHY_CTRL_REG		0x160
#define PHY_FUNC_EN			0x0001
#define PHY_PLL_LOCK			0x0002
#define HOST_LEGACY_MODE		0x80000000

#define SDHC_PHY_FUNC_REG		0x164
#define PHY_TEST_EN			0x0080
#define HS200_USE_RFIFO			0x8000

#define SDHC_PHY_DLLCFG			0x168
#define DLL_PREDLY_NUM			0x04
#define DLL_FULLDLY_RANGE		0x10
#define DLL_VREG_CTRL			0x40
#define DLL_ENABLE			0x80000000
#define DLL_REFRESH_SWEN_SHIFT		0x1C
#define DLL_REFRESH_SW_SHIFT		0x1D

#define SDHC_PHY_DLLCFG1		0x16C
#define DLL_REG2_CTRL			0x0C
#define DLL_REG3_CTRL_MASK		0xFF
#define DLL_REG3_CTRL_SHIFT		0x10
#define DLL_REG2_CTRL_MASK		0xFF
#define DLL_REG2_CTRL_SHIFT		0x08
#define DLL_REG1_CTRL			0x92
#define DLL_REG1_CTRL_MASK		0xFF
#define DLL_REG1_CTRL_SHIFT		0x00

#define SDHC_PHY_DLLSTS			0x170
#define DLL_LOCK_STATE			0x01

#define SDHC_PHY_DLLSTS1		0x174
#define DLL_MASTER_DELAY_MASK		0xFF
#define DLL_MASTER_DELAY_SHIFT		0x10

#define SDHC_PHY_PADCFG_REG		0x178
#define RX_BIAS_CTRL_SHIFT		0x5
#define PHY_DRIVE_SEL_SHIFT		0x0
#define PHY_DRIVE_SEL_MASK		0x7
#define PHY_DRIVE_SEL_DEFAULT		0x4

#define MMC1_IO_V18EN			0x04
#define AKEY_ASFAR			0xBABA
#define AKEY_ASSAR			0xEB10

#define SDHC_RX_TUNE_DELAY_MIN		0x0
#define SDHC_RX_TUNE_DELAY_MAX		0xFF
#define SDHC_RX_TUNE_DELAY_STEP		0x1

#define CANDIDATE_WIN_NUM		3
#define SELECT_DELAY_NUM		9
#define WINDOW_1ST			0
#define WINDOW_2ND			1
#define WINDOW_3RD			2

#define RX_TUNING_WINDOW_THRESHOLD	80
#define RX_TUNING_DLINE_REG		0x09
#define TX_TUNING_DLINE_REG		0x00
#define TX_TUNING_DELAYCODE		127

enum window_type {
	LEFT_WINDOW = 0,
	MIDDLE_WINDOW = 1,
	RIGHT_WINDOW = 2,
};

struct tuning_window {
	u8 type;
	u8 min_delay;
	u8 max_delay;
};

struct rx_tuning {
	u8 rx_dline_reg;
	u8 select_delay_num;
	/* 0: biggest window, 1: bigger, 2: small */
	struct tuning_window windows[CANDIDATE_WIN_NUM];
	u8 select_delay[SELECT_DELAY_NUM];

	u8 window_limit;
};

struct spacemit_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	struct reset_ctl_bulk resets;
	struct clk_bulk clks;

	u32 aib_mmc1_io_reg;
	u32 apbc_asfar_reg;
	u32 apbc_assar_reg;

	u8 tx_dline_reg;
	u8 tx_delaycode;
	struct rx_tuning rxtuning;
};

struct spacemit_sdhci_priv {
	struct sdhci_host host;
};

static const u32 tuning_pattern4[16] = {
	0x00ff0fff, 0xccc3ccff, 0xffcc3cc3, 0xeffefffe,
	0xddffdfff, 0xfbfffbff, 0xff7fffbf, 0xefbdf777,
	0xf0fff0ff, 0x3cccfc0f, 0xcfcc33cc, 0xeeffefff,
	0xfdfffdff, 0xffbfffdf, 0xfff7ffbb, 0xde7b7ff7,
};

static const u32 tuning_pattern8[32] = {
	0xff00ffff, 0x0000ffff, 0xccccffff, 0xcccc33cc,
	0xcc3333cc, 0xffffcccc, 0xffffeeff, 0xffeeeeff,
	0xffddffff, 0xddddffff, 0xbbffffff, 0xbbffffff,
	0xffffffbb, 0xffffff77, 0x77ff7777, 0xffeeddbb,
	0x00ffffff, 0x00ffffff, 0xccffff00, 0xcc33cccc,
	0x3333cccc, 0xffcccccc, 0xffeeffff, 0xeeeeffff,
	0xddffffff, 0xddffffff, 0xffffffdd, 0xffffffbb,
	0xffffbbbb, 0xffff77ff, 0xff7777ff, 0xeeddbb77,
};

/*
 * Reference: PMU_SDH0_CLK_RES_CTRL (0x054), SDH0_CLK_SEL=0x0,
 * SDH0_CLK_DIV=0x1. The default clock source is 204.8 MHz
 * (pll1_d6_409p6Mhz / 2).
 *
 * During start-up, use a 200 kHz frequency.
 */
#define SDHC_DEFAULT_MAX_CLOCK (204800000)
#define SDHC_MIN_CLOCK (200 * 1000)

static void spacemit_sdhci_phy_init(struct udevice *dev,
				    struct sdhci_host *host)
{
	u32 reg = 0;

	if (dev_read_bool(dev, "no-sd") && dev_read_bool(dev, "no-sdio")) {
		/* MMC card mode */
		reg = sdhci_readl(host, SDHC_MMC_CTRL_REG);
		reg |= MMC_CARD_MODE;
		sdhci_writel(host, reg, SDHC_MMC_CTRL_REG);

		/* Use PHY functional mode */
		reg = sdhci_readl(host, SDHC_PHY_CTRL_REG);
		reg |= (PHY_FUNC_EN | PHY_PLL_LOCK);
		sdhci_writel(host, reg, SDHC_PHY_CTRL_REG);

		reg = sdhci_readl(host, SDHC_PHY_PADCFG_REG);
		reg |= (1 << RX_BIAS_CTRL_SHIFT);
		sdhci_writel(host, reg, SDHC_PHY_PADCFG_REG);
	} else {
		reg = sdhci_readl(host, SDHC_TX_CFG_REG);
		reg |= TX_INT_CLK_SEL;
		sdhci_writel(host, reg, SDHC_TX_CFG_REG);
	}

	reg = sdhci_readl(host, SDHC_MMC_CTRL_REG);
	reg &= ~ENHANCE_STROBE_EN;
	sdhci_writel(host, reg, SDHC_MMC_CTRL_REG);
}

static int spacemit_sdhci_set_vqmmc_voltage(struct mmc *mmc, int voltage)
{
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	int ret;

	if (!mmc->vqmmc_supply)
		return 0;

	ret = regulator_set_value(mmc->vqmmc_supply, voltage);
	if (ret) {
		log_err("failed to set vqmmc voltage to %d.%dV\n",
			voltage / 1000000, (voltage / 100000) % 10);
		return ret;
	}
	ret = regulator_set_enable_if_allowed(mmc->vqmmc_supply, true);
	if (ret) {
		log_err("failed to enable vqmmc supply\n");
		return ret;
	}
#endif
	return 0;
}

static void spacemit_sdhci_set_voltage(struct sdhci_host *host)
{
	if (IS_ENABLED(CONFIG_MMC_IO_VOLTAGE)) {
		struct mmc *mmc = (struct mmc *)host->mmc;
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

static void spacemit_sdhci_set_aib_mmc1_io(struct sdhci_host *host, int voltage)
{
	struct mmc *mmc = host->mmc;
	struct spacemit_sdhci_plat *plat = dev_get_plat(mmc->dev);
	void __iomem *aib_mmc1_io, *apbc_asfar, *apbc_assar;
	u32 reg;

	if (!plat->aib_mmc1_io_reg || !plat->apbc_asfar_reg ||
	    !plat->apbc_assar_reg)
		return;

	aib_mmc1_io = map_sysmem((uintptr_t)plat->aib_mmc1_io_reg,
				 sizeof(u32));
	apbc_asfar = map_sysmem((uintptr_t)plat->apbc_asfar_reg,
				sizeof(u32));
	apbc_assar = map_sysmem((uintptr_t)plat->apbc_assar_reg,
				sizeof(u32));

	writel(AKEY_ASFAR, apbc_asfar);
	writel(AKEY_ASSAR, apbc_assar);
	reg = readl(aib_mmc1_io);

	switch (voltage) {
	case MMC_SIGNAL_VOLTAGE_180:
		reg |= MMC1_IO_V18EN;
		break;
	default:
		reg &= ~MMC1_IO_V18EN;
		break;
	}
	writel(AKEY_ASFAR, apbc_asfar);
	writel(AKEY_ASSAR, apbc_assar);
	writel(reg, aib_mmc1_io);
}

static void spacemit_sdhci_set_clk_gate(struct sdhci_host *host, int auto_gate)
{
	u32 reg;

	reg = sdhci_readl(host, SDHC_OP_EXT_REG);
	if (auto_gate)
		reg &= ~(OVRRD_CLK_OEN | FORCE_CLK_ON);
	else
		reg |= (OVRRD_CLK_OEN | FORCE_CLK_ON);
	sdhci_writel(host, reg, SDHC_OP_EXT_REG);
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
	unsigned long timeout = timer_get_us() + timeout_us;
	u32 tmp;

	/*
	 * readx_poll_timeout is unsuitable because sdhci_readl accepts
	 * two arguments
	 */
	do {
		tmp = sdhci_readl(host, SDHCI_PRESENT_STATE);
		if (!!(tmp & SDHCI_DATA_0_LVL_MASK) == !!state) {
			if (spacemit_sdhci_is_voltage_switch_cmd(host))
				spacemit_sdhci_set_clk_gate(host, 1);
			return 0;
		}
	} while (!timeout_us || !time_after(timer_get_us(), timeout));

	return -ETIMEDOUT;
}

static void spacemit_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 reg;

	spacemit_sdhci_set_voltage(host);
	spacemit_sdhci_set_aib_mmc1_io(host, mmc->signal_voltage);

	if (spacemit_sdhci_is_voltage_switch_cmd(host))
		spacemit_sdhci_set_clk_gate(host, 0);

	/*
	 * Set TX_INT_CLK_SEL to guarantee hold time at default speed,
	 * HS, SDR12/SDR25/SDR50 modes. See SDHC_TX_CFG_REG (0x11c).
	 */
	reg = sdhci_readl(host, SDHC_TX_CFG_REG);
	if (mmc->selected_mode == MMC_LEGACY ||
	    mmc->selected_mode == MMC_HS ||
	    mmc->selected_mode == SD_HS ||
	    mmc->selected_mode == UHS_SDR12 ||
	    mmc->selected_mode == UHS_SDR25 ||
	    mmc->selected_mode == UHS_SDR50) {
		reg |= TX_INT_CLK_SEL;
	} else {
		reg &= ~TX_INT_CLK_SEL;
	}
	sdhci_writel(host, reg, SDHC_TX_CFG_REG);

	/* Set pinctrl state */
	if (IS_ENABLED(CONFIG_PINCTRL)) {
		if (mmc->clock >= 200000000)
			pinctrl_select_state(mmc->dev, "fast");
		else
			pinctrl_select_state(mmc->dev, "default");
	}

	if (mmc->selected_mode == MMC_HS_200 ||
	    mmc->selected_mode == MMC_HS_400 ||
	    mmc->selected_mode == MMC_HS_400_ES) {
		reg = sdhci_readw(host, SDHC_MMC_CTRL_REG);
		if (mmc->selected_mode == MMC_HS_200)
			reg |= MMC_HS200;
		else
			reg |= MMC_HS400;
		sdhci_writew(host, reg, SDHC_MMC_CTRL_REG);
	} else {
		reg = sdhci_readw(host, SDHC_MMC_CTRL_REG);
		reg &= ~(MMC_HS200 | MMC_HS400 | ENHANCE_STROBE_EN);
		sdhci_writew(host, reg, SDHC_MMC_CTRL_REG);
	}

	sdhci_set_uhs_timing(host);
}

static void spacemit_sdhci_rx_tuning_prepare(struct sdhci_host *host,
					     u8 dline_reg)
{
	struct mmc *mmc = host->mmc;
	u32 reg;

	reg = sdhci_readl(host, SDHC_DLINE_CFG_REG);
	reg &= ~(RX_DLINE_REG_MASK << RX_DLINE_REG_SHIFT);
	reg |= dline_reg << RX_DLINE_REG_SHIFT;

	reg &= ~(RX_DLINE_GAIN_MASK << RX_DLINE_GAIN_SHIFT);
	if (mmc->selected_mode == UHS_SDR50 && (reg & 0x40))
		reg |= RX_DLINE_GAIN << RX_DLINE_GAIN_SHIFT;

	sdhci_writel(host, reg, SDHC_DLINE_CFG_REG);

	reg = sdhci_readl(host, SDHC_DLINE_CTRL_REG);
	reg |= DLINE_PU;
	sdhci_writel(host, reg, SDHC_DLINE_CTRL_REG);
	udelay(5);

	reg = sdhci_readl(host, SDHC_RX_CFG_REG);
	reg &= ~(RX_SDCLK_SEL1_MASK << RX_SDCLK_SEL1_SHIFT);
	reg |= RX_SDCLK_SEL1 << RX_SDCLK_SEL1_SHIFT;
	sdhci_writel(host, reg, SDHC_RX_CFG_REG);

	if (mmc->selected_mode == MMC_HS_200) {
		reg = sdhci_readl(host, SDHC_PHY_FUNC_REG);
		reg |= HS200_USE_RFIFO;
		sdhci_writel(host, reg, SDHC_PHY_FUNC_REG);
	}
}

static void spacemit_sdhci_rx_set_delaycode(struct sdhci_host *host, u32 delay)
{
	u32 reg;

	reg = sdhci_readl(host, SDHC_DLINE_CTRL_REG);
	reg &= ~(RX_DLINE_CODE_MASK << RX_DLINE_CODE_SHIFT);
	reg |= (delay & RX_DLINE_CODE_MASK) << RX_DLINE_CODE_SHIFT;
	sdhci_writel(host, reg, SDHC_DLINE_CTRL_REG);
}

static void spacemit_sdhci_tx_tuning_prepare(struct sdhci_host *host)
{
	u32 reg;

	/* Set TX_MUX_SEL */
	reg = sdhci_readl(host, SDHC_TX_CFG_REG);
	reg |= TX_MUX_SEL;
	sdhci_writel(host, reg, SDHC_TX_CFG_REG);

	reg = sdhci_readl(host, SDHC_DLINE_CTRL_REG);
	reg |= DLINE_PU;
	sdhci_writel(host, reg, SDHC_DLINE_CTRL_REG);
	udelay(5);
}

static void spacemit_sdhci_tx_set_dlinereg(struct sdhci_host *host,
					   u8 dline_reg)
{
	u32 reg;

	reg = sdhci_readl(host, SDHC_DLINE_CFG_REG);
	reg &= ~(TX_DLINE_REG_MASK << TX_DLINE_REG_SHIFT);
	reg |= dline_reg << TX_DLINE_REG_SHIFT;
	sdhci_writel(host, reg, SDHC_DLINE_CFG_REG);
}

static void spacemit_sdhci_tx_set_delaycode(struct sdhci_host *host, u32 delay)
{
	u32 reg;

	reg = sdhci_readl(host, SDHC_DLINE_CTRL_REG);
	reg &= ~(TX_DLINE_CODE_MASK << TX_DLINE_CODE_SHIFT);
	reg |= (delay & TX_DLINE_CODE_MASK) << TX_DLINE_CODE_SHIFT;
	sdhci_writel(host, reg, SDHC_DLINE_CTRL_REG);
}

static void spacemit_sdhci_clear_set_irqs(struct sdhci_host *host,
					  u32 clr, u32 set)
{
	u32 ier;

	ier = sdhci_readl(host, SDHCI_INT_ENABLE);
	ier &= ~clr;
	ier |= set;
	sdhci_writel(host, ier, SDHCI_INT_ENABLE);
	sdhci_writel(host, ier, SDHCI_SIGNAL_ENABLE);
}

static int spacemit_sdhci_tuning_pattern_check(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 read_pattern;
	unsigned int i;
	u32 *tuning_pattern;
	int pattern_len;
	int err = 0;

	if (mmc->bus_width == 8) {
		tuning_pattern = (u32 *)tuning_pattern8;
		pattern_len = ARRAY_SIZE(tuning_pattern8);
	} else {
		tuning_pattern = (u32 *)tuning_pattern4;
		pattern_len = ARRAY_SIZE(tuning_pattern4);
	}

	for (i = 0; i < pattern_len; i++) {
		read_pattern = sdhci_readl(host, SDHCI_BUFFER);
		if (read_pattern != tuning_pattern[i])
			err++;
	}

	return err;
}

static int spacemit_sdhci_send_tuning(struct sdhci_host *host, u32 opcode,
				      int *cmd_error)
{
	struct mmc *mmc = host->mmc;
	struct mmc_cmd cmd;
	int size, blk_size, err;

	size = sizeof(tuning_pattern4);
	cmd.cmdidx = opcode;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R1;

	blk_size = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 64);
	sdhci_writew(host, blk_size, SDHCI_BLOCK_SIZE);
	sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		log_warning("%s: tuning send cmd err: %d\n", host->name, err);
		return err;
	}
	return 0;
}

static int spacemit_sdhci_send_tuning_cmd(struct sdhci_host *host, u32 opcode)
{
	int err = 0;

	err = spacemit_sdhci_send_tuning(host, opcode, NULL);
	if (err) {
		log_warning("%s: send tuning err:%d\n", host->name, err);
		return err;
	}

	err = spacemit_sdhci_tuning_pattern_check(host);
	return err;
}

static void spacemit_sdhci_clear_tuned_clk(struct sdhci_host *host)
{
	u16 ctrl;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl &= ~(SDHCI_CTRL_TUNED_CLK | SDHCI_CTRL_EXEC_TUNING);
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
}

static int spacemit_sdhci_rx_select_window(struct sdhci_host *host, u32 opcode)
{
	int min, max;
	int i, j, len;
	int err = 0;
	u32 ier;
	struct tuning_window tmp;
	struct mmc *mmc = host->mmc;
	struct spacemit_sdhci_plat *plat = dev_get_plat(mmc->dev);
	struct rx_tuning *rxtuning = &plat->rxtuning;

	/* Change to PIO mode during tuning */
	ier = sdhci_readl(host, SDHCI_INT_ENABLE);
	spacemit_sdhci_clear_set_irqs(host, ier, SDHCI_INT_DATA_AVAIL);

	min = SDHC_RX_TUNE_DELAY_MIN;
	do {
		/* Find the minimum delay that can pass tuning */
		while (min < SDHC_RX_TUNE_DELAY_MAX) {
			spacemit_sdhci_rx_set_delaycode(host, min);
			err = spacemit_sdhci_send_tuning_cmd(host, opcode);
			if (!err)
				break;
			spacemit_sdhci_clear_tuned_clk(host);
			min += SDHC_RX_TUNE_DELAY_STEP;
		}

		/* Find the maximum delay that cannot pass tuning */
		max = min + SDHC_RX_TUNE_DELAY_STEP;
		while (max < SDHC_RX_TUNE_DELAY_MAX) {
			spacemit_sdhci_rx_set_delaycode(host, max);
			err = spacemit_sdhci_send_tuning_cmd(host, opcode);
			if (err) {
				spacemit_sdhci_clear_tuned_clk(host);
				break;
			}
			max += SDHC_RX_TUNE_DELAY_STEP;
		}

		log_info("%s: pass window [%d %d)\n", host->name, min, max);
		/* Store the top 3 windows */
		if ((max - min) >= rxtuning->window_limit) {
			tmp.max_delay = max;
			tmp.min_delay = min;
			tmp.type = MIDDLE_WINDOW;
			for (i = 0; i < CANDIDATE_WIN_NUM; i++) {
				len = rxtuning->windows[i].max_delay -
				      rxtuning->windows[i].min_delay;
				if ((tmp.max_delay - tmp.min_delay) > len) {
					for (j = CANDIDATE_WIN_NUM - 1;
					     j > i; j--)
						rxtuning->windows[j] =
						rxtuning->windows[j - 1];
					rxtuning->windows[i] = tmp;
					break;
				}
			}
		}
		min = max + SDHC_RX_TUNE_DELAY_STEP;
	} while (min < SDHC_RX_TUNE_DELAY_MAX);

	spacemit_sdhci_clear_set_irqs(host, SDHCI_INT_DATA_AVAIL, ier);
	return 0;
}

static int spacemit_sdhci_rx_select_delay(struct sdhci_host *host)
{
	int i;
	int win_len, min, max, mid;
	u8 n;
	struct tuning_window *window;
	struct mmc *mmc = host->mmc;
	struct spacemit_sdhci_plat *plat = dev_get_plat(mmc->dev);
	struct rx_tuning *tuning = &plat->rxtuning;

	for (i = 0; i < CANDIDATE_WIN_NUM; i++) {
		window = &tuning->windows[i];
		min = window->min_delay;
		max = window->max_delay;
		mid = (min + max - 1) / 2;
		win_len = max - min;
		if (win_len < tuning->window_limit)
			continue;

		n = tuning->select_delay_num;
		if (window->type == LEFT_WINDOW) {
			tuning->select_delay[n++] = min + win_len / 3;
			tuning->select_delay[n++] = min + win_len / 2;
		} else if (window->type == RIGHT_WINDOW) {
			tuning->select_delay[n++] = max - win_len / 4;
			tuning->select_delay[n++] = min - win_len / 3;
		} else {
			tuning->select_delay[n++] = mid;
			tuning->select_delay[n++] = mid + win_len / 4;
			tuning->select_delay[n++] = mid - win_len / 4;
		}
		tuning->select_delay_num = n;
	}

	return tuning->select_delay_num;
}

static int spacemit_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct sdhci_host *host = mmc->priv;
	struct spacemit_sdhci_plat *plat = dev_get_plat(mmc->dev);
	struct rx_tuning *rxtuning = &plat->rxtuning;
	int ret;

	/*
	 * Tuning is required for SDR50/SDR104 mode
	 */
	if (!IS_SD(host->mmc) ||
	    !(mmc->selected_mode == UHS_SDR50 ||
	       mmc->selected_mode == UHS_SDR104))
		return 0;

	/* TX tuning config */
	if (IS_SD(host->mmc)) {
		spacemit_sdhci_tx_set_dlinereg(host, plat->tx_dline_reg);
		spacemit_sdhci_tx_set_delaycode(host, plat->tx_delaycode);
		log_info("%s: set tx_delaycode: %d\n",
			 host->name, plat->tx_delaycode);
		spacemit_sdhci_tx_tuning_prepare(host);
	}

	rxtuning->select_delay_num = 0;
	memset(rxtuning->windows, 0, sizeof(rxtuning->windows));
	memset(rxtuning->select_delay, 0xFF, sizeof(rxtuning->select_delay));

	spacemit_sdhci_rx_tuning_prepare(host, rxtuning->rx_dline_reg);
	ret = spacemit_sdhci_rx_select_window(host, opcode);
	if (ret) {
		log_warning("%s: abort tuning, err:%d\n", host->name, ret);
		return ret;
	}

	if (!spacemit_sdhci_rx_select_delay(host)) {
		log_warning("%s: fail to get delaycode\n", host->name);
		return -EIO;
	}

	spacemit_sdhci_rx_set_delaycode(host, rxtuning->select_delay[0]);
	log_info("%s: tuning done, use the firstly delay_code:%d\n",
		 host->name, rxtuning->select_delay[0]);
	return 0;
}

#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
static int spacemit_sdhci_phy_dll_init(struct sdhci_host *host)
{
	u32 reg;
	int i;

	/* Configure DLL_REG1 and DLL_REG2 */
	reg = sdhci_readl(host, SDHC_PHY_DLLCFG);
	reg |= (DLL_PREDLY_NUM | DLL_FULLDLY_RANGE | DLL_VREG_CTRL);
	sdhci_writel(host, reg, SDHC_PHY_DLLCFG);

	reg = sdhci_readl(host, SDHC_PHY_DLLCFG1);
	reg |= (DLL_REG1_CTRL & DLL_REG1_CTRL_MASK);
	sdhci_writel(host, reg, SDHC_PHY_DLLCFG1);

	/* Enable DLL */
	reg = sdhci_readl(host, SDHC_PHY_DLLCFG);
	reg |= DLL_ENABLE;
	sdhci_writel(host, reg, SDHC_PHY_DLLCFG);

	/* Wait for DLL lock */
	i = 0;
	while (i++ < 100) {
		if (sdhci_readl(host, SDHC_PHY_DLLSTS) & DLL_LOCK_STATE)
			break;
		udelay(10);
	}
	if (i == 100) {
		log_err("%s: phy dll lock timeout\n", host->name);
		return -ETIMEDOUT;
	}

	return 0;
}

static int spacemit_sdhci_hs400_enhanced_strobe(struct sdhci_host *host)
{
	u32 reg;

	reg = sdhci_readl(host, SDHC_MMC_CTRL_REG);
	reg |= ENHANCE_STROBE_EN;
	sdhci_writel(host, reg, SDHC_MMC_CTRL_REG);

	return spacemit_sdhci_phy_dll_init(host);
}
#endif

const struct sdhci_ops spacemit_sdhci_ops = {
	.set_control_reg		= spacemit_sdhci_set_control_reg,
	.platform_execute_tuning	= spacemit_sdhci_execute_tuning,
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

	ret = clk_get_bulk(dev, &plat->clks);
	if (ret) {
		log_err("Can't get clk: %d\n", ret);
		return ret;
	}

	ret = clk_enable_bulk(&plat->clks);
	if (ret) {
		log_err("Failed to enable clk: %d\n", ret);
		return ret;
	}

	ret = reset_get_bulk(dev, &plat->resets);
	if (ret) {
		log_err("Can't get reset: %d\n", ret);
		return ret;
	}

	ret = reset_deassert_bulk(&plat->resets);
	if (ret) {
		log_err("Failed to reset: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret) {
		log_err("Can't get io clk: %d\n", ret);
		return ret;
	}

	ret = clk_set_rate(&clk, plat->cfg.f_max);
	if (ret) {
		log_err("Failed to set io clk: %d\n", ret);
		return ret;
	}

	/* Set quirks */
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;
	else
		host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
				SDHCI_QUIRK_32BIT_DMA_ADDR;
	host->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz;
	host->max_clk = plat->cfg.f_max;

	plat->cfg.f_min = SDHC_MIN_CLOCK;
	host->ops = &spacemit_sdhci_ops;

	ret = sdhci_setup_cfg(&plat->cfg, host, plat->cfg.f_max,
			      SDHC_MIN_CLOCK);
	if (ret)
		return ret;

	ret = sdhci_probe(dev);
	if (ret)
		return ret;

	spacemit_sdhci_phy_init(dev, host);
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
	memcpy(plat, drv_data, sizeof(struct spacemit_sdhci_plat));
	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct spacemit_sdhci_plat k1_data = {
	.aib_mmc1_io_reg	= 0xd401e81c,
	.apbc_asfar_reg		= 0xd4015050,
	.apbc_assar_reg		= 0xd4015054,
	.tx_dline_reg		= TX_TUNING_DLINE_REG,
	.tx_delaycode		= 0x5f,
	.rxtuning		= {
		.rx_dline_reg	= RX_TUNING_DLINE_REG,
		.window_limit	= 50,
	},
};

static const struct udevice_id spacemit_sdhci_ids[] = {
	{
		.compatible = "spacemit,k1-sdhci",
		.data = (ulong)&k1_data,
	}, {
	}
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
