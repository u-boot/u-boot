/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <dm/device.h>
#include <mmc.h>
#include <sdhci.h>

/* HRS - Host Register Set (specific to Cadence) */
#define SDHCI_CDNS_HRS04		0x10		/* PHY access port */
#define   SDHCI_CDNS_HRS04_ACK			BIT(26)
#define   SDHCI_CDNS_HRS04_RD			BIT(25)
#define   SDHCI_CDNS_HRS04_WR			BIT(24)
#define   SDHCI_CDNS_HRS04_RDATA_SHIFT		12
#define   SDHCI_CDNS_HRS04_WDATA_SHIFT		8
#define   SDHCI_CDNS_HRS04_ADDR_SHIFT		0

/* SRS - Slot Register Set (SDHCI-compatible) */
#define SDHCI_CDNS_SRS_BASE		0x200

/* PHY */
#define SDHCI_CDNS_PHY_DLY_SD_HS	0x00
#define SDHCI_CDNS_PHY_DLY_SD_DEFAULT	0x01
#define SDHCI_CDNS_PHY_DLY_UHS_SDR12	0x02
#define SDHCI_CDNS_PHY_DLY_UHS_SDR25	0x03
#define SDHCI_CDNS_PHY_DLY_UHS_SDR50	0x04
#define SDHCI_CDNS_PHY_DLY_UHS_DDR50	0x05
#define SDHCI_CDNS_PHY_DLY_EMMC_LEGACY	0x06
#define SDHCI_CDNS_PHY_DLY_EMMC_SDR	0x07
#define SDHCI_CDNS_PHY_DLY_EMMC_DDR	0x08

struct sdhci_cdns_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	void __iomem *hrs_addr;
};

static void sdhci_cdns_write_phy_reg(struct sdhci_cdns_plat *plat,
				     u8 addr, u8 data)
{
	void __iomem *reg = plat->hrs_addr + SDHCI_CDNS_HRS04;
	u32 tmp;

	tmp = (data << SDHCI_CDNS_HRS04_WDATA_SHIFT) |
	      (addr << SDHCI_CDNS_HRS04_ADDR_SHIFT);
	writel(tmp, reg);

	tmp |= SDHCI_CDNS_HRS04_WR;
	writel(tmp, reg);

	tmp &= ~SDHCI_CDNS_HRS04_WR;
	writel(tmp, reg);
}

static void sdhci_cdns_phy_init(struct sdhci_cdns_plat *plat)
{
	sdhci_cdns_write_phy_reg(plat, SDHCI_CDNS_PHY_DLY_SD_HS, 4);
	sdhci_cdns_write_phy_reg(plat, SDHCI_CDNS_PHY_DLY_SD_DEFAULT, 4);
	sdhci_cdns_write_phy_reg(plat, SDHCI_CDNS_PHY_DLY_EMMC_LEGACY, 9);
	sdhci_cdns_write_phy_reg(plat, SDHCI_CDNS_PHY_DLY_EMMC_SDR, 2);
	sdhci_cdns_write_phy_reg(plat, SDHCI_CDNS_PHY_DLY_EMMC_DDR, 3);
}

static int sdhci_cdns_bind(struct udevice *dev)
{
	struct sdhci_cdns_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static int sdhci_cdns_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_cdns_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	fdt_addr_t base;
	int ret;

	base = dev_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->hrs_addr = devm_ioremap(dev, base, SZ_1K);
	if (!plat->hrs_addr)
		return -ENOMEM;

	host->name = dev->name;
	host->ioaddr = plat->hrs_addr + SDHCI_CDNS_SRS_BASE;
	host->quirks |= SDHCI_QUIRK_WAIT_SEND_CMD;

	sdhci_cdns_phy_init(plat);

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;

	upriv->mmc = &plat->mmc;
	host->mmc = &plat->mmc;
	host->mmc->priv = host;

	return sdhci_probe(dev);
}

static const struct udevice_id sdhci_cdns_match[] = {
	{ .compatible = "socionext,uniphier-sd4hc" },
	{ .compatible = "cdns,sd4hc" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sdhci_cdns) = {
	.name = "sdhci-cdns",
	.id = UCLASS_MMC,
	.of_match = sdhci_cdns_match,
	.bind = sdhci_cdns_bind,
	.probe = sdhci_cdns_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct sdhci_cdns_plat),
	.ops = &sdhci_ops,
};
