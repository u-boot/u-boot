/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <asm/arch/tegra.h>

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long nvtboot_boot_x0;

/*
 * A parsed version of /memory/reg from the DTB that is passed to U-Boot in x0.
 *
 * We only support up to two banks since that's all the binary  bootloader
 * ever sets. We assume bank 0 is RAM below 4G and bank 1 is RAM  above 4G.
 * This is all a fairly safe assumption, since the L4T kernel makes  the same
 * assumptions, so the bootloader is unlikely to change.
 *
 * This is written to before relocation, and hence cannot be in .bss, since
 * .bss overlaps the DTB that's appended to the U-Boot binary. The initializer
 * forces this into .data and avoids this issue. This also has the nice side-
 * effect of the content being valid after relocation.
 */
static struct {
	u64 start;
	u64 size;
} ram_banks[2] = {{1}};

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
		error("Can't find /memory node in nvtboot DTB");
		hang();
	}
	prop = fdt_getprop(nvtboot_blob, node, "reg", &len);
	if (!prop) {
		error("Can't find /memory/reg property in nvtboot DTB");
		hang();
	}

	len /= (na + ns);
	if (len > ARRAY_SIZE(ram_banks))
		len = ARRAY_SIZE(ram_banks);

	gd->ram_size = 0;
	for (i = 0; i < len; i++) {
		ram_banks[i].start = fdt_read_number(prop, na);
		prop += na;
		ram_banks[i].size = fdt_read_number(prop, ns);
		prop += ns;
		gd->ram_size += ram_banks[i].size;
	}

	return 0;
}

extern unsigned long nvtboot_boot_x0;

int dram_init_banksize(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		gd->bd->bi_dram[i].start = ram_banks[i].start;
		gd->bd->bi_dram[i].size = ram_banks[i].size;
	}

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	return ram_banks[0].start + ram_banks[0].size;
}
