// SPDX-License-Identifier: GPL-2.0+
/*
 * Common initialisation for Qualcomm Snapdragon boards.
 *
 * Copyright (c) 2024 Linaro Ltd.
 * Author: Casey Connolly <casey.connolly@linaro.org>
 */

#define LOG_CATEGORY LOGC_BOARD
#define pr_fmt(fmt) "QCOM: " fmt

#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <asm-generic/unaligned.h>
#include <cpu_func.h>
#include <fdt_support.h>
#include <linux/errno.h>
#include <linux/sizes.h>
#include <sort.h>
#include <time.h>

#include "qcom-priv.h"

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region rbx_mem_map[CONFIG_NR_DRAM_BANKS + 2] = { { 0 } };

struct mm_region *mem_map = rbx_mem_map;

static void build_mem_map(void)
{
	int i, j;

	/*
	 * Ensure the peripheral block is sized to correctly cover the address range
	 * up to the first memory bank.
	 * Don't map the first page to ensure that we actually trigger an abort on a
	 * null pointer access rather than just hanging.
	 * FIXME: we should probably split this into more precise regions
	 */
	mem_map[0].phys = 0x1000;
	mem_map[0].virt = mem_map[0].phys;
	mem_map[0].size = gd->bd->bi_dram[0].start - mem_map[0].phys;
	mem_map[0].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN;

	for (i = 1, j = 0; i < ARRAY_SIZE(rbx_mem_map) - 1 && gd->bd->bi_dram[j].size; i++, j++) {
		mem_map[i].phys = gd->bd->bi_dram[j].start;
		mem_map[i].virt = mem_map[i].phys;
		mem_map[i].size = gd->bd->bi_dram[j].size;
		mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | \
				   PTE_BLOCK_INNER_SHARE;
	}

	mem_map[i].phys = UINT64_MAX;
	mem_map[i].size = 0;

#ifdef DEBUG
	debug("Configured memory map:\n");
	for (i = 0; mem_map[i].size; i++)
		debug("  0x%016llx - 0x%016llx: entry %d\n",
		      mem_map[i].phys, mem_map[i].phys + mem_map[i].size, i);
#endif
}

u64 get_page_table_size(void)
{
	return SZ_1M;
}

struct mem_resource_attrs {
	fdt_addr_t start;
	fdt_addr_t size;
	u64 attrs;
};

static int fdt_cmp_res(const void *v1, const void *v2)
{
	const struct mem_resource_attrs *res1 = v1, *res2 = v2;

	return res1->start - res2->start;
}

#define N_RESERVED_REGIONS 64

/* Map and unmap reserved memory regions as appropriate.
 * Mark all no-map regions as PTE_TYPE_FAULT to prevent speculative access.
 * On some platforms this is enough to trigger a security violation and trap
 * to EL3.
 * Regions that may be accessed by drivers get mapped explicitly.
 */
static void configure_reserved_memory(void)
{
	static struct mem_resource_attrs res[N_RESERVED_REGIONS] = { 0 };
	int parent, rmem, count, i = 0;
	phys_addr_t start;
	size_t size;
	u64 attrs;

	/* Some reserved nodes must be carved out, as the cache-prefetcher may otherwise
	 * attempt to access them, causing a security exception.
	 */
	parent = fdt_path_offset(gd->fdt_blob, "/reserved-memory");
	if (parent <= 0) {
		log_err("No reserved memory regions found\n");
		return;
	}

	/* Collect the reserved memory regions and appropriate attrs */
	fdt_for_each_subnode(rmem, gd->fdt_blob, parent) {
		const fdt32_t *ptr;
		attrs = PTE_TYPE_FAULT;
		/* If the no-map property isn't set then the region is valid */
		if (!fdt_getprop(gd->fdt_blob, rmem, "no-map", NULL))
			attrs = PTE_TYPE_VALID | PTE_BLOCK_MEMTYPE(MT_NORMAL);
		/* If the compatible property is set then this region may be accessed by drivers and should
		 * be marked valid too. */
		if (fdt_getprop(gd->fdt_blob, rmem, "compatible", NULL))
			attrs = PTE_TYPE_VALID | PTE_BLOCK_MEMTYPE(MT_NORMAL);

		if (i == N_RESERVED_REGIONS) {
			log_err("Too many reserved regions!\n");
			break;
		}

		/* Read the address and size out from the reg property. Doing this "properly" with
		 * fdt_get_resource() takes ~70ms on SDM845, but open-coding the happy path here
		 * takes <1ms... Oh the woes of no dcache.
		 */
		ptr = fdt_getprop(gd->fdt_blob, rmem, "reg", NULL);
		if (ptr) {
			/* Qualcomm devices use #address/size-cells = <2> but all reserved regions are within
			 * the 32-bit address space. So we can cheat here for speed.
			 */
			res[i].start = fdt32_to_cpu(ptr[1]);
			res[i].size = fdt32_to_cpu(ptr[3]);
			res[i].attrs = attrs;
			i++;
		}
	}

	/* Sort the reserved memory regions by address */
	count = i;
	qsort(res, count, sizeof(res[0]), fdt_cmp_res);
	debug("Mapping %d regions!\n", count);

	/* Now set the right attributes for them. Often a lot of the regions are tightly packed together
	 * so we can optimise the number of calls to mmu_change_region_attr_nobreak() by combining adjacent
	 * regions.
	 */
	start = res[0].start;
	size = res[0].size;
	attrs = res[0].attrs;
	/* For each region after the first one, either increase the `size` to eventually be mapped or
	 * map the region we have and start a new one, this allows us to reduce the number of calls to
	 * mmu_map_region(). The loop is therefore "lagging" behind by one iteration. */
	for (i = 1; i <= count; i++) {
		/* If i == count we are done, just map the last region. If the last region is
		 * too far away or the attrs don't match then map the meta-region we have and
		 * start a new one. */
		if (i == count || start + size < res[i].start - SZ_8K || attrs != res[i].attrs) {
			debug("  0x%016llx - 0x%016llx: %s\n",
				start, start + size, attrs == PTE_TYPE_FAULT ? "FAULT" : "VALID");
			/* No need to break-before-make since dcache is disabled */
			mmu_change_region_attr_nobreak(start, size, attrs);
			/* We have now mapped all the regions */
			if (i == count)
				break;
			/* Start a new meta-region */
			start = res[i].start;
			size = res[i].size;
			attrs = res[i].attrs;
		} else {
			/* This region is next to (<8K) the previous one so combine them.
			 * Accounting for any small (<8K) gap. */
			size = (res[i].start - start) + res[i].size;
		}
	}
}

/* This function open-codes setup_all_pgtables() so that we can
 * insert additional mappings *before* turning on the MMU.
 */
void enable_caches(void)
{
	u64 tlb_addr = gd->arch.tlb_addr;
	u64 tlb_size = gd->arch.tlb_size;
	u64 pt_size;
	ulong carveout_start;

	gd->arch.tlb_fillptr = tlb_addr;

	build_mem_map();

	icache_enable();

	/* Create normal system page tables */
	setup_pgtables();

	pt_size = (uintptr_t)gd->arch.tlb_fillptr -
		  (uintptr_t)gd->arch.tlb_addr;
	debug("Primary pagetable size: %lluKiB\n", pt_size / 1024);

	/* Create emergency page tables */
	gd->arch.tlb_size -= pt_size;
	gd->arch.tlb_addr = gd->arch.tlb_fillptr;
	setup_pgtables();
	gd->arch.tlb_emerg = gd->arch.tlb_addr;
	gd->arch.tlb_addr = tlb_addr;
	gd->arch.tlb_size = tlb_size;

	/*
	 * On some boards speculative access may trigger a NOC or XPU violation so explicitly mark
	 * reserved regions as inacessible (PTE_TYPE_FAULT)
	 */
	if (qcom_memmap_source == QCOM_MEMMAP_SOURCE_SMEM ||
	    fdt_node_check_compatible(gd->fdt_blob, 0, "qcom,qcs404") == 0) {
		carveout_start = get_timer(0);
		/* Takes ~20-50ms on SDM845 */
		configure_reserved_memory();
		debug("carveout time: %lums\n", get_timer(carveout_start));
	}
	dcache_enable();
}
