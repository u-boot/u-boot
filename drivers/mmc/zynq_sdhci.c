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
#include "mmc_private.h"
#include <linux/libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <zynqmp_tap_delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define ZYNQMP_ITAP_DELAYS		{0x0, 0x15, 0x0, 0x0, 0x3D, 0x0,\
					 0x15, 0x12, 0x15}
#define ZYNQMP_OTAP_DELAYS		{0x0, 0x5, 0x3, 0x3, 0x4, 0x3,\
					 0x5, 0x6, 0x6}
#define VERSAL_ITAP_DELAYS		{0x0, 0x2C, 0x0, 0x0, 0x36, 0x0,\
					 0x2C, 0x1E, 0x2C}
#define VERSAL_OTAP_DELAYS		{0x0, 0x4, 0x3, 0x2, 0x3, 0x2,\
					 0x4, 0x5, 0x5}

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
};

#if defined(CONFIG_ARCH_ZYNQMP) || defined(CONFIG_ARCH_VERSAL)
#define MMC_HS200_BUS_SPEED	5

#define MMC_TIMING_UHS_SDR12    0
#define MMC_TIMING_UHS_SDR25    1
#define MMC_TIMING_UHS_SDR50    2
#define MMC_TIMING_UHS_SDR104   3
#define MMC_TIMING_UHS_DDR50    4
#define MMC_TIMING_HS200        5

static const u8 mode2timing[] = {
	[MMC_LEGACY] = UHS_SDR12_BUS_SPEED,
	[SD_LEGACY] = UHS_SDR12_BUS_SPEED,
	[MMC_HS] = HIGH_SPEED_BUS_SPEED,
	[SD_HS] = HIGH_SPEED_BUS_SPEED,
	[MMC_HS_52] = HIGH_SPEED_BUS_SPEED,
	[MMC_DDR_52] = HIGH_SPEED_BUS_SPEED,
	[UHS_SDR12] = UHS_SDR12_BUS_SPEED,
	[UHS_SDR25] = UHS_SDR25_BUS_SPEED,
	[UHS_SDR50] = UHS_SDR50_BUS_SPEED,
	[UHS_DDR50] = UHS_DDR50_BUS_SPEED,
	[UHS_SDR104] = UHS_SDR104_BUS_SPEED,
	[MMC_HS_200] = MMC_HS200_BUS_SPEED,
};

#define SDHCI_TUNING_LOOP_COUNT	40

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

#if defined(CONFIG_ARCH_VERSAL)
#define SDHCI_ARASAN_ITAPDLY_REGISTER   0xF0F8
#define SDHCI_ARASAN_OTAPDLY_REGISTER   0xF0FC

#define SDHCI_ITAPDLY_CHGWIN            0x200
#define SDHCI_ITAPDLY_ENABLE            0x100
#define SDHCI_OTAPDLY_ENABLE            0x40

static void arasan_set_tapdelay(struct sdhci_host *host, u32 itap_delay,
				u32 otap_delay)
{
	u32 regval;

	if (itap_delay) {
		regval = sdhci_readl(host, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= SDHCI_ITAPDLY_CHGWIN;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= SDHCI_ITAPDLY_ENABLE;
		regval = sdhci_readl(host, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= SDHCI_ITAPDLY_CHGWIN;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= SDHCI_ITAPDLY_ENABLE;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval |= itap_delay;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
		regval &= ~SDHCI_ITAPDLY_CHGWIN;
		sdhci_writel(host, regval, SDHCI_ARASAN_ITAPDLY_REGISTER);
	}

	if (otap_delay) {
		regval = sdhci_readl(host, SDHCI_ARASAN_OTAPDLY_REGISTER);
		regval |= SDHCI_OTAPDLY_ENABLE;
		sdhci_writel(host, regval, SDHCI_ARASAN_OTAPDLY_REGISTER);
		regval |= otap_delay;
		sdhci_writel(host, regval, SDHCI_ARASAN_OTAPDLY_REGISTER);
	}
}
#endif

static void arasan_sdhci_set_tapdelay(struct sdhci_host *host)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(host->mmc->dev);
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 uhsmode;
	u32 itap_delay;
	u32 otap_delay;

	uhsmode = mode2timing[mmc->selected_mode];

	debug("%s, host:%s devId:%d, bank:%d, mode:%d\n", __func__, host->name,
	      priv->deviceid, priv->bank, uhsmode);
	if ((uhsmode >= MMC_TIMING_UHS_SDR25) &&
	    (uhsmode <= MMC_TIMING_HS200)) {
		itap_delay = priv->itapdly[uhsmode];
		otap_delay = priv->otapdly[uhsmode];
#if defined(CONFIG_ARCH_ZYNQMP)
		arasan_zynqmp_set_tapdelay(priv->deviceid, itap_delay,
					   otap_delay);
#elif defined(CONFIG_ARCH_VERSAL)
		arasan_set_tapdelay(host, itap_delay, otap_delay);
#endif
	}
}

static void arasan_dt_read_tap_delay(struct udevice *dev, u32 *tapdly,
				     u8 mode, const char *prop)
{
	/*
	 * Read Tap Delay values from DT, if the DT does not contain the
	 * Tap Values then use the pre-defined values
	 */
	if (dev_read_u32(dev, prop, &tapdly[mode])) {
		dev_dbg(dev, "Using predefined tapdly for %s = %d\n",
			prop, tapdly[mode]);
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
	u32 *itapdly;
	u32 *otapdly;
	int i;

	if (ofnode_device_is_compatible(dev_ofnode(dev), "xlnx,zynqmp-8.9a")) {
		itapdly = (u32 [MMC_MAX_BUS_SPEED]) ZYNQMP_ITAP_DELAYS;
		otapdly = (u32 [MMC_MAX_BUS_SPEED]) ZYNQMP_OTAP_DELAYS;
	} else {
		itapdly = (u32 [MMC_MAX_BUS_SPEED]) VERSAL_ITAP_DELAYS;
		otapdly = (u32 [MMC_MAX_BUS_SPEED]) VERSAL_OTAP_DELAYS;
	}

	/* as of now bank2 tap delays are same for zynqmp and versal */
	if (priv->bank == MMC_BANK2) {
		itapdly[MMC_TIMING_UHS_SDR104] = 0x0;
		otapdly[MMC_TIMING_UHS_SDR104] = 0x2;
		itapdly[MMC_TIMING_HS200] = 0x0;
		otapdly[MMC_TIMING_HS200] = 0x2;
	}

	arasan_dt_read_tap_delay(dev, itapdly, SD_HS_BUS_SPEED,
				 "xlnx,itap-delay-sd-hsd");
	arasan_dt_read_tap_delay(dev, itapdly, MMC_TIMING_UHS_SDR25,
				 "xlnx,itap-delay-sdr25");
	arasan_dt_read_tap_delay(dev, itapdly, MMC_TIMING_UHS_SDR50,
				 "xlnx,itap-delay-sdr50");
	arasan_dt_read_tap_delay(dev, itapdly, MMC_TIMING_UHS_SDR104,
				 "xlnx,itap-delay-sdr104");
	arasan_dt_read_tap_delay(dev, itapdly, MMC_TIMING_UHS_DDR50,
				 "xlnx,itap-delay-sd-ddr50");
	arasan_dt_read_tap_delay(dev, itapdly, MMC_HS_BUS_SPEED,
				 "xlnx,itap-delay-mmc-hsd");
	arasan_dt_read_tap_delay(dev, itapdly, MMC_DDR52_BUS_SPEED,
				 "xlnx,itap-delay-mmc-ddr52");
	arasan_dt_read_tap_delay(dev, itapdly, MMC_TIMING_HS200,
				 "xlnx,itap-delay-mmc-hs200");
	arasan_dt_read_tap_delay(dev, otapdly, SD_HS_BUS_SPEED,
				 "xlnx,otap-delay-sd-hsd");
	arasan_dt_read_tap_delay(dev, otapdly, MMC_TIMING_UHS_SDR25,
				 "xlnx,otap-delay-sdr25");
	arasan_dt_read_tap_delay(dev, otapdly, MMC_TIMING_UHS_SDR50,
				 "xlnx,otap-delay-sdr50");
	arasan_dt_read_tap_delay(dev, otapdly, MMC_TIMING_UHS_SDR104,
				 "xlnx,otap-delay-sdr104");
	arasan_dt_read_tap_delay(dev, otapdly, MMC_TIMING_UHS_DDR50,
				 "xlnx,otap-delay-sd-ddr50");
	arasan_dt_read_tap_delay(dev, otapdly, MMC_HS_BUS_SPEED,
				 "xlnx,otap-delay-mmc-hsd");
	arasan_dt_read_tap_delay(dev, otapdly, MMC_DDR52_BUS_SPEED,
				 "xlnx,otap-delay-mmc-ddr52");
	arasan_dt_read_tap_delay(dev, otapdly, MMC_TIMING_HS200,
				 "xlnx,otap-delay-mmc-hs200");

	for (i = 0; i < MMC_MAX_BUS_SPEED; i++) {
		priv->itapdly[i] = itapdly[i];
		priv->otapdly[i] = otapdly[i];
	}
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
	    mmc->selected_mode <= MMC_HS_200) {
		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg &= ~SDHCI_CTRL_UHS_MASK;
		switch (mmc->selected_mode) {
		case UHS_SDR12:
			reg |= UHS_SDR12_BUS_SPEED;
			break;
		case UHS_SDR25:
			reg |= UHS_SDR25_BUS_SPEED;
			break;
		case UHS_SDR50:
			reg |= UHS_SDR50_BUS_SPEED;
			break;
		case UHS_SDR104:
		case MMC_HS_200:
			reg |= UHS_SDR104_BUS_SPEED;
			break;
		case UHS_DDR50:
		case MMC_DDR_52:
			reg |= UHS_DDR50_BUS_SPEED;
			break;
		default:
			break;
		}
		sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
	}
}

const struct sdhci_ops arasan_ops = {
	.platform_execute_tuning	= &arasan_sdhci_execute_tuning,
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

	host->max_clk = clock;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;

	ret = sdhci_setup_cfg(&plat->cfg, host, plat->f_max,
			      CONFIG_ZYNQ_SDHCI_MIN_FREQ);
	if (ret)
		return ret;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int arasan_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_platdata(dev);
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);

	priv->host = calloc(1, sizeof(struct sdhci_host));
	if (!priv->host)
		return -1;

	priv->host->name = dev->name;

#if defined(CONFIG_ARCH_ZYNQMP) || defined(CONFIG_ARCH_VERSAL)
	priv->host->ops = &arasan_ops;
	arasan_zynqmp_dt_parse_tap_delays(dev);
#endif

	priv->host->ioaddr = (void *)dev_read_addr(dev);
	if (IS_ERR(priv->host->ioaddr))
		return PTR_ERR(priv->host->ioaddr);

	priv->deviceid = dev_read_u32_default(dev, "xlnx,device_id", -1);
	priv->bank = dev_read_u32_default(dev, "xlnx,mio_bank", -1);
	priv->no_1p8 = dev_read_bool(dev, "no-1-8-v");
	plat->f_max = dev_read_u32_default(dev, "max-frequency",
					   CONFIG_ZYNQ_SDHCI_MAX_FREQ);
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
