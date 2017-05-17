/*
 *  Copyright (c) 2017
 *  Patrice Chotard <patrice.chotard@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <sdhci.h>
#include <asm/arch/sdhci.h>

DECLARE_GLOBAL_DATA_PTR;

struct sti_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

/*
 * used to get access to MMC1 reset,
 * will be removed when STi reset driver will be available
 */
#define STIH410_SYSCONF5_BASE		0x092b0000

/**
 * sti_mmc_core_config: configure the Arasan HC
 * @regbase: base address
 * @mmc_instance: mmc instance id
 * Description: this function is to configure the Arasan MMC HC.
 * This should be called when the system starts in case of, on the SoC,
 * it is needed to configure the host controller.
 * This happens on some SoCs, i.e. StiH410, where the MMC0 inside the flashSS
 * needs to be configured as MMC 4.5 to have full capabilities.
 * W/o these settings the SDHCI could configure and use the embedded controller
 * with limited features.
 */
static void sti_mmc_core_config(const u32 regbase, int mmc_instance)
{
	unsigned long *sysconf;

	/* only MMC1 has a reset line */
	if (mmc_instance) {
		sysconf = (unsigned long *)(STIH410_SYSCONF5_BASE +
			  ST_MMC_CCONFIG_REG_5);
		generic_set_bit(SYSCONF_MMC1_ENABLE_BIT, sysconf);
	}

	writel(STI_FLASHSS_MMC_CORE_CONFIG_1,
	       regbase + FLASHSS_MMC_CORE_CONFIG_1);

	if (mmc_instance) {
		writel(STI_FLASHSS_MMC_CORE_CONFIG2,
		       regbase + FLASHSS_MMC_CORE_CONFIG_2);
		writel(STI_FLASHSS_MMC_CORE_CONFIG3,
		       regbase + FLASHSS_MMC_CORE_CONFIG_3);
	} else {
		writel(STI_FLASHSS_SDCARD_CORE_CONFIG2,
		       regbase + FLASHSS_MMC_CORE_CONFIG_2);
		writel(STI_FLASHSS_SDCARD_CORE_CONFIG3,
		       regbase + FLASHSS_MMC_CORE_CONFIG_3);
	}
	writel(STI_FLASHSS_MMC_CORE_CONFIG4,
	       regbase + FLASHSS_MMC_CORE_CONFIG_4);
}

static int sti_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sti_sdhci_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret, mmc_instance;

	/*
	 * identify current mmc instance, mmc1 has a reset, not mmc0
	 * MMC0 is wired to the SD slot,
	 * MMC1 is wired on the high speed connector
	 */

	if (fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "resets", NULL))
		mmc_instance = 1;
	else
		mmc_instance = 0;

	sti_mmc_core_config((const u32) host->ioaddr, mmc_instance);

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_32BIT_DMA_ADDR |
		       SDHCI_QUIRK_NO_HISPD_BIT;

	host->host_caps = MMC_MODE_DDR_52MHz;

	ret = sdhci_setup_cfg(&plat->cfg, host, 50000000, 400000);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int sti_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);

	host->name = strdup(dev->name);
	host->ioaddr = (void *)devfdt_get_addr(dev);

	host->bus_width = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					 "bus-width", 4);

	return 0;
}

static int sti_sdhci_bind(struct udevice *dev)
{
	struct sti_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id sti_sdhci_ids[] = {
	{ .compatible = "st,sdhci" },
	{ }
};

U_BOOT_DRIVER(sti_mmc) = {
	.name = "sti_sdhci",
	.id = UCLASS_MMC,
	.of_match = sti_sdhci_ids,
	.bind = sti_sdhci_bind,
	.ops = &sdhci_ops,
	.ofdata_to_platdata = sti_sdhci_ofdata_to_platdata,
	.probe = sti_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct sti_sdhci_plat),
};
