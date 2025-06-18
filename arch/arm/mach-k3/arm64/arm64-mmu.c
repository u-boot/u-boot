// SPDX-License-Identifier:     GPL-2.0+
/*
 * K3: ARM64 MMU setup
 *
 * Copyright (C) 2018-2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *	Suman Anna <s-anna@ti.com>
 * (This file is derived from arch/arm/mach-zynqmp/cpu.c)
 *
 */

#include <linux/sizes.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <mach/k3-ddr.h>
#include <sort.h>

#include "../common_fdt.h"

DECLARE_GLOBAL_DATA_PTR;

struct mm_region k3_mem_map[K3_MMU_REGIONS_COUNT] = {
	{
		/* Peripherals */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Higher DDR banks */
		.virt = 0x880000000UL,
		.phys = 0x880000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* Flash peripherals */
		.virt = 0x500000000UL,
		.phys = 0x500000000UL,
		.size = 0x380000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Map SPL load region and the next 128MiB as cacheable */
		.virt = CONFIG_SPL_TEXT_BASE,
		.phys = CONFIG_SPL_TEXT_BASE,
		.size = SZ_128M,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = k3_mem_map;

static int dt_reserved_cmp(const void *a, const void *b)
{
	const struct fdt_resource *val_a = a, *val_b = b;

	return (val_a->start - val_b->start);
}

int k3_mem_map_init(void)
{
	fdt_addr_t mem_base;
	fdt_size_t mem_size;
	struct fdt_resource dt_reserved[K3_MMU_REGIONS_COUNT],
		coalesced[K3_MMU_REGIONS_COUNT];
	int k3_map_idx = -EINVAL, ret, nodeoffset, subnode;
	void *blob = (void *)gd->fdt_blob;
	unsigned int carveout_len, i, j;

	ret = fdt_fixup_reserved(blob, "tfa", CONFIG_K3_ATF_LOAD_ADDR, 0x80000);
	if (ret) {
		pr_err("%s: Failed to fixup reserved node for tfa [%d]\n",
		       __func__, ret);
		return ret;
	}

	ret = fdt_fixup_reserved(blob, "optee", CONFIG_K3_OPTEE_LOAD_ADDR,
				 0x1800000);
	if (ret) {
		pr_err("%s: Failed to fixup reserved node for optee [%d]\n",
		       __func__, ret);
		return ret;
	}

	nodeoffset = fdt_subnode_offset(blob, 0, "memory");
	if (nodeoffset < 0) {
		pr_err("%s: Failed to get memory data: %s\n", __func__,
		       fdt_strerror(nodeoffset));
		return nodeoffset;
	}

	mem_base = fdtdec_get_addr_size(blob, nodeoffset, "reg", &mem_size);
	if (mem_base != CFG_SYS_SDRAM_BASE)
		return -EINVAL;

	for (i = 0; i < K3_MMU_REGIONS_COUNT; i++) {
		if (k3_mem_map[i].virt == CONFIG_SPL_TEXT_BASE) {
			k3_map_idx = i;
			break;
		}
	}

	if (k3_map_idx == -EINVAL) {
		pr_err("%s: Failed to find DDR region in MMU memory map\n",
		       __func__);
		return -EINVAL;
	}

	i = 0;
	nodeoffset = fdt_subnode_offset(blob, 0, "reserved-memory");
	fdt_for_each_subnode(subnode, blob, nodeoffset) {
		const char *name;
		fdt_addr_t addr, end_addr;
		fdt_size_t size;

		if (i >= K3_MMU_REGIONS_COUNT) {
			/*
			 * This is a recoverable error if the regions can be
			 * coalesced, the required logic can be implemented once
			 * requirement arises.
			 */
			pr_err("%s: Not enough space in MMU map for carveouts\n",
			       __func__);
			return -ENOMEM;
		}

		name = fdt_get_name(blob, subnode, NULL);
		addr = fdtdec_get_addr_size(blob, subnode, "reg", &size);

		if (addr == FDT_ADDR_T_NONE)
			continue;

		if (!fdtdec_get_bool(blob, subnode, "no-map"))
			continue;

		if (addr >= mem_base + mem_size)
			continue;

		end_addr = addr + size;

		if (end_addr <= mem_base)
			continue;

		debug("Added memory carveout at 0x%llx, size: 0x%llx for '%s'\n",
		      addr, size, name);

		addr = max(addr, mem_base);
		end_addr = min(end_addr, mem_base + mem_size);
		size = end_addr - addr;
		dt_reserved[i].start = addr;
		dt_reserved[i].end = end_addr;
		i++;
	}
	carveout_len = i;

	if (!carveout_len)
		return 0;

	/* sort carveout regions by address required for creating carveouts */
	qsort(dt_reserved, carveout_len, sizeof(dt_reserved[0]),
	      dt_reserved_cmp);

	/* coalesce regions */
	struct fdt_resource coalescing_temp = dt_reserved[0];

	j = 0;
	for (i = 1; i < carveout_len; i++) {
		struct fdt_resource current = dt_reserved[i];

		if (coalescing_temp.end >= current.start) {
			coalescing_temp.end = current.end;
			continue;
		}
		coalesced[j] = coalescing_temp;
		coalescing_temp = current;
		j++;
	}

	coalesced[j] = coalescing_temp;
	carveout_len = j + 1;

	if (coalesced[0].start != mem_base) {
		k3_mem_map[k3_map_idx].virt = mem_base;
		k3_mem_map[k3_map_idx].phys = mem_base;
		k3_mem_map[k3_map_idx].size = coalesced[0].start - mem_base;
		k3_mem_map[k3_map_idx].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					       PTE_BLOCK_INNER_SHARE;
		k3_map_idx++;
	}

	for (i = 1; i < carveout_len; i++) {
		k3_mem_map[k3_map_idx].virt = coalesced[i - 1].end;
		k3_mem_map[k3_map_idx].phys = coalesced[i - 1].end;
		k3_mem_map[k3_map_idx].size =
			coalesced[i].start - coalesced[i - 1].end;
		k3_mem_map[k3_map_idx].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					       PTE_BLOCK_INNER_SHARE;
		k3_map_idx++;
	}

	k3_mem_map[k3_map_idx].virt = coalesced[carveout_len - 1].end;
	k3_mem_map[k3_map_idx].phys = coalesced[carveout_len - 1].end;
	k3_mem_map[k3_map_idx].size =
		mem_base + mem_size - coalesced[carveout_len - 1].end;
	k3_mem_map[k3_map_idx].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				       PTE_BLOCK_INNER_SHARE;
	k3_map_idx++;

	/* map reserved memory as non cachable */
	for (i = 0; i < carveout_len; i++) {
		k3_mem_map[k3_map_idx].virt = coalesced[i].start;
		k3_mem_map[k3_map_idx].phys = coalesced[i].start;
		k3_mem_map[k3_map_idx].size =
			coalesced[i].end - coalesced[i].start;
		k3_mem_map[k3_map_idx].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) |
					       PTE_BLOCK_INNER_SHARE;
		k3_map_idx++;
	}

	debug("%s: MMU Table configured as:\n", __func__);
	debug("   |virt start\t\t|virt end\t|phys\t\t|size\t\t|attrs:\n");
	for (i = 0; i < k3_map_idx; i++) {
		debug("%2d: 0x%-12llx\t0x%-12llx\t0x%-12llx\t0x%-12llx\t0x%llx\n",
		      i, k3_mem_map[i].virt,
		      k3_mem_map[i].virt + k3_mem_map[i].size,
		      k3_mem_map[i].phys, k3_mem_map[i].size,
		      k3_mem_map[i].attrs);
	}

	k3_mem_map[k3_map_idx] = (const struct mm_region){ 0 };

	return 0;
}
