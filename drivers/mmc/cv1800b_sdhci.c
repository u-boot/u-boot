// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <dm.h>
#include <mmc.h>
#include <sdhci.h>
#include <linux/delay.h>

#define SDHCI_PHY_TX_RX_DLY  0x240
#define MMC_MAX_CLOCK        375000000
#define TUNE_MAX_PHCODE      128

#define PHY_TX_SRC_INVERT  BIT(8)

struct cv1800b_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static void cv1800b_set_tap_delay(struct sdhci_host *host, u16 tap)
{
	sdhci_writel(host, PHY_TX_SRC_INVERT | tap << 16, SDHCI_PHY_TX_RX_DLY);
}

static void cv1800b_sdhci_reset(struct sdhci_host *host, u8 mask)
{
	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask)
		udelay(10);
}

#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
static int cv1800b_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct sdhci_host *host = dev_get_priv(mmc->dev);

	u16 tap;

	int current_size = 0;
	int max_size = 0;
	int max_window = 0;

	for (tap = 0; tap < TUNE_MAX_PHCODE; tap++) {
		cv1800b_set_tap_delay(host, tap);

		if (mmc_send_tuning(host->mmc, opcode)) {
			current_size = 0;
		} else {
			current_size++;
			if (current_size > max_size) {
				max_size = current_size;
				max_window = tap;
			}
		}
	}

	cv1800b_sdhci_reset(host, SDHCI_RESET_CMD | SDHCI_RESET_DATA);

	cv1800b_set_tap_delay(host, max_window - max_size / 2);

	return 0;
}
#endif

const struct sdhci_ops cv1800b_sdhci_sd_ops = {
#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
	.platform_execute_tuning = cv1800b_execute_tuning,
#endif
};

static int cv1800b_sdhci_bind(struct udevice *dev)
{
	struct cv1800b_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static int cv1800b_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct cv1800b_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	host->name = dev->name;
	host->ioaddr = devfdt_get_addr_ptr(dev);

	upriv->mmc = &plat->mmc;
	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	host->ops = &cv1800b_sdhci_sd_ops;
	host->max_clk = MMC_MAX_CLOCK;

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 200000);
	if (ret)
		return ret;

	return sdhci_probe(dev);
}

static const struct udevice_id cv1800b_sdhci_match[] = {
	{ .compatible = "sophgo,cv1800b-dwcmshc" },
	{ }
};

U_BOOT_DRIVER(cv1800b_sdhci) = {
	.name = "sdhci-cv1800b",
	.id = UCLASS_MMC,
	.of_match = cv1800b_sdhci_match,
	.bind = cv1800b_sdhci_bind,
	.probe = cv1800b_sdhci_probe,
	.priv_auto = sizeof(struct sdhci_host),
	.plat_auto = sizeof(struct cv1800b_sdhci_plat),
	.ops = &sdhci_ops,
};
