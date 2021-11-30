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
#include <asm/cache.h>
#include <malloc.h>
#include <sdhci.h>
#include <zynqmp_firmware.h>

#define SDHCI_ARASAN_ITAPDLY_REGISTER	0xF0F8
#define SDHCI_ARASAN_ITAPDLY_SEL_MASK	GENMASK(7, 0)
#define SDHCI_ARASAN_OTAPDLY_REGISTER	0xF0FC
#define SDHCI_ARASAN_OTAPDLY_SEL_MASK	GENMASK(5, 0)
#define SDHCI_ITAPDLY_CHGWIN		BIT(9)
#define SDHCI_ITAPDLY_ENABLE		BIT(8)
#define SDHCI_OTAPDLY_ENABLE		BIT(6)

#define SDHCI_TUNING_LOOP_COUNT		40
#define MMC_BANK2			0x2

#define SD_DLL_CTRL			0xFF180358
#define SD_ITAP_DLY			0xFF180314
#define SD_OTAP_DLY			0xFF180318
#define SD0_DLL_RST			BIT(2)
#define SD1_DLL_RST			BIT(18)
#define SD0_ITAPCHGWIN			BIT(9)
#define SD1_ITAPCHGWIN			BIT(25)
#define SD0_ITAPDLYENA			BIT(8)
#define SD1_ITAPDLYENA			BIT(24)
#define SD0_ITAPDLYSEL_MASK		GENMASK(7, 0)
#define SD1_ITAPDLYSEL_MASK		GENMASK(23, 16)
#define SD0_OTAPDLYSEL_MASK		GENMASK(5, 0)
#define SD1_OTAPDLYSEL_MASK		GENMASK(21, 16)

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

/* For Versal platforms zynqmp_mmio_write() won't be available */
__weak int zynqmp_mmio_write(const u32 address, const u32 mask, const u32 value)
{
	return 0;
}

__weak int xilinx_pm_request(u32 api_id, u32 arg0, u32 arg1, u32 arg2,
			     u32 arg3, u32 *ret_payload)
{
	return 0;
}

#if defined(CONFIG_ARCH_ZYNQMP) || defined(CONFIG_ARCH_VERSAL)
/* Default settings for ZynqMP Clock Phases */
static const u32 zynqmp_iclk_phases[] = {0, 63, 63, 0, 63,  0,
					 0, 183, 54,  0, 0};
static const u32 zynqmp_oclk_phases[] = {0, 72, 60, 0, 60, 72,
					 135, 48, 72, 135, 0};

/* Default settings for Versal Clock Phases */
static const u32 versal_iclk_phases[] = {0, 132, 132, 0, 132,
					 0, 0, 162, 90, 0, 0};
static const u32 versal_oclk_phases[] = {0,  60, 48, 0, 48, 72,
					 90, 36, 60, 90, 0};

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

static inline int arasan_zynqmp_set_in_tapdelay(u8 node_id, u32 itap_delay)
{
	int ret;

	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
		if (node_id == NODE_SD_0) {
			ret = zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN,
						SD0_ITAPCHGWIN);
			if (ret)
				return ret;

			ret = zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYENA,
						SD0_ITAPDLYENA);
			if (ret)
				return ret;

			ret = zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYSEL_MASK,
						itap_delay);
			if (ret)
				return ret;

			ret = zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN, 0);
			if (ret)
				return ret;
		}
		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN,
					SD1_ITAPCHGWIN);
		if (ret)
			return ret;

		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYENA,
					SD1_ITAPDLYENA);
		if (ret)
			return ret;

		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYSEL_MASK,
					(itap_delay << 16));
		if (ret)
			return ret;

		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN, 0);
		if (ret)
			return ret;
	} else {
		return xilinx_pm_request(PM_IOCTL, (u32)node_id,
					 IOCTL_SET_SD_TAPDELAY,
					 PM_TAPDELAY_INPUT, itap_delay, NULL);
	}

	return 0;
}

static inline int arasan_zynqmp_set_out_tapdelay(u8 node_id, u32 otap_delay)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
		if (node_id == NODE_SD_0)
			return zynqmp_mmio_write(SD_OTAP_DLY,
						 SD0_OTAPDLYSEL_MASK,
						 otap_delay);

		return zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					 (otap_delay << 16));
	} else {
		return xilinx_pm_request(PM_IOCTL, (u32)node_id,
					 IOCTL_SET_SD_TAPDELAY,
					 PM_TAPDELAY_OUTPUT, otap_delay, NULL);
	}
}

static inline int zynqmp_dll_reset(u8 node_id, u32 type)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
		if (node_id == NODE_SD_0)
			return zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST,
						 type == PM_DLL_RESET_ASSERT ?
						 SD0_DLL_RST : 0);

		return zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST,
					 type == PM_DLL_RESET_ASSERT ?
					 SD1_DLL_RST : 0);
	} else {
		return xilinx_pm_request(PM_IOCTL, (u32)node_id,
					 IOCTL_SD_DLL_RESET, type, 0, NULL);
	}
}

static int arasan_zynqmp_dll_reset(struct sdhci_host *host, u8 node_id)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	struct udevice *dev = mmc->dev;
	unsigned long timeout;
	int ret;
	u16 clk;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~(SDHCI_CLOCK_CARD_EN);
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Issue DLL Reset */
	ret = zynqmp_dll_reset(node_id, PM_DLL_RESET_ASSERT);
	if (ret) {
		dev_err(dev, "dll_reset assert failed with err: %d\n", ret);
		return ret;
	}

	/* Allow atleast 1ms delay for proper DLL reset */
	mdelay(1);
	ret = zynqmp_dll_reset(node_id, PM_DLL_RESET_RELEASE);
	if (ret) {
		dev_err(dev, "dll_reset release failed with err: %d\n", ret);
		return ret;
	}

	/* Wait max 20 ms */
	timeout = 100;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
				& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			dev_err(dev, ": Internal clock never stabilised.\n");
			return -EBUSY;
		}
		timeout--;
		udelay(1000);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	return 0;
}

static int arasan_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	u32 ctrl;
	struct sdhci_host *host;
	struct arasan_sdhci_priv *priv = dev_get_priv(mmc->dev);
	char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;
	u8 node_id = priv->deviceid ? NODE_SD_1 : NODE_SD_0;

	debug("%s\n", __func__);

	host = priv->host;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	mdelay(1);

	arasan_zynqmp_dll_reset(host, node_id);

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
	arasan_zynqmp_dll_reset(host, node_id);

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
 * @host:		Pointer to the sdhci_host structure.
 * @degrees:		The clock phase shift between 0 - 359.
 * Return: 0
 *
 * Set the SD Output Clock Tap Delays for Output path
 */
static int sdhci_zynqmp_sdcardclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	struct udevice *dev = mmc->dev;
	struct arasan_sdhci_priv *priv = dev_get_priv(mmc->dev);
	u8 node_id = priv->deviceid ? NODE_SD_1 : NODE_SD_0;
	u8 tap_delay, tap_max = 0;
	int timing = mode2timing[mmc->selected_mode];
	int ret;

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * ZynqMP does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300)
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

	/* Limit output tap_delay value to 6 bits */
	tap_delay &= SDHCI_ARASAN_OTAPDLY_SEL_MASK;

	/* Set the Clock Phase */
	ret = arasan_zynqmp_set_out_tapdelay(node_id, tap_delay);
	if (ret) {
		dev_err(dev, "Error setting output Tap Delay\n");
		return ret;
	}

	/* Release DLL Reset */
	ret = zynqmp_dll_reset(node_id, PM_DLL_RESET_RELEASE);
	if (ret) {
		dev_err(dev, "dll_reset release failed with err: %d\n", ret);
		return ret;
	}

	return 0;
}

/**
 * sdhci_zynqmp_sampleclk_set_phase - Set the SD Input Clock Tap Delays
 *
 * @host:		Pointer to the sdhci_host structure.
 * @degrees:		The clock phase shift between 0 - 359.
 * Return: 0
 *
 * Set the SD Input Clock Tap Delays for Input path
 */
static int sdhci_zynqmp_sampleclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	struct udevice *dev = mmc->dev;
	struct arasan_sdhci_priv *priv = dev_get_priv(mmc->dev);
	u8 node_id = priv->deviceid ? NODE_SD_1 : NODE_SD_0;
	u8 tap_delay, tap_max = 0;
	int timing = mode2timing[mmc->selected_mode];
	int ret;

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * ZynqMP does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300)
		return 0;

	/* Assert DLL Reset */
	ret = zynqmp_dll_reset(node_id, PM_DLL_RESET_ASSERT);
	if (ret) {
		dev_err(dev, "dll_reset assert failed with err: %d\n", ret);
		return ret;
	}

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

	/* Limit input tap_delay value to 8 bits */
	tap_delay &= SDHCI_ARASAN_ITAPDLY_SEL_MASK;

	ret = arasan_zynqmp_set_in_tapdelay(node_id, tap_delay);
	if (ret) {
		dev_err(dev, "Error setting Input Tap Delay\n");
		return ret;
	}

	return 0;
}

/**
 * sdhci_versal_sdcardclk_set_phase - Set the SD Output Clock Tap Delays
 *
 * @host:		Pointer to the sdhci_host structure.
 * @degrees:		The clock phase shift between 0 - 359.
 * Return: 0
 *
 * Set the SD Output Clock Tap Delays for Output path
 */
static int sdhci_versal_sdcardclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 tap_delay, tap_max = 0;
	int timing = mode2timing[mmc->selected_mode];
	u32 regval;

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * Versal does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300)
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

	/* Limit output tap_delay value to 6 bits */
	tap_delay &= SDHCI_ARASAN_OTAPDLY_SEL_MASK;

	/* Set the Clock Phase */
	regval = sdhci_readl(host, SDHCI_ARASAN_OTAPDLY_REGISTER);
	regval |= SDHCI_OTAPDLY_ENABLE;
	sdhci_writel(host, regval, SDHCI_ARASAN_OTAPDLY_REGISTER);
	regval &= ~SDHCI_ARASAN_OTAPDLY_SEL_MASK;
	regval |= tap_delay;
	sdhci_writel(host, regval, SDHCI_ARASAN_OTAPDLY_REGISTER);

	return 0;
}

/**
 * sdhci_versal_sampleclk_set_phase - Set the SD Input Clock Tap Delays
 *
 * @host:		Pointer to the sdhci_host structure.
 * @degrees:		The clock phase shift between 0 - 359.
 * Return: 0
 *
 * Set the SD Input Clock Tap Delays for Input path
 */
static int sdhci_versal_sampleclk_set_phase(struct sdhci_host *host,
					    int degrees)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 tap_delay, tap_max = 0;
	int timing = mode2timing[mmc->selected_mode];
	u32 regval;

	/*
	 * This is applicable for SDHCI_SPEC_300 and above
	 * Versal does not set phase for <=25MHz clock.
	 * If degrees is zero, no need to do anything.
	 */
	if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300)
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

	/* Limit input tap_delay value to 8 bits */
	tap_delay &= SDHCI_ARASAN_ITAPDLY_SEL_MASK;

	/* Set the Clock Phase */
	regval = sdhci_readl(host, SDHCI_ARASAN_ITAPDLY_REGISTER);
	regval |= SDHCI_ITAPDLY_CHGWIN;
	sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
	regval |= SDHCI_ITAPDLY_ENABLE;
	sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
	regval &= ~SDHCI_ARASAN_ITAPDLY_SEL_MASK;
	regval |= tap_delay;
	sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
	regval &= ~SDHCI_ITAPDLY_CHGWIN;
	sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);

	return 0;
}

static int arasan_sdhci_set_tapdelay(struct sdhci_host *host)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(host->mmc->dev);
	struct arasan_sdhci_clk_data *clk_data = &priv->clk_data;
	struct mmc *mmc = (struct mmc *)host->mmc;
	struct udevice *dev = mmc->dev;
	u8 timing = mode2timing[mmc->selected_mode];
	u32 iclk_phase = clk_data->clk_phase_in[timing];
	u32 oclk_phase = clk_data->clk_phase_out[timing];
	int ret;

	dev_dbg(dev, "%s, host:%s, mode:%d\n", __func__, host->name, timing);

	if (IS_ENABLED(CONFIG_ARCH_ZYNQMP) &&
	    device_is_compatible(dev, "xlnx,zynqmp-8.9a")) {
		ret = sdhci_zynqmp_sampleclk_set_phase(host, iclk_phase);
		if (ret)
			return ret;

		ret = sdhci_zynqmp_sdcardclk_set_phase(host, oclk_phase);
		if (ret)
			return ret;
	} else if (IS_ENABLED(CONFIG_ARCH_VERSAL) &&
		   device_is_compatible(dev, "xlnx,versal-8.9a")) {
		ret = sdhci_versal_sampleclk_set_phase(host, iclk_phase);
		if (ret)
			return ret;

		ret = sdhci_versal_sdcardclk_set_phase(host, oclk_phase);
		if (ret)
			return ret;
	}

	return 0;
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
 * @dev:                Pointer to our struct udevice.
 *
 * Called at initialization to parse the values of Tap Delays.
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

static const struct sdhci_ops arasan_ops = {
	.platform_execute_tuning	= &arasan_sdhci_execute_tuning,
	.set_delay = &arasan_sdhci_set_tapdelay,
	.set_control_reg = &sdhci_set_control_reg,
};
#endif

static int arasan_sdhci_probe(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_plat(dev);
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
	if (ret) {
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

	/*
	 * WORKAROUND: Versal platforms have an issue with card detect state.
	 * Due to this, host controller is switching off voltage to sd card
	 * causing sd card timeout error. Workaround this by adding a wait for
	 * 1000msec till the card detect state gets stable.
	 */
	if (IS_ENABLED(CONFIG_ARCH_VERSAL)) {
		u32 timeout = 1000;

		while (((sdhci_readl(host, SDHCI_PRESENT_STATE) &
			 SDHCI_CARD_STATE_STABLE) == 0) && timeout--) {
			mdelay(1);
		}
		if (!timeout) {
			dev_err(dev, "Sdhci card detect state not stable\n");
			return -ETIMEDOUT;
		}
	}

	return sdhci_probe(dev);
}

static int arasan_sdhci_of_to_plat(struct udevice *dev)
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
	struct arasan_sdhci_plat *plat = dev_get_plat(dev);

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
	.of_to_plat = arasan_sdhci_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= arasan_sdhci_bind,
	.probe		= arasan_sdhci_probe,
	.priv_auto	= sizeof(struct arasan_sdhci_priv),
	.plat_auto	= sizeof(struct arasan_sdhci_plat),
};
