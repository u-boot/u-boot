#include <common.h>
#include <malloc.h>
#include <sdhci.h>

#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
static struct sdhci_ops mv_ops;

#if defined(CONFIG_SHEEVA_88SV331xV5)
#define SD_CE_ATA_2	0xEA
#define  MMC_CARD	0x1000
#define  MMC_WIDTH	0x0100
static inline void mv_sdhci_writeb(struct sdhci_host *host, u8 val, int reg)
{
	struct mmc *mmc = host->mmc;
	u32 ata = (u32)host->ioaddr + SD_CE_ATA_2;

	if (!IS_SD(mmc) && reg == SDHCI_HOST_CONTROL) {
		if (mmc->bus_width == 8)
			writew(readw(ata) | (MMC_CARD | MMC_WIDTH), ata);
		else
			writew(readw(ata) & ~(MMC_CARD | MMC_WIDTH), ata);
	}

	writeb(val, host->ioaddr + reg);
}

#else
#define mv_sdhci_writeb	NULL
#endif /* CONFIG_SHEEVA_88SV331xV5 */
#endif /* CONFIG_MMC_SDHCI_IO_ACCESSORS */

static char *MVSDH_NAME = "mv_sdh";
int mv_sdh_init(u32 regbase, u32 max_clk, u32 min_clk, u32 quirks)
{
	struct sdhci_host *host = NULL;
	host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
	if (!host) {
		printf("sdh_host malloc fail!\n");
		return 1;
	}

	host->name = MVSDH_NAME;
	host->ioaddr = (void *)regbase;
	host->quirks = quirks;
#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
	memset(&mv_ops, 0, sizeof(struct sdhci_ops));
	mv_ops.write_b = mv_sdhci_writeb;
	host->ops = &mv_ops;
#endif
	if (quirks & SDHCI_QUIRK_REG32_RW)
		host->version = sdhci_readl(host, SDHCI_HOST_VERSION - 2) >> 16;
	else
		host->version = sdhci_readw(host, SDHCI_HOST_VERSION);
	add_sdhci(host, max_clk, min_clk);
	return 0;
}
