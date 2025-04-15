// SPDX-License-Identifier: GPL-2.0+
/*
 * Synopsys DesignWare Multimedia Card Interface driver
 * extensions used in various Synopsys ARC devboards.
 *
 * Copyright (C) 2019 Synopsys
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 */

#include <clk.h>
#include <dm.h>
#include <dwmmc.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/libfdt.h>
#include <linux/err.h>
#include <malloc.h>

#define CLOCK_MIN		400000	/*  400 kHz */
#define FIFO_MIN		8
#define FIFO_MAX		4096

struct snps_dwmci_plat {
	struct mmc_config	cfg;
	struct mmc		mmc;
};

struct snps_dwmci_priv_data {
	struct dwmci_host	host;
	u32			f_max;
	struct gpio_desc	cd_gpio;
};

static int snps_dwmmc_clk_setup(struct udevice *dev)
{
	struct snps_dwmci_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;

	struct clk clk_ciu, clk_biu;
	int ret;

	ret = clk_get_by_name(dev, "ciu", &clk_ciu);
	if (ret)
		goto clk_err;

	ret = clk_enable(&clk_ciu);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP)
		goto clk_err;

	host->bus_hz = clk_get_rate(&clk_ciu);
	if (host->bus_hz < CLOCK_MIN) {
		ret = -EINVAL;
		goto clk_err_ciu_dis;
	}

	ret = clk_get_by_name(dev, "biu", &clk_biu);
	if (ret)
		goto clk_err_ciu_dis;

	ret = clk_enable(&clk_biu);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP)
		goto clk_err_ciu_dis;

	return 0;

clk_err_ciu_dis:
	clk_disable(&clk_ciu);
clk_err:
	dev_err(dev, "failed to setup clocks, ret %d\n", ret);

	return ret;
}

static int snps_dwmmc_of_to_plat(struct udevice *dev)
{
	struct snps_dwmci_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	u32 fifo_depth;
	int ret;

	host->ioaddr = dev_read_addr_ptr(dev);

	/*
	 * If fifo-depth is unset don't set fifo_depth - we will try to
	 * auto detect it.
	 */
	ret = dev_read_u32(dev, "fifo-depth", &fifo_depth);
	if (!ret) {
		if (fifo_depth < FIFO_MIN || fifo_depth > FIFO_MAX)
			return -EINVAL;

		host->fifo_depth = fifo_depth;
	}

	host->buswidth = dev_read_u32_default(dev, "bus-width", 4);
	if (host->buswidth != 1 && host->buswidth != 4 && host->buswidth != 8)
		return -EINVAL;

	/*
	 * If max-frequency is unset don't set priv->f_max - we will use
	 * host->bus_hz in probe() instead.
	 */
	ret = dev_read_u32(dev, "max-frequency", &priv->f_max);
	if (!ret && priv->f_max < CLOCK_MIN)
		return -EINVAL;

	if (CONFIG_IS_ENABLED(DM_GPIO))
		gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio,
				     GPIOD_IS_IN);

	host->fifo_mode = dev_read_bool(dev, "fifo-mode");
	host->name = dev->name;
	host->dev_index = 0;
	host->priv = priv;

	return 0;
}

int snps_dwmmc_getcd(struct udevice *dev)
{
	struct snps_dwmci_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;

	if (CONFIG_IS_ENABLED(DM_GPIO) && dm_gpio_is_valid(&priv->cd_gpio))
		return dm_gpio_get_value(&priv->cd_gpio);

	return !(dwmci_readl(host, DWMCI_CDETECT) & 1);
}

struct dm_mmc_ops snps_dwmci_dm_ops;

static int snps_dwmmc_probe(struct udevice *dev)
{
#ifdef CONFIG_BLK
	struct snps_dwmci_plat *plat = dev_get_plat(dev);
#endif
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct snps_dwmci_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	unsigned int clock_max;
	int ret;

	/* Extend generic 'dm_dwmci_ops' with our 'getcd' implementation */
	memcpy(&snps_dwmci_dm_ops, &dm_dwmci_ops, sizeof(struct dm_mmc_ops));
	snps_dwmci_dm_ops.get_cd = snps_dwmmc_getcd;

	ret = snps_dwmmc_clk_setup(dev);
	if (ret)
		return ret;

	if (!priv->f_max)
		clock_max = host->bus_hz;
	else
		clock_max = min_t(unsigned int, host->bus_hz, priv->f_max);

#ifdef CONFIG_BLK
	dwmci_setup_cfg(&plat->cfg, host, clock_max, CLOCK_MIN);
	host->mmc = &plat->mmc;
#else
	ret = add_dwmci(host, clock_max, CLOCK_MIN);
	if (ret)
		return ret;
#endif
	host->mmc->priv = &priv->host;
	upriv->mmc = host->mmc;
	host->mmc->dev = dev;

	return dwmci_probe(dev);
}

static int snps_dwmmc_bind(struct udevice *dev)
{
#ifdef CONFIG_BLK
	struct snps_dwmci_plat *plat = dev_get_plat(dev);
	int ret;

	ret = dwmci_bind(dev, &plat->mmc, &plat->cfg);
	if (ret)
		return ret;
#endif

	return 0;
}

static const struct udevice_id snps_dwmmc_ids[] = {
	{ .compatible = "snps,dw-mshc" },
	{ .compatible = "starfive,jh7110-mmc" },
	{ }
};

U_BOOT_DRIVER(snps_dwmmc_drv) = {
	.name				= "snps_dw_mmc",
	.id				= UCLASS_MMC,
	.of_match			= snps_dwmmc_ids,
	.of_to_plat		= snps_dwmmc_of_to_plat,
	.ops				= &snps_dwmci_dm_ops,
	.bind				= snps_dwmmc_bind,
	.probe				= snps_dwmmc_probe,
	.priv_auto		= sizeof(struct snps_dwmci_priv_data),
	.plat_auto	= sizeof(struct snps_dwmci_plat),
};
