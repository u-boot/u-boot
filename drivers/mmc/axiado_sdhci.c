// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021-26 Axiado Corporation (or its affiliates).
 */

#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include "mmc_private.h"
#include <dm/device.h>
#include <linux/err.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <generic-phy.h>
#include <dm/device_compat.h>
#include <linux/delay.h>

#define SDHCI_HOST_CTRL2		0x3E
#define SDHCI_CTRL_VDD_180		0x0008
#define SDHCI_CTRL_EXEC_TUNING		0x0040
#define SDHCI_TUNING_LOOP_COUNT		40
#define EMMC_MIN_FREQ			400000
#define EMMC_MAX_FREQ			200000000

DECLARE_GLOBAL_DATA_PTR;

struct axiado_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	unsigned int f_max;
	struct phy phy;
	bool phy_is_on;
};

struct axiado_sdhci_priv {
	struct sdhci_host *host;
};

static void axiado_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u32 reg;

	switch (mmc->signal_voltage) {
	case MMC_SIGNAL_VOLTAGE_180:
		reg = sdhci_readw(host, SDHCI_HOST_CTRL2);
		reg |= SDHCI_CTRL_VDD_180;  /* Set 1.8V mode */
		sdhci_writew(host, reg, SDHCI_HOST_CTRL2);
		break;

	case MMC_SIGNAL_VOLTAGE_330:
		reg = sdhci_readw(host, SDHCI_HOST_CTRL2);
		reg &= ~SDHCI_CTRL_VDD_180; /* Clear 1.8V mode (3.3V) */
		sdhci_writew(host, reg, SDHCI_HOST_CTRL2);
		break;

	default:
		debug("axiado_sdhci: Unsupported voltage: %d\n",
		      mmc->signal_voltage);
		break;
	}

	sdhci_set_uhs_timing(host);
}

static int axiado_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct udevice *dev = host->mmc->dev;
	struct axiado_sdhci_plat *plat = dev_get_plat(dev);
	unsigned int speed = host->mmc->clock;
	u16 clk;

	/* Reset SD Clock Enable */
	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Power off PHY */
	if (plat->phy_is_on) {
		debug("Powering off eMMC PHY\n");
		generic_phy_power_off(&plat->phy);
		plat->phy_is_on = false;
	}

	/* Restart SD Clock */
	sdhci_set_clock(host->mmc, speed);

	/* Power on PHY if required */
	if (speed > EMMC_MIN_FREQ) {
		debug("Speed is high enough, powering on PHY...\n");
		generic_phy_power_on(&plat->phy);
		plat->phy_is_on = true;
	}

	return 0;
}

static int axiado_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	u32 ctrl;
	struct sdhci_host *host;
	char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;

	debug("%s\n", __func__);

	host = mmc->priv;

	ctrl = sdhci_readw(host, SDHCI_HOST_CTRL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CTRL2);

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
		sdhci_writew(host, ctrl, SDHCI_HOST_CTRL2);
	}

	if (!(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		printf("%s: Tuning failed\n", __func__);
		return -1;
	}

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return 0;
}

static struct sdhci_ops axiado_ops = {
	.set_ios_post = &axiado_sdhci_set_ios_post,
	.platform_execute_tuning = &axiado_sdhci_execute_tuning,
	.set_control_reg = &axiado_sdhci_set_control_reg,
};

static int axiado_sdhci_probe(struct udevice *dev)
{
	struct axiado_sdhci_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct axiado_sdhci_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host;
	unsigned long clock = 0;
	int ret;
	struct clk clk;

	host = priv->host;
	/* Get clock frequency */
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

	ret = clk_enable(&clk);
	/* U-Boot commonly uses ENOSYS for "function not implemented" */
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	if (device_is_compatible(dev, "axiado,ax3000-sdhci")) {
		/* Get and init phy */
		ret = generic_phy_get_by_name(dev, "phy_arasan", &plat->phy);
		if (ret) {
			dev_err(dev, "can't get phy from DT\n");
			return ret;
		}

		ret = generic_phy_init(&plat->phy);
		if (ret) {
			dev_err(dev, "couldn't initialize MMC PHY\n");
			return ret;
		}

		plat->phy_is_on = false;
	}

	host->ops = &axiado_ops;
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;

	host->max_clk = clock;
	host->bus_width = dev_read_u32_default(dev, "bus-width", 4);

	if (host->bus_width == 8)
		host->host_caps |= MMC_MODE_8BIT;
	else if (host->bus_width == 4)
		host->host_caps |= MMC_MODE_4BIT;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;

	ret = sdhci_setup_cfg(&plat->cfg, host, plat->f_max, EMMC_MIN_FREQ);
	if (ret)
		goto err;
	upriv->mmc = host->mmc;

	ret = sdhci_probe(dev);
	if (ret)
		goto err;

	return 0;

err:
#if !defined(CONFIG_TARGET_AX3000_EVK)
	clk_disable(&clk);
#endif
	return ret;
}

static int axiado_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct axiado_sdhci_plat *plat = dev_get_plat(dev);
	struct axiado_sdhci_priv *priv = dev_get_priv(dev);
	priv->host = calloc(1, sizeof(struct sdhci_host));
	if (!priv->host) {
		dev_err(dev, "sdhci_host malloc failed\n");
		return -ENOMEM;
	}

	priv->host->name = dev->name;

	priv->host->ioaddr = dev_read_addr_ptr(dev);
	if (!priv->host->ioaddr) {
		dev_err(dev, "failed to get I/O address\n");
		return -EINVAL;
	}

	plat->f_max = dev_read_u32_default(dev, "mmc-clk", EMMC_MAX_FREQ);

	return 0;
}

static int axiado_sdhci_bind(struct udevice *dev)
{
	struct axiado_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id axiado_sdhci_ids[] = {
	{ .compatible = "axiado,ax3000-sdhci" },
	{ }
};

static int axiado_sdhci_remove(struct udevice *dev)
{
	struct axiado_sdhci_plat *plat = dev_get_plat(dev);

	if (device_is_compatible(dev, "axiado,ax3000-sdhci")) {
		if (plat->phy_is_on)
			generic_phy_power_off(&plat->phy);
		generic_phy_exit(&plat->phy);
	}

	return 0;
}

U_BOOT_DRIVER(axiado_sdhci_drv) = {
	.name = "axiado_sdhci",
	.id = UCLASS_MMC,
	.of_match = axiado_sdhci_ids,
	.of_to_plat = axiado_sdhci_ofdata_to_platdata,
	.ops = &sdhci_ops,
	.bind = axiado_sdhci_bind,
	.probe = axiado_sdhci_probe,
	.remove = axiado_sdhci_remove,
	.priv_auto = sizeof(struct axiado_sdhci_priv),
	.plat_auto = sizeof(struct axiado_sdhci_plat),
};
