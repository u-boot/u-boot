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

#define SDHCI_ITAPDLYSEL_SD_HSD		0x00000015
#define SDHCI_ITAPDLYSEL_SDR25		0x00000015
#define SDHCI_ITAPDLYSEL_SDR50		0x00000000
#define SDHCI_ITAPDLYSEL_SDR104_B2	0x00000000
#define SDHCI_ITAPDLYSEL_SDR104_B0	0x00000000
#define SDHCI_ITAPDLYSEL_MMC_HSD	0x00000015
#define SDHCI_ITAPDLYSEL_SD_DDR50	0x0000003D
#define SDHCI_ITAPDLYSEL_MMC_DDR52	0x00000012
#define SDHCI_ITAPDLYSEL_MMC_HS200_B2	0x00000000
#define SDHCI_ITAPDLYSEL_MMC_HS200_B0	0x00000000
#define SDHCI_OTAPDLYSEL_SD_HSD		0x00000005
#define SDHCI_OTAPDLYSEL_SDR25		0x00000005
#define SDHCI_OTAPDLYSEL_SDR50		0x00000003
#define SDHCI_OTAPDLYSEL_SDR104_B0	0x00000003
#define SDHCI_OTAPDLYSEL_SDR104_B2	0x00000002
#define SDHCI_OTAPDLYSEL_MMC_HSD	0x00000006
#define SDHCI_OTAPDLYSEL_SD_DDR50	0x00000004
#define SDHCI_OTAPDLYSEL_MMC_DDR52	0x00000006
#define SDHCI_OTAPDLYSEL_MMC_HS200_B0	0x00000003
#define SDHCI_OTAPDLYSEL_MMC_HS200_B2	0x00000002

#define MMC_BANK2			0x2

struct arasan_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	unsigned int f_max;
};

struct arasan_sdhci_priv {
	struct sdhci_host *host;
	u32 itapdly[MMC_MAX_BUS_SPEED];
	u32 otapdly[MMC_MAX_BUS_SPEED];
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
	u32 itap_delay;
	u32 otap_delay;

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
	    (uhsmode <= MMC_TIMING_HS200)) {
		itap_delay = priv->itapdly[uhsmode];
		otap_delay = priv->otapdly[uhsmode];
		arasan_zynqmp_set_tapdelay(priv->deviceid, itap_delay,
					   otap_delay);
	}
}

/**
 * arasan_zynqmp_dt_parse_tap_delays - Read Tap Delay values from DT
 *
 * Called at initialization to parse the values of Tap Delays.
 *
 * @dev:                Pointer to our struct udevice.
 */
static void arasan_zynqmp_dt_parse_tap_delays(struct udevice *dev)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);
	u32 *itapdly = priv->itapdly;
	u32 *otapdly = priv->otapdly;
	int ret;

	/*
	 * Read Tap Delay values from DT, if the DT does not contain the
	 * Tap Values then use the pre-defined values
	 */
	ret = dev_read_u32(dev, "xlnx,itap-delay-sd-hsd",
			   &itapdly[SD_HS_BUS_SPEED]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for SD_HS_BUS_SPEED\n");
		itapdly[SD_HS_BUS_SPEED] = SDHCI_ITAPDLYSEL_SD_HSD;
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-sd-hsd",
			   &otapdly[SD_HS_BUS_SPEED]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for SD_HS_BUS_SPEED\n");
		otapdly[SD_HS_BUS_SPEED] = SDHCI_OTAPDLYSEL_SD_HSD;
	}

	ret = dev_read_u32(dev, "xlnx,itap-delay-sdr25",
			   &itapdly[MMC_TIMING_UHS_SDR25]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for MMC_TIMING_UHS_SDR25\n");
		itapdly[MMC_TIMING_UHS_SDR25] = SDHCI_ITAPDLYSEL_SDR25;
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-sdr25",
			   &otapdly[MMC_TIMING_UHS_SDR25]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for MMC_TIMING_UHS_SDR25\n");
		otapdly[MMC_TIMING_UHS_SDR25] = SDHCI_OTAPDLYSEL_SDR25;
	}

	ret = dev_read_u32(dev, "xlnx,itap-delay-sdr50",
			   &itapdly[MMC_TIMING_UHS_SDR50]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for MMC_TIMING_UHS_SDR50\n");
		itapdly[MMC_TIMING_UHS_SDR50] = SDHCI_ITAPDLYSEL_SDR50;
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-sdr50",
			   &otapdly[MMC_TIMING_UHS_SDR50]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for MMC_TIMING_UHS_SDR50\n");
		otapdly[MMC_TIMING_UHS_SDR50] = SDHCI_OTAPDLYSEL_SDR50;
	}

	ret = dev_read_u32(dev, "xlnx,itap-delay-sd-ddr50",
			   &itapdly[MMC_TIMING_UHS_DDR50]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for MMC_TIMING_UHS_DDR50\n");
		itapdly[MMC_TIMING_UHS_DDR50] = SDHCI_ITAPDLYSEL_SD_DDR50;
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-sd-ddr50",
			   &otapdly[MMC_TIMING_UHS_DDR50]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for MMC_TIMING_UHS_DDR50\n");
		otapdly[MMC_TIMING_UHS_DDR50] = SDHCI_OTAPDLYSEL_SD_DDR50;
	}

	ret = dev_read_u32(dev, "xlnx,itap-delay-mmc-hsd",
			   &itapdly[MMC_HS_BUS_SPEED]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for MMC_HS_BUS_SPEED\n");
		itapdly[MMC_HS_BUS_SPEED] = SDHCI_ITAPDLYSEL_MMC_HSD;
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-mmc-hsd",
			   &otapdly[MMC_HS_BUS_SPEED]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for MMC_HS_BUS_SPEED\n");
		otapdly[MMC_HS_BUS_SPEED] = SDHCI_OTAPDLYSEL_MMC_HSD;
	}

	ret = dev_read_u32(dev, "xlnx,itap-delay-mmc-ddr52",
			   &itapdly[MMC_DDR52_BUS_SPEED]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for MMC_DDR52_BUS_SPEED\n");
		itapdly[MMC_DDR52_BUS_SPEED] = SDHCI_ITAPDLYSEL_MMC_DDR52;
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-mmc-ddr52",
			   &otapdly[MMC_DDR52_BUS_SPEED]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for MMC_DDR52_BUS_SPEED\n");
		otapdly[MMC_DDR52_BUS_SPEED] = SDHCI_OTAPDLYSEL_MMC_DDR52;
	}

	ret = dev_read_u32(dev, "xlnx,itap-delay-sdr104",
			   &itapdly[MMC_TIMING_UHS_SDR104]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for MMC_TIMING_UHS_SDR104\n");
		if (priv->bank == MMC_BANK2) {
			itapdly[MMC_TIMING_UHS_SDR104] =
				SDHCI_ITAPDLYSEL_SDR104_B2;
		} else {
			itapdly[MMC_TIMING_UHS_SDR104] =
				SDHCI_ITAPDLYSEL_SDR104_B0;
		}
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-sdr104",
			   &otapdly[MMC_TIMING_UHS_SDR104]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for MMC_TIMING_UHS_SDR104\n");
		if (priv->bank == MMC_BANK2) {
			otapdly[MMC_TIMING_UHS_SDR104] =
				SDHCI_OTAPDLYSEL_SDR104_B2;
		} else {
			otapdly[MMC_TIMING_UHS_SDR104] =
				SDHCI_OTAPDLYSEL_SDR104_B0;
		}
	}

	ret = dev_read_u32(dev, "xlnx,itap-delay-mmc-hs200",
			   &itapdly[MMC_TIMING_HS200]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined itapdly for MMC_TIMING_HS200\n");
		if (priv->bank == MMC_BANK2) {
			itapdly[MMC_TIMING_HS200] =
				SDHCI_ITAPDLYSEL_MMC_HS200_B2;
		} else {
			itapdly[MMC_TIMING_HS200] =
				SDHCI_ITAPDLYSEL_MMC_HS200_B0;
		}
	}

	ret = dev_read_u32(dev, "xlnx,otap-delay-mmc-hs200",
			   &otapdly[MMC_TIMING_HS200]);
	if (ret) {
		dev_dbg(dev,
			"Using predefined otapdly for MMC_TIMING_HS200\n");
		if (priv->bank == MMC_BANK2) {
			otapdly[MMC_TIMING_HS200] =
				SDHCI_OTAPDLYSEL_MMC_HS200_B2;
		} else {
			otapdly[MMC_TIMING_HS200] =
				SDHCI_OTAPDLYSEL_MMC_HS200_B0;
		}
	}
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
	host->quirks |= SDHCI_QUIRK_BROKEN_HISPD_MODE;
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
	arasan_zynqmp_dt_parse_tap_delays(dev);
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
