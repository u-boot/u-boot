// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell SD Host Controller Interface
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/mbus.h>

#define SDHCI_WINDOW_CTRL(win)		(0x4080 + ((win) << 4))
#define SDHCI_WINDOW_BASE(win)		(0x4084 + ((win) << 4))

static void sdhci_mvebu_mbus_config(void __iomem *base)
{
	const struct mbus_dram_target_info *dram;
	int i;

	dram = mvebu_mbus_dram_info();

	for (i = 0; i < 4; i++) {
		writel(0, base + SDHCI_WINDOW_CTRL(i));
		writel(0, base + SDHCI_WINDOW_BASE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		/* Write size, attributes and target id to control register */
		writel(((cs->size - 1) & 0xffff0000) | (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       base + SDHCI_WINDOW_CTRL(i));

		/* Write base address to base register */
		writel(cs->base, base + SDHCI_WINDOW_BASE(i));
	}
}

#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
static struct sdhci_ops mv_ops;

#if defined(CONFIG_SHEEVA_88SV331xV5)
#define SD_CE_ATA_2	0xEA
#define  MMC_CARD	0x1000
#define  MMC_WIDTH	0x0100
static inline void mv_sdhci_writeb(struct sdhci_host *host, u8 val, int reg)
{
	struct mmc *mmc = host->mmc;
	u32 ata = (unsigned long)host->ioaddr + SD_CE_ATA_2;

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
int mv_sdh_init(unsigned long regbase, u32 max_clk, u32 min_clk, u32 quirks)
{
	struct sdhci_host *host = NULL;
	host = calloc(1, sizeof(*host));
	if (!host) {
		printf("sdh_host malloc fail!\n");
		return -ENOMEM;
	}

	host->name = MVSDH_NAME;
	host->ioaddr = (void *)regbase;
	host->quirks = quirks;
	host->max_clk = max_clk;
#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
	memset(&mv_ops, 0, sizeof(struct sdhci_ops));
	mv_ops.write_b = mv_sdhci_writeb;
	host->ops = &mv_ops;
#endif

	if (CONFIG_IS_ENABLED(ARCH_MVEBU)) {
		/* Configure SDHCI MBUS mbus bridge windows */
		sdhci_mvebu_mbus_config((void __iomem *)regbase);
	}

	return add_sdhci(host, 0, min_clk);
}
