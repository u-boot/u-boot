// SPDX-License-Identifier: GPL-2.0+
/*
 * SDHCI ADMA2 helper functions.
 */

#include <cpu_func.h>
#include <sdhci.h>
#include <malloc.h>
#include <asm/cache.h>

void sdhci_adma_write_desc(struct sdhci_host *host, void **next_desc,
			   dma_addr_t addr, int len, bool end)
{
	struct sdhci_adma_desc *desc = *next_desc;
	u8 attr;

	attr = ADMA_DESC_ATTR_VALID | ADMA_DESC_TRANSFER_DATA;
	if (end)
		attr |= ADMA_DESC_ATTR_END;

	desc->attr = attr;
	desc->len = len & 0xffff;
	desc->reserved = 0;
	desc->addr_lo = lower_32_bits(addr);
#ifdef CONFIG_MMC_SDHCI_ADMA_64BIT
	desc->addr_hi = upper_32_bits(addr);
#endif

	*next_desc += ADMA_DESC_LEN;
}

static inline void __sdhci_adma_write_desc(struct sdhci_host *host,
					   void **desc, dma_addr_t addr,
					   int len, bool end)
{
	if (host && host->ops && host->ops->adma_write_desc)
		host->ops->adma_write_desc(host, desc, addr, len, end);
	else
		sdhci_adma_write_desc(host, desc, addr, len, end);
}

/**
 * sdhci_prepare_adma_table() - Populate the ADMA table
 *
 * @host:	Pointer to the sdhci_host
 * @table:	Pointer to the ADMA table
 * @data:	Pointer to MMC data
 * @addr:	DMA address to write to or read from
 *
 * Fill the ADMA table according to the MMC data to read from or write to the
 * given DMA address.
 * Please note, that the table size depends on CONFIG_SYS_MMC_MAX_BLK_COUNT and
 * we don't have to check for overflow.
 */
void sdhci_prepare_adma_table(struct sdhci_host *host,
			      struct sdhci_adma_desc *table,
			      struct mmc_data *data, dma_addr_t start_addr)
{
	dma_addr_t addr = start_addr;
	uint trans_bytes = data->blocksize * data->blocks;
	void *next_desc = table;
	int i = DIV_ROUND_UP(trans_bytes, ADMA_MAX_LEN);

	while (--i) {
		__sdhci_adma_write_desc(host, &next_desc, addr,
					ADMA_MAX_LEN, false);
		addr += ADMA_MAX_LEN;
		trans_bytes -= ADMA_MAX_LEN;
	}

	__sdhci_adma_write_desc(host, &next_desc, addr, trans_bytes, true);

	flush_cache((phys_addr_t)table,
		    ROUND(next_desc - (void *)table,
			  ARCH_DMA_MINALIGN));
}

/**
 * sdhci_adma_init() - initialize the ADMA descriptor table
 *
 * Return: pointer to the allocated descriptor table or NULL in case of an
 * error.
 */
struct sdhci_adma_desc *sdhci_adma_init(void)
{
	return memalign(ARCH_DMA_MINALIGN, ADMA_TABLE_SZ);
}
