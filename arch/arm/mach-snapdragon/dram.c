// SPDX-License-Identifier: GPL-2.0+
/*
 * Memory layout parsing for Qualcomm.
 */

#define LOG_CATEGORY LOGC_BOARD
#define pr_fmt(fmt) "QCOM-DRAM: " fmt

#include <asm-generic/unaligned.h>
#include <dm.h>
#include <log.h>
#include <sort.h>
#include <soc/qcom/smem.h>

#include "qcom-priv.h"
#include "rampart.h"

static struct {
	phys_addr_t start;
	phys_size_t size;
} prevbl_ddr_banks[CONFIG_NR_DRAM_BANKS] __section(".data") = { 0 };

int dram_init(void)
{
	/*
	 * gd->ram_base / ram_size have been setup already
	 * in qcom_parse_memory().
	 */
	return 0;
}

static int ddr_bank_cmp(const void *v1, const void *v2)
{
	const struct {
		phys_addr_t start;
		phys_size_t size;
	} *res1 = v1, *res2 = v2;

	if (!res1->size)
		return 1;
	if (!res2->size)
		return -1;

	return (res1->start >> 24) - (res2->start >> 24);
}

/* This has to be done post-relocation since gd->bd isn't preserved */
static void qcom_configure_bi_dram(void)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = prevbl_ddr_banks[i].start;
		gd->bd->bi_dram[i].size = prevbl_ddr_banks[i].size;
		debug("Bank[%d]: start = %#011llx, size = %#011llx\n",
		      i, gd->bd->bi_dram[i].start, gd->bd->bi_dram[i].size);
		if (!prevbl_ddr_banks[i].size)
			break;
	}
}

int dram_init_banksize(void)
{
	qcom_configure_bi_dram();

	return 0;
}

#define entry_field(v, e, field) (v == 0 ? ((struct ram_partition_entry_v0 *)(e))->field : \
				 (v == 1 ? ((struct ram_partition_entry_v1 *)(e))->field : \
				  ((struct ram_partition_entry_v3 *)(e))->field))
#define entry_start(v, e) entry_field(v, e, start_address)
#define entry_length(v, e) entry_field(v, e, length)
#define entry_category(v, e) entry_field(v, e, partition_category)
#define entry_domain(v, e) entry_field(v, e, partition_domain)
#define entry_type(v, e) entry_field(v, e, partition_type)

/* Parse memory map from SMEM, return the number of entries */
static int qcom_parse_memory_smem(phys_addr_t *ram_end)
{
	size_t size;
	int i, j = 0, ret;
	struct usable_ram_partition_table_header *header;
	u32 ver;
	void *entry;
	u32 entry_size; // Size of each RAM partition entry (version dependent)

	ret = qcom_smem_init();
	if (ret) {
		debug("Failed to initialize SMEM: %d.\n", ret);
		return ret;
	}

	header = qcom_smem_get(QCOM_SMEM_HOST_ANY, SMEM_USABLE_RAM_PARTITION_TABLE, &size);
	if (!header) {
		debug("Failed to find SMEM partition.\n");
		return -ENODEV;
	}

	ver = header->version;
	debug("SMEM RAM partition table version %u. %u entries\n", ver, header->num_partitions);

	switch (ver) {
	case 0:
		entry_size = sizeof(struct ram_partition_entry_v0);
		entry = ((struct usable_ram_partition_table_v0 *)header)->entries;
		break;
	case 1:
		entry_size = sizeof(struct ram_partition_entry_v1);
		entry = ((struct usable_ram_partition_table_v1 *)header)->entries;
		break;
	default:
		pr_warn("Unknown SMEM ram partition table version!\n");
	case 2:
	case 3:
		entry_size = sizeof(struct ram_partition_entry_v3);
		entry = ((struct usable_ram_partition_table_v3 *)header)->entries;
		break;
	}

	debug("SMEM RAM partition entry size: %u bytes\n", entry_size);

	/* Check validy of RAM */
	for (i = 0; i < header->num_partitions && j < CONFIG_NR_DRAM_BANKS; i++, entry += entry_size) {
		debug("Entry %d: [%#010llx - %#010llx]\n", i, entry_start(ver, entry),
		      (u64)entry_start(ver, entry) + entry_length(ver, entry));
		debug("          cat: %#04x type: %#04x domain: %#04x\n", entry_category(ver, entry),
		      entry_type(ver, entry), entry_domain(ver, entry));


		if (entry_category(ver, entry) != RAM_PARTITION_SDRAM ||
		    entry_type(ver, entry) != RAM_PARTITION_SYS_MEMORY)
			continue;
		if (!entry_length(ver, entry) && !entry_start(ver, entry))
			break;

		prevbl_ddr_banks[j].start = entry_start(ver, entry);
		prevbl_ddr_banks[j].size = entry_length(ver, entry);
		*ram_end = max(*ram_end, prevbl_ddr_banks[j].start + prevbl_ddr_banks[j].size);
		j++;
	}

	if (j == CONFIG_NR_DRAM_BANKS)
		pr_err("SMEM: More than CONFIG_NR_DRAM_BANKS (%u) entries!", CONFIG_NR_DRAM_BANKS);

	return j;
}

static void qcom_parse_memory_dt(const fdt64_t *fdt, int *banks, phys_addr_t *ram_end)
{
	int offset;
	const fdt64_t *memory;
	int memsize;
	int i, j;

	*ram_end = 0;

	offset = fdt_path_offset(fdt, "/memory");
	if (offset < 0)
		return;

	memory = fdt_getprop(fdt, offset, "reg", &memsize);
	if (!memory)
		return;

	*banks = min(memsize / (2 * sizeof(u64)), (ulong)CONFIG_NR_DRAM_BANKS);

	if (memsize / sizeof(u64) > CONFIG_NR_DRAM_BANKS * 2)
		log_err("Provided more than the max of %d memory banks\n", CONFIG_NR_DRAM_BANKS);

	if (*banks > CONFIG_NR_DRAM_BANKS)
		log_err("Provided more memory banks than we can handle\n");

	for (i = 0, j = 0; i < *banks * 2; i += 2, j++) {
		prevbl_ddr_banks[j].start = get_unaligned_be64(&memory[i]);
		prevbl_ddr_banks[j].size = get_unaligned_be64(&memory[i + 1]);
		if (!prevbl_ddr_banks[j].size) {
			j--;
			continue;
		}
		*ram_end = max(*ram_end, prevbl_ddr_banks[j].start + prevbl_ddr_banks[j].size);
	}
}

/**
 * The generic memory parsing code in U-Boot lacks a few things that we
 * need on Qualcomm:
 *
 * 1. It sets gd->ram_size and gd->ram_base to represent a single memory block
 * 2. setup_dest_addr() later relocates U-Boot to ram_base + ram_size, the end
 *    of that first memory block.
 *
 * This results in all memory beyond U-Boot being unusable in Linux when booting
 * with EFI.
 *
 * Since the ranges in the memory node may be out of order, the only way for us
 * to correctly determine the relocation address for U-Boot is to parse all
 * memory regions and find the highest valid address.
 *
 * We can't use fdtdec_setup_memory_banksize() since it stores the result in
 * gd->bd, which is not yet allocated.
 *
 * @fdt: FDT blob to parse /memory node from
 * @fdt_is_internal: is the FDT one embedded into U-Boot or was it provided by a prior
 *		     bootloader stage? This determined if we should try to rely on SMEM
 *		     (internal FDT) or error. We currently assume that if we are passed
 *		     an external FDT then it already has the memory map populated.
 *
 * Return: 0 on success or -ENODATA if /memory node is missing or incomplete
 */
int qcom_parse_memory(const void *fdt, bool fdt_is_internal)
{
	phys_addr_t ram_end = 0;
	int banks;

	qcom_memmap_source = fdt_is_internal ? QCOM_MEMMAP_SOURCE_INTERNAL_FDT
					     : QCOM_MEMMAP_SOURCE_EXTERNAL_FDT;

	qcom_parse_memory_dt(fdt, &banks, &ram_end);

	/*
	 * If using an internal FDT but the memory node is empty
	 * then fall back to SMEM.
	 */
	if (!prevbl_ddr_banks[0].size && fdt_is_internal) {
		banks = qcom_parse_memory_smem(&ram_end);
		if (banks < 0)
			panic("Couldn't find a valid memory map!\n");
		qcom_memmap_source = QCOM_MEMMAP_SOURCE_SMEM;
	}

	/* Sort our RAM banks -_- */
	qsort(prevbl_ddr_banks, banks, sizeof(prevbl_ddr_banks[0]), ddr_bank_cmp);

	gd->ram_base = prevbl_ddr_banks[0].start;
	gd->ram_size = ram_end - gd->ram_base;

	return 0;
}
