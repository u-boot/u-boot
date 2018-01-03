/*
 * Copyright (c) 2016-2018, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <asm/arch/tegra.h>

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

/*
 * These variables are written to before relocation, and hence cannot be
 * in.bss, since .bss overlaps the DTB that's appended to the U-Boot binary.
 * The section attribute forces this into .data and avoids this issue. This
 * also has the nice side-effect of the content being valid after relocation.
 */

/* A parsed version of /memory/reg from the DTB passed to U-Boot in x0 */
static struct {
	u64 start;
	u64 size;
} ram_banks[CONFIG_NR_DRAM_BANKS] __attribute__((section(".data")));

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

	memset(ram_banks, 0, sizeof(ram_banks));

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
	if (len > ARRAY_SIZE(ram_banks))
		len = ARRAY_SIZE(ram_banks);
	ram_bank_count = len;

	gd->ram_size = 0;
	for (i = 0; i < ram_bank_count; i++) {
		u64 bank_end, usable_bank_size;

		ram_banks[i].start = fdt_read_number(prop, na);
		prop += na;
		ram_banks[i].size = fdt_read_number(prop, ns);
		prop += ns;
		gd->ram_size += ram_banks[i].size;
		debug("Bank %d: start: %llx size: %llx\n", i,
		      ram_banks[i].start, ram_banks[i].size);

		bank_end = ram_banks[i].start + ram_banks[i].size;
		debug("  end  %llx\n", bank_end);
		if (bank_end > SZ_4G)
			bank_end = SZ_4G;
		debug("  end  %llx (usable)\n", bank_end);
		usable_bank_size = bank_end - ram_banks[i].start;
		debug("  size %llx (usable)\n", usable_bank_size);
		if ((usable_bank_size >= MIN_USABLE_RAM_SIZE) &&
		    (bank_end > ram_top)) {
			ram_top = bank_end;
			region_base = ram_banks[i].start;
			debug("ram top now %llx\n", ram_top);
		}
	}
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
		gd->bd->bi_dram[i].start = ram_banks[i].start;
		gd->bd->bi_dram[i].size = ram_banks[i].size;
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
