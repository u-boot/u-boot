/*
 * Copyright (c) 2016-2018, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <asm/arch/tegra.h>
#include <asm/armv8/mmu.h>

#define SZ_4G 0x100000000ULL

/*
 * Size of a region that's large enough to hold the relocated U-Boot and all
 * other allocations made around it (stack, heap, page tables, etc.)
 * In practice, running "bdinfo" at the shell prompt, the stack reaches about
 * 5MB from the address selected for ram_top as of the time of writing,
 * so a 16MB region should be plenty.
 */
#define MIN_USABLE_RAM_SIZE SZ_16M
/*
 * The amount of space we expect to require for stack usage. Used to validate
 * that all reservations fit into the region selected for the relocation target
 */
#define MIN_USABLE_STACK_SIZE SZ_1M

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long nvtboot_boot_x0;
extern struct mm_region tegra_mem_map[];

/*
 * These variables are written to before relocation, and hence cannot be
 * in.bss, since .bss overlaps the DTB that's appended to the U-Boot binary.
 * The section attribute forces this into .data and avoids this issue. This
 * also has the nice side-effect of the content being valid after relocation.
 */

/* The number of valid entries in ram_banks[] */
static int ram_bank_count __attribute__((section(".data")));

/*
 * The usable top-of-RAM for U-Boot. This is both:
 * a) Below 4GB to avoid issues with peripherals that use 32-bit addressing.
 * b) At the end of a region that has enough space to hold the relocated U-Boot
 *    and all other allocations made around it (stack, heap, page tables, etc.)
 */
static u64 ram_top __attribute__((section(".data")));
/* The base address of the region of RAM that ends at ram_top */
static u64 region_base __attribute__((section(".data")));

int dram_init(void)
{
	unsigned int na, ns;
	const void *nvtboot_blob = (void *)nvtboot_boot_x0;
	int node, len, i;
	const u32 *prop;

	na = fdtdec_get_uint(nvtboot_blob, 0, "#address-cells", 2);
	ns = fdtdec_get_uint(nvtboot_blob, 0, "#size-cells", 2);

	node = fdt_path_offset(nvtboot_blob, "/memory");
	if (node < 0) {
		pr_err("Can't find /memory node in nvtboot DTB");
		hang();
	}
	prop = fdt_getprop(nvtboot_blob, node, "reg", &len);
	if (!prop) {
		pr_err("Can't find /memory/reg property in nvtboot DTB");
		hang();
	}

	/* Calculate the true # of base/size pairs to read */
	len /= 4;		/* Convert bytes to number of cells */
	len /= (na + ns);	/* Convert cells to number of banks */
	if (len > CONFIG_NR_DRAM_BANKS)
		len = CONFIG_NR_DRAM_BANKS;

	/* Parse the /memory node, and save useful entries */
	gd->ram_size = 0;
	ram_bank_count = 0;
	for (i = 0; i < len; i++) {
		u64 bank_start, bank_end, bank_size, usable_bank_size;

		/* Extract raw memory region data from DTB */
		bank_start = fdt_read_number(prop, na);
		prop += na;
		bank_size = fdt_read_number(prop, ns);
		prop += ns;
		gd->ram_size += bank_size;
		bank_end = bank_start + bank_size;
		debug("Bank %d: %llx..%llx (+%llx)\n", i,
		      bank_start, bank_end, bank_size);

		/*
		 * Align the bank to MMU section size. This is not strictly
		 * necessary, since the translation table construction code
		 * handles page granularity without issue. However, aligning
		 * the MMU entries reduces the size and number of levels in the
		 * page table, so is worth it.
		 */
		bank_start = ROUND(bank_start, SZ_2M);
		bank_end = bank_end & ~(SZ_2M - 1);
		bank_size = bank_end - bank_start;
		debug("  aligned: %llx..%llx (+%llx)\n",
		      bank_start, bank_end, bank_size);
		if (bank_end <= bank_start)
			continue;

		/* Record data used to create MMU translation tables */
		ram_bank_count++;
		/* Index below is deliberately 1-based to skip MMIO entry */
		tegra_mem_map[ram_bank_count].virt = bank_start;
		tegra_mem_map[ram_bank_count].phys = bank_start;
		tegra_mem_map[ram_bank_count].size = bank_size;
		tegra_mem_map[ram_bank_count].attrs =
			PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE;

		/* Determine best bank to relocate U-Boot into */
		if (bank_end > SZ_4G)
			bank_end = SZ_4G;
		debug("  end  %llx (usable)\n", bank_end);
		usable_bank_size = bank_end - bank_start;
		debug("  size %llx (usable)\n", usable_bank_size);
		if ((usable_bank_size >= MIN_USABLE_RAM_SIZE) &&
		    (bank_end > ram_top)) {
			ram_top = bank_end;
			region_base = bank_start;
			debug("ram top now %llx\n", ram_top);
		}
	}

	/* Ensure memory map contains the desired sentinel entry */
	tegra_mem_map[ram_bank_count + 1].virt = 0;
	tegra_mem_map[ram_bank_count + 1].phys = 0;
	tegra_mem_map[ram_bank_count + 1].size = 0;
	tegra_mem_map[ram_bank_count + 1].attrs = 0;

	/* Error out if a relocation target couldn't be found */
	if (!ram_top) {
		pr_err("Can't find a usable RAM top");
		hang();
	}

	return 0;
}

int dram_init_banksize(void)
{
	int i;

	if ((gd->start_addr_sp - region_base) < MIN_USABLE_STACK_SIZE) {
		pr_err("Reservations exceed chosen region size");
		hang();
	}

	for (i = 0; i < ram_bank_count; i++) {
		gd->bd->bi_dram[i].start = tegra_mem_map[1 + i].virt;
		gd->bd->bi_dram[i].size = tegra_mem_map[1 + i].size;
	}

#ifdef CONFIG_PCI
	gd->pci_ram_top = ram_top;
#endif

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	return ram_top;
}
