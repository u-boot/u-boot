// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <linux/libfdt.h>
#include <mmc.h>
#include <sdhci.h>
#include "sdhci-cadence.h"

struct sdhci_cdns_phy_cfg {
	const char *property;
	u8 addr;
};

static const struct sdhci_cdns_phy_cfg sdhci_cdns_phy_cfgs[] = {
	{ "cdns,phy-input-delay-sd-highspeed", SDHCI_CDNS_PHY_DLY_SD_HS, },
	{ "cdns,phy-input-delay-legacy", SDHCI_CDNS_PHY_DLY_SD_DEFAULT, },
	{ "cdns,phy-input-delay-sd-uhs-sdr12", SDHCI_CDNS_PHY_DLY_UHS_SDR12, },
	{ "cdns,phy-input-delay-sd-uhs-sdr25", SDHCI_CDNS_PHY_DLY_UHS_SDR25, },
	{ "cdns,phy-input-delay-sd-uhs-sdr50", SDHCI_CDNS_PHY_DLY_UHS_SDR50, },
	{ "cdns,phy-input-delay-sd-uhs-ddr50", SDHCI_CDNS_PHY_DLY_UHS_DDR50, },
	{ "cdns,phy-input-delay-mmc-highspeed", SDHCI_CDNS_PHY_DLY_EMMC_SDR, },
	{ "cdns,phy-input-delay-mmc-ddr", SDHCI_CDNS_PHY_DLY_EMMC_DDR, },
	{ "cdns,phy-dll-delay-sdclk", SDHCI_CDNS_PHY_DLY_SDCLK, },
	{ "cdns,phy-dll-delay-sdclk-hsmmc", SDHCI_CDNS_PHY_DLY_HSMMC, },
	{ "cdns,phy-dll-delay-strobe", SDHCI_CDNS_PHY_DLY_STROBE, },
};

static int sdhci_cdns_write_phy_reg(struct sdhci_cdns_plat *plat,
				    u8 addr, u8 data)
{
	void __iomem *reg = plat->hrs_addr + SDHCI_CDNS_HRS04;
	u32 tmp;
	int ret;

	tmp = FIELD_PREP(SDHCI_CDNS_HRS04_WDATA, data) |
	      FIELD_PREP(SDHCI_CDNS_HRS04_ADDR, addr);
	writel(tmp, reg);

	tmp |= SDHCI_CDNS_HRS04_WR;
	writel(tmp, reg);

	ret = readl_poll_timeout(reg, tmp, tmp & SDHCI_CDNS_HRS04_ACK, 10);
	if (ret)
		return ret;

	tmp &= ~SDHCI_CDNS_HRS04_WR;
	writel(tmp, reg);

	return 0;
}

static int sdhci_cdns_phy_init(struct sdhci_cdns_plat *plat,
				const void *fdt, int nodeoffset)
{
	const fdt32_t *prop;
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(sdhci_cdns_phy_cfgs); i++) {
		prop = fdt_getprop(fdt, nodeoffset,
				   sdhci_cdns_phy_cfgs[i].property, NULL);
		if (!prop)
			continue;

		ret = sdhci_cdns_write_phy_reg(plat,
					       sdhci_cdns_phy_cfgs[i].addr,
					       fdt32_to_cpu(*prop));
		if (ret)
			return ret;
	}

	return 0;
}

static void sdhci_cdns_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	struct sdhci_cdns_plat *plat = dev_get_plat(mmc->dev);
	unsigned int clock = mmc->clock;
	u32 mode, tmp;

	/*
	 * REVISIT:
	 * The mode should be decided by MMC_TIMING_* like Linux, but
	 * U-Boot does not support timing.  Use the clock frequency instead.
	 */
	if (clock <= 26000000) {
		mode = SDHCI_CDNS_HRS06_MODE_SD; /* use this for Legacy */
	} else if (clock <= 52000000) {
		if (mmc->ddr_mode)
			mode = SDHCI_CDNS_HRS06_MODE_MMC_DDR;
		else
			mode = SDHCI_CDNS_HRS06_MODE_MMC_SDR;
	} else {
		if (mmc->ddr_mode)
			mode = SDHCI_CDNS_HRS06_MODE_MMC_HS400;
		else
			mode = SDHCI_CDNS_HRS06_MODE_MMC_HS200;
	}

	tmp = readl(plat->hrs_addr + SDHCI_CDNS_HRS06);
	tmp &= ~SDHCI_CDNS_HRS06_MODE;
	tmp |= FIELD_PREP(SDHCI_CDNS_HRS06_MODE, mode);
	writel(tmp, plat->hrs_addr + SDHCI_CDNS_HRS06);

	if (device_is_compatible(mmc->dev, "cdns,sd6hc"))
		sdhci_cdns6_phy_adj(mmc->dev, plat, mode);
}

static const struct sdhci_ops sdhci_cdns_ops = {
	.set_control_reg = sdhci_cdns_set_control_reg,
};

static int sdhci_cdns_set_tune_val(struct sdhci_cdns_plat *plat,
				   unsigned int val)
{
	void __iomem *reg = plat->hrs_addr + SDHCI_CDNS_HRS06;
	u32 tmp;
	int i, ret;

	if (device_is_compatible(plat->mmc.dev, "cdns,sd6hc"))
		return sdhci_cdns6_set_tune_val(plat, val);

	if (WARN_ON(!FIELD_FIT(SDHCI_CDNS_HRS06_TUNE, val)))
		return -EINVAL;

	tmp = readl(reg);
	tmp &= ~SDHCI_CDNS_HRS06_TUNE;
	tmp |= FIELD_PREP(SDHCI_CDNS_HRS06_TUNE, val);

	/*
	 * Workaround for IP errata:
	 * The IP6116 SD/eMMC PHY design has a timing issue on receive data
	 * path. Send tune request twice.
	 */
	for (i = 0; i < 2; i++) {
		tmp |= SDHCI_CDNS_HRS06_TUNE_UP;
		writel(tmp, reg);

		ret = readl_poll_timeout(reg, tmp,
					 !(tmp & SDHCI_CDNS_HRS06_TUNE_UP), 1);
		if (ret)
			return ret;
	}

	return 0;
}

static int __maybe_unused sdhci_cdns_execute_tuning(struct udevice *dev,
						    unsigned int opcode)
{
	struct sdhci_cdns_plat *plat = dev_get_plat(dev);
	struct mmc *mmc = &plat->mmc;
	int cur_streak = 0;
	int max_streak = 0;
	int end_of_streak = 0;
	int i;

	/*
	 * This handler only implements the eMMC tuning that is specific to
	 * this controller.  The tuning for SD timing should be handled by the
	 * SDHCI core.
	 */
	if (!IS_MMC(mmc))
		return -ENOTSUPP;

	if (WARN_ON(opcode != MMC_CMD_SEND_TUNING_BLOCK_HS200))
		return -EINVAL;

	for (i = 0; i < SDHCI_CDNS_MAX_TUNING_LOOP; i++) {
		if (sdhci_cdns_set_tune_val(plat, i) ||
		    mmc_send_tuning(mmc, opcode)) { /* bad */
			cur_streak = 0;
		} else { /* good */
			cur_streak++;
			if (cur_streak > max_streak) {
				max_streak = cur_streak;
				end_of_streak = i;
			}
		}
	}

	if (!max_streak) {
		dev_err(dev, "no tuning point found\n");
		return -EIO;
	}

	return sdhci_cdns_set_tune_val(plat, end_of_streak - max_streak / 2);
}

static struct dm_mmc_ops sdhci_cdns_mmc_ops;

static int sdhci_cdns_bind(struct udevice *dev)
{
	struct sdhci_cdns_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static int sdhci_cdns_probe(struct udevice *dev)
{
	DECLARE_GLOBAL_DATA_PTR;
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_cdns_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	fdt_addr_t base;
	int ret;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->hrs_addr = devm_ioremap(dev, base, SZ_1K);
	if (!plat->hrs_addr)
		return -ENOMEM;

	host->name = dev->name;
	host->ioaddr = plat->hrs_addr + SDHCI_CDNS_SRS_BASE;
	host->ops = &sdhci_cdns_ops;
	host->quirks |= SDHCI_QUIRK_WAIT_SEND_CMD;
	sdhci_cdns_mmc_ops = sdhci_ops;
#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
	sdhci_cdns_mmc_ops.execute_tuning = sdhci_cdns_execute_tuning;
#endif

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	if (device_is_compatible(dev, "cdns,sd6hc"))
		ret = sdhci_cdns6_phy_init(dev, plat);
	else
		ret = sdhci_cdns_phy_init(plat, gd->fdt_blob, dev_of_offset(dev));
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;

	upriv->mmc = &plat->mmc;
	host->mmc->priv = host;

	return sdhci_probe(dev);
}

static const struct udevice_id sdhci_cdns_match[] = {
	{ .compatible = "socionext,uniphier-sd4hc" },
	{ .compatible = "cdns,sd4hc" },
	{ .compatible = "cdns,sd6hc" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sdhci_cdns) = {
	.name = "sdhci-cdns",
	.id = UCLASS_MMC,
	.of_match = sdhci_cdns_match,
	.bind = sdhci_cdns_bind,
	.probe = sdhci_cdns_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct sdhci_cdns_plat),
	.ops = &sdhci_cdns_mmc_ops,
};
