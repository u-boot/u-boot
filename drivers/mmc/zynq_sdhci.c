/*
 * (C) Copyright 2013 - 2015 Xilinx, Inc.
 *
 * Xilinx Zynq SD Host Controller Interface
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <mmc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <zynqmp_tap_delay.h>
#include "mmc_private.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_ZYNQ_SDHCI_MIN_FREQ
# define CONFIG_ZYNQ_SDHCI_MIN_FREQ	0
#endif

struct arasan_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	unsigned int f_max;
};

struct arasan_sdhci_priv {
	struct sdhci_host *host;
	u8 deviceid;
	u8 bank;
	u8 no_1p8;
	bool pwrseq;
};

#if defined(CONFIG_ARCH_ZYNQMP)
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
	u8 tuning_loop_counter = 40;
	u8 deviceid;

	debug("%s\n", __func__);

	host = priv->host;
	deviceid = priv->deviceid;

	ctrl = sdhci_readw(host, SDHCI_HOST_CTRL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CTRL2);

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
		ctrl = sdhci_readw(host, SDHCI_HOST_CTRL2);

		if (cmd.cmdidx == MMC_CMD_SEND_TUNING_BLOCK)
			udelay(1);

	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	if (tuning_loop_counter < 0) {
		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		sdhci_writel(host, ctrl, SDHCI_HOST_CTRL2);
	}

	if (!(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		debug("%s:Tuning failed\n", __func__);
		return -1;
	} else {
		udelay(1);
		arasan_zynqmp_dll_reset(host, deviceid);
	}

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return 0;
}

static void arasan_sdhci_set_tapdelay(struct sdhci_host *host)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(host->mmc->dev);
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 uhsmode;

	if (mmc->is_uhs)
		uhsmode = mmc->uhsmode;
	else if (mmc->card_caps & MMC_MODE_HS)
		uhsmode = MMC_TIMING_HS;
	else if (mmc->card_caps & MMC_MODE_HS200)
		uhsmode = MMC_TIMING_HS200;
	else
		return;

	debug("%s, host:%s devId:%d, bank:%d, mode:%d\n", __func__, host->name,
	      priv->deviceid, priv->bank, uhsmode);
	if ((uhsmode >= MMC_TIMING_UHS_SDR25) &&
	    (uhsmode <= MMC_TIMING_HS200))
		arasan_zynqmp_set_tapdelay(priv->deviceid, uhsmode,
					   priv->bank);
}
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

	host = priv->host;

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_BROKEN_R1B |
		       SDHCI_QUIRK_USE_ACMD12;

#ifdef CONFIG_ZYNQ_HISPD_BROKEN
	host->quirks |= SDHCI_QUIRK_NO_HISPD_BIT;
#endif

	if (priv->no_1p8)
		host->quirks |= SDHCI_QUIRK_NO_1_8_V;

	host->max_clk = clock;

	ret = sdhci_setup_cfg(&plat->cfg, host, plat->f_max,
			      CONFIG_ZYNQ_SDHCI_MIN_FREQ);
	host->mmc = &plat->mmc;
	if (ret)
		return ret;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

#if defined(CONFIG_ARCH_ZYNQMP)
	host->set_delay = arasan_sdhci_set_tapdelay;
	host->platform_execute_tuning = arasan_sdhci_execute_tuning;
#endif

	if (priv->pwrseq) {
		debug("Unsupported mmcpwrseq for %s\n", dev->name);
		return 0;
	}

	ret = sdhci_probe(dev);
	if (ret)
		return ret;

	return mmc_init(&plat->mmc);
}

static int arasan_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_platdata(dev);
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);

	priv->host = calloc(1, sizeof(struct sdhci_host));
	if (priv->host == NULL)
		return -1;

	priv->host->name = dev->name;
	priv->host->ioaddr = (void *)devfdt_get_addr(dev);

	plat->f_max = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				"max-frequency", CONFIG_ZYNQ_SDHCI_MAX_FREQ);

	priv->deviceid = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"xlnx,device_id", -1);
	priv->bank = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				    "xlnx,mio_bank", -1);

	if (fdt_get_property(gd->fdt_blob, dev_of_offset(dev), "no-1-8-v", NULL)
#if defined(CONFIG_ARCH_ZYNQMP)
	    || (chip_id(VERSION) == ZYNQMP_SILICON_V1)
#endif
	    )
		priv->no_1p8 = 1;
	else
		priv->no_1p8 = 0;

	if (fdt_get_property(gd->fdt_blob, dev_of_offset(dev), "mmc-pwrseq",
			     NULL))
		priv->pwrseq = true;

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
