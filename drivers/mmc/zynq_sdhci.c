// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 - 2015 Xilinx, Inc.
 *
 * Xilinx Zynq SD Host Controller Interface
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/delay.h>
#include "mmc_private.h"
#include <log.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <zynqmp_tap_delay.h>

#define SDHCI_ARASAN_ITAPDLY_REGISTER   0xF0F8
#define SDHCI_ARASAN_OTAPDLY_REGISTER   0xF0FC
#define SDHCI_ITAPDLY_CHGWIN            0x200
#define SDHCI_ITAPDLY_ENABLE            0x100
#define SDHCI_OTAPDLY_ENABLE            0x40

#define SDHCI_TUNING_LOOP_COUNT		40
#define MMC_BANK2			0x2

struct arasan_sdhci_clk_data {
	int clk_phase_in[MMC_TIMING_MMC_HS400 + 1];
	int clk_phase_out[MMC_TIMING_MMC_HS400 + 1];
};

struct arasan_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct arasan_sdhci_priv {
	struct sdhci_host *host;
	struct arasan_sdhci_clk_data clk_data;
	u8 deviceid;
	u8 bank;
	u8 no_1p8;
};

#if defined(CONFIG_ARCH_ZYNQMP) || defined(CONFIG_ARCH_VERSAL)
/* Default settings for ZynqMP Clock Phases */
const u32 zynqmp_iclk_phases[] = {0, 63, 63, 0, 63,  0,   0, 183, 54,  0, 0};
const u32 zynqmp_oclk_phases[] = {0, 72, 60, 0, 60, 72, 135, 48, 72, 135, 0};

/* Default settings for Versal Clock Phases */
const u32 versal_iclk_phases[] = {0, 132, 132, 0, 132, 0, 0, 162, 90, 0, 0};
const u32 versal_oclk_phases[] = {0,  60, 48, 0, 48, 72, 90, 36, 60, 90, 0};

static const u8 mode2timing[] = {
	[MMC_LEGACY] = MMC_TIMING_LEGACY,
	[MMC_HS] = MMC_TIMING_MMC_HS,
	[SD_HS] = MMC_TIMING_SD_HS,
	[MMC_HS_52] = MMC_TIMING_UHS_SDR50,
	[MMC_DDR_52] = MMC_TIMING_UHS_DDR50,
	[UHS_SDR12] = MMC_TIMING_UHS_SDR12,
	[UHS_SDR25] = MMC_TIMING_UHS_SDR25,
	[UHS_SDR50] = MMC_TIMING_UHS_SDR50,
	[UHS_DDR50] = MMC_TIMING_UHS_DDR50,
	[UHS_SDR104] = MMC_TIMING_UHS_SDR104,
	[MMC_HS_200] = MMC_TIMING_MMC_HS200,
};

static void arasan_zynqmp_dll_reset(struct sdhci_host *host, u8 deviceid)
{
	u16 clk;
	unsigned long timeout;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~(SDHCI_CLOCK_CARD_EN);
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Issue DLL Reset */
	zynqmp_dll_reset(deviceid);

	/* Wait max 20 ms */
	timeout = 100;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
				& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			dev_err(mmc_dev(host->mmc),
				": Internal clock never stabilised.\n");
			return;
		}
		timeout--;
		udelay(1000);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}

static int arasan_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	u32 ctrl;
	struct sdhci_host *host;
	struct arasan_sdhci_priv *priv = dev_get_priv(mmc->dev);
	char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;
	u8 deviceid;

	debug("%s\n", __func__);

	host = priv->host;
	deviceid = priv->deviceid;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	mdelay(1);

	arasan_zynqmp_dll_reset(host, deviceid);

	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_INT_ENABLE);
	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_SIGNAL_ENABLE);

	do {
		cmd.cmdidx = opcode;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		data.blocksize = 64;
		data.blocks = 1;
		data.flags = MMC_DATA_READ;

		if (tuning_loop_counter-- == 0)
			break;

		if (cmd.cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200 &&
		    mmc->bus_width == 8)
			data.blocksize = 128;

		sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
						    data.blocksize),
			     SDHCI_BLOCK_SIZE);
		sdhci_writew(host, data.blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

		mmc_send_cmd(mmc, &cmd, NULL);
		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

		if (cmd.cmdidx == MMC_CMD_SEND_TUNING_BLOCK)
			udelay(1);

	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	if (tuning_loop_counter < 0) {
		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		sdhci_writel(host, ctrl, SDHCI_HOST_CONTROL2);
	}

	if (!(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		printf("%s:Tuning failed\n", __func__);
		return -1;
	}

	udelay(1);
	arasan_zynqmp_dll_reset(host, deviceid);

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return 0;
}

/**
 * sdhci_zynqmp_sdcardclk_set_phase - Set the SD Output Clock Tap Delays
 *
 * Set the SD Output Clock Tap Delays for Output path
 *
 * @host:		Pointer to the sdhci_host structure.
 * @degrees:		The clock phase shift between 0 - 359.
 * Return: 0 on success and error value on error
 */
static int sdhci_zynqmp_sdcardclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(host->mmc->dev);
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 tap_delay, tap_max = 0;
	int ret;
	int timing = mode2timing[mmc->selected_mode];

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * ZynqMP does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300 ||
	    timing == MMC_TIMING_LEGACY ||
	    timing == MMC_TIMING_UHS_SDR12 || !degrees)
		return 0;

	switch (timing) {
	case MMC_TIMING_MMC_HS:
	case MMC_TIMING_SD_HS:
	case MMC_TIMING_UHS_SDR25:
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		/* For 50MHz clock, 30 Taps are available */
		tap_max = 30;
		break;
	case MMC_TIMING_UHS_SDR50:
		/* For 100MHz clock, 15 Taps are available */
		tap_max = 15;
		break;
	case MMC_TIMING_UHS_SDR104:
	case MMC_TIMING_MMC_HS200:
		/* For 200MHz clock, 8 Taps are available */
		tap_max = 8;
	default:
		break;
	}

	tap_delay = (degrees * tap_max) / 360;

	arasan_zynqmp_set_tapdelay(priv->deviceid, 0, tap_delay);

	return ret;
}

/**
 * sdhci_zynqmp_sampleclk_set_phase - Set the SD Input Clock Tap Delays
 *
 * Set the SD Input Clock Tap Delays for Input path
 *
 * @host:		Pointer to the sdhci_host structure.
 * @degrees:		The clock phase shift between 0 - 359.
 * Return: 0 on success and error value on error
 */
static int sdhci_zynqmp_sampleclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(host->mmc->dev);
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 tap_delay, tap_max = 0;
	int ret;
	int timing = mode2timing[mmc->selected_mode];

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * ZynqMP does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300 ||
	    timing == MMC_TIMING_LEGACY ||
	    timing == MMC_TIMING_UHS_SDR12 || !degrees)
		return 0;

	switch (timing) {
	case MMC_TIMING_MMC_HS:
	case MMC_TIMING_SD_HS:
	case MMC_TIMING_UHS_SDR25:
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		/* For 50MHz clock, 120 Taps are available */
		tap_max = 120;
		break;
	case MMC_TIMING_UHS_SDR50:
		/* For 100MHz clock, 60 Taps are available */
		tap_max = 60;
		break;
	case MMC_TIMING_UHS_SDR104:
	case MMC_TIMING_MMC_HS200:
		/* For 200MHz clock, 30 Taps are available */
		tap_max = 30;
	default:
		break;
	}

	tap_delay = (degrees * tap_max) / 360;

	arasan_zynqmp_set_tapdelay(priv->deviceid, tap_delay, 0);

	return ret;
}

/**
 * sdhci_versal_sdcardclk_set_phase - Set the SD Output Clock Tap Delays
 *
 * Set the SD Output Clock Tap Delays for Output path
 *
 * @host:		Pointer to the sdhci_host structure.
 * @degrees		The clock phase shift between 0 - 359.
 * Return: 0 on success and error value on error
 */
static int sdhci_versal_sdcardclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 tap_delay, tap_max = 0;
	int ret;
	int timing = mode2timing[mmc->selected_mode];

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * Versal does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300 ||
	    timing == MMC_TIMING_LEGACY ||
	    timing == MMC_TIMING_UHS_SDR12 || !degrees)
		return 0;

	switch (timing) {
	case MMC_TIMING_MMC_HS:
	case MMC_TIMING_SD_HS:
	case MMC_TIMING_UHS_SDR25:
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		/* For 50MHz clock, 30 Taps are available */
		tap_max = 30;
		break;
	case MMC_TIMING_UHS_SDR50:
		/* For 100MHz clock, 15 Taps are available */
		tap_max = 15;
		break;
	case MMC_TIMING_UHS_SDR104:
	case MMC_TIMING_MMC_HS200:
		/* For 200MHz clock, 8 Taps are available */
		tap_max = 8;
	default:
		break;
	}

	tap_delay = (degrees * tap_max) / 360;

	/* Set the Clock Phase */
	if (tap_delay) {
		u32 regval;

		regval = sdhci_readl(host, SDHCI_ARASAN_OTAPDLY_REGISTER);
		regval |= SDHCI_OTAPDLY_ENABLE;
		sdhci_writel(host, regval, SDHCI_ARASAN_OTAPDLY_REGISTER);
		regval |= tap_delay;
		sdhci_writel(host, regval, SDHCI_ARASAN_OTAPDLY_REGISTER);
	}

	return ret;
}

/**
 * sdhci_versal_sampleclk_set_phase - Set the SD Input Clock Tap Delays
 *
 * Set the SD Input Clock Tap Delays for Input path
 *
 * @host:		Pointer to the sdhci_host structure.
 * @degrees		The clock phase shift between 0 - 359.
 * Return: 0 on success and error value on error
 */
static int sdhci_versal_sampleclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 tap_delay, tap_max = 0;
	int ret;
	int timing = mode2timing[mmc->selected_mode];

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * Versal does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300 ||
	    timing == MMC_TIMING_LEGACY ||
	    timing == MMC_TIMING_UHS_SDR12 || !degrees)
		return 0;

	switch (timing) {
	case MMC_TIMING_MMC_HS:
	case MMC_TIMING_SD_HS:
	case MMC_TIMING_UHS_SDR25:
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		/* For 50MHz clock, 120 Taps are available */
		tap_max = 120;
		break;
	case MMC_TIMING_UHS_SDR50:
		/* For 100MHz clock, 60 Taps are available */
		tap_max = 60;
		break;
	case MMC_TIMING_UHS_SDR104:
	case MMC_TIMING_MMC_HS200:
		/* For 200MHz clock, 30 Taps are available */
		tap_max = 30;
	default:
		break;
	}

	tap_delay = (degrees * tap_max) / 360;

	/* Set the Clock Phase */
	if (tap_delay) {
		u32 regval;

		regval = sdhci_readl(host, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= SDHCI_ITAPDLY_CHGWIN;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= SDHCI_ITAPDLY_ENABLE;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= tap_delay;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval &= ~SDHCI_ITAPDLY_CHGWIN;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
	}

	return ret;
}

static void arasan_sdhci_set_tapdelay(struct sdhci_host *host)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(host->mmc->dev);
	struct arasan_sdhci_clk_data *clk_data = &priv->clk_data;
	struct mmc *mmc = (struct mmc *)host->mmc;
	struct udevice *dev = mmc->dev;
	u8 timing = mode2timing[mmc->selected_mode];
	u32 iclk_phase = clk_data->clk_phase_in[timing];
	u32 oclk_phase = clk_data->clk_phase_out[timing];

	dev_dbg(dev, "%s, host:%s, mode:%d\n", __func__, host->name, timing);

	if (IS_ENABLED(CONFIG_ARCH_ZYNQMP) &&
	    device_is_compatible(dev, "xlnx,zynqmp-8.9a")) {
		sdhci_zynqmp_sampleclk_set_phase(host, iclk_phase);
		sdhci_zynqmp_sdcardclk_set_phase(host, oclk_phase);
	} else if (IS_ENABLED(CONFIG_ARCH_VERSAL) &&
		   device_is_compatible(dev, "xlnx,versal-8.9a")) {
		sdhci_versal_sampleclk_set_phase(host, iclk_phase);
		sdhci_versal_sdcardclk_set_phase(host, oclk_phase);
	}
}

static void arasan_dt_read_clk_phase(struct udevice *dev, unsigned char timing,
				     const char *prop)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);
	struct arasan_sdhci_clk_data *clk_data = &priv->clk_data;
	u32 clk_phase[2] = {0};

	/*
	 * Read Tap Delay values from DT, if the DT does not contain the
	 * Tap Values then use the pre-defined values
	 */
	if (dev_read_u32_array(dev, prop, &clk_phase[0], 2)) {
		dev_dbg(dev, "Using predefined clock phase for %s = %d %d\n",
			prop, clk_data->clk_phase_in[timing],
			clk_data->clk_phase_out[timing]);
		return;
	}

	/* The values read are Input and Output Clock Delays in order */
	clk_data->clk_phase_in[timing] = clk_phase[0];
	clk_data->clk_phase_out[timing] = clk_phase[1];
}

/**
 * arasan_dt_parse_clk_phases - Read Tap Delay values from DT
 *
 * Called at initialization to parse the values of Tap Delays.
 *
 * @dev:                Pointer to our struct udevice.
 */
static void arasan_dt_parse_clk_phases(struct udevice *dev)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);
	struct arasan_sdhci_clk_data *clk_data = &priv->clk_data;
	int i;

	if (IS_ENABLED(CONFIG_ARCH_ZYNQMP) &&
	    device_is_compatible(dev, "xlnx,zynqmp-8.9a")) {
		for (i = 0; i <= MMC_TIMING_MMC_HS400; i++) {
			clk_data->clk_phase_in[i] = zynqmp_iclk_phases[i];
			clk_data->clk_phase_out[i] = zynqmp_oclk_phases[i];
		}

		if (priv->bank == MMC_BANK2) {
			clk_data->clk_phase_out[MMC_TIMING_UHS_SDR104] = 90;
			clk_data->clk_phase_out[MMC_TIMING_MMC_HS200] = 90;
		}
	}

	if (IS_ENABLED(CONFIG_ARCH_VERSAL) &&
	    device_is_compatible(dev, "xlnx,versal-8.9a")) {
		for (i = 0; i <= MMC_TIMING_MMC_HS400; i++) {
			clk_data->clk_phase_in[i] = versal_iclk_phases[i];
			clk_data->clk_phase_out[i] = versal_oclk_phases[i];
		}
	}

	arasan_dt_read_clk_phase(dev, MMC_TIMING_LEGACY,
				 "clk-phase-legacy");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_MMC_HS,
				 "clk-phase-mmc-hs");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_SD_HS,
				 "clk-phase-sd-hs");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_UHS_SDR12,
				 "clk-phase-uhs-sdr12");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_UHS_SDR25,
				 "clk-phase-uhs-sdr25");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_UHS_SDR50,
				 "clk-phase-uhs-sdr50");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_UHS_SDR104,
				 "clk-phase-uhs-sdr104");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_UHS_DDR50,
				 "clk-phase-uhs-ddr50");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_MMC_DDR52,
				 "clk-phase-mmc-ddr52");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_MMC_HS200,
				 "clk-phase-mmc-hs200");
	arasan_dt_read_clk_phase(dev, MMC_TIMING_MMC_HS400,
				 "clk-phase-mmc-hs400");
}

static void arasan_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u32 reg;

	if (!IS_SD(mmc))
		return;

	if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
	}

	if (mmc->selected_mode > SD_HS &&
	    mmc->selected_mode <= MMC_HS_200)
		sdhci_set_uhs_timing(host);
}

const struct sdhci_ops arasan_ops = {
	.platform_execute_tuning = &arasan_sdhci_execute_tuning,
	.set_delay = &arasan_sdhci_set_tapdelay,
	.set_control_reg = &arasan_sdhci_set_control_reg,
};
#endif

static int arasan_sdhci_probe(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host;
	struct clk clk;
	unsigned long clock;
	int ret;

	host = priv->host;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return clock;
	}

	debug("%s: CLK %ld\n", __func__, clock);

	ret = clk_enable(&clk);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_BROKEN_R1B;

#ifdef CONFIG_ZYNQ_HISPD_BROKEN
	host->quirks |= SDHCI_QUIRK_BROKEN_HISPD_MODE;
#endif

	if (priv->no_1p8)
		host->quirks |= SDHCI_QUIRK_NO_1_8_V;

	plat->cfg.f_max = CONFIG_ZYNQ_SDHCI_MAX_FREQ;

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	host->max_clk = clock;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;

	ret = sdhci_setup_cfg(&plat->cfg, host, plat->cfg.f_max,
			      CONFIG_ZYNQ_SDHCI_MIN_FREQ);
	if (ret)
		return ret;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int arasan_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);

	priv->host = calloc(1, sizeof(struct sdhci_host));
	if (!priv->host)
		return -1;

	priv->host->name = dev->name;

#if defined(CONFIG_ARCH_ZYNQMP) || defined(CONFIG_ARCH_VERSAL)
	priv->host->ops = &arasan_ops;
	arasan_dt_parse_clk_phases(dev);
#endif

	priv->host->ioaddr = (void *)dev_read_addr(dev);
	if (IS_ERR(priv->host->ioaddr))
		return PTR_ERR(priv->host->ioaddr);

	priv->deviceid = dev_read_u32_default(dev, "xlnx,device_id", -1);
	priv->bank = dev_read_u32_default(dev, "xlnx,mio-bank", 0);
	priv->no_1p8 = dev_read_bool(dev, "no-1-8-v");

	return 0;
}

static int arasan_sdhci_bind(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id arasan_sdhci_ids[] = {
	{ .compatible = "arasan,sdhci-8.9a" },
	{ }
};

U_BOOT_DRIVER(arasan_sdhci_drv) = {
	.name		= "arasan_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= arasan_sdhci_ids,
	.ofdata_to_platdata = arasan_sdhci_ofdata_to_platdata,
	.ops		= &sdhci_ops,
	.bind		= arasan_sdhci_bind,
	.probe		= arasan_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct arasan_sdhci_priv),
	.platdata_auto_alloc_size = sizeof(struct arasan_sdhci_plat),
};
