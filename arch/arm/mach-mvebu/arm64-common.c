/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * On ARMv8, MBus is not configured in U-Boot. To enable compilation
 * of the already implemented drivers, lets add a dummy version of
 * this function so that linking does not fail.
 */
const struct mbus_dram_target_info *mvebu_mbus_dram_info(void)
{
	return NULL;
}

/* DRAM init code ... */

static const void *get_memory_reg_prop(const void *fdt, int *lenp)
{
	int offset;

	offset = fdt_path_offset(fdt, "/memory");
	if (offset < 0)
		return NULL;

	return fdt_getprop(fdt, offset, "reg", lenp);
}

int dram_init(void)
{
	const void *fdt = gd->fdt_blob;
	const fdt32_t *val;
	int ac, sc, len;

	ac = fdt_address_cells(fdt, 0);
	sc = fdt_size_cells(fdt, 0);
	if (ac < 0 || sc < 1 || sc > 2) {
		printf("invalid address/size cells\n");
		return -EINVAL;
	}

	val = get_memory_reg_prop(fdt, &len);
	if (len / sizeof(*val) < ac + sc)
		return -EINVAL;

	val += ac;

	gd->ram_size = fdtdec_get_number(val, sc);

	debug("DRAM size = %08lx\n", (unsigned long)gd->ram_size);

	return 0;
}

void dram_init_banksize(void)
{
	const void *fdt = gd->fdt_blob;
	const fdt32_t *val;
	int ac, sc, cells, len, i;

	val = get_memory_reg_prop(fdt, &len);
	if (len < 0)
		return;

	ac = fdt_address_cells(fdt, 0);
	sc = fdt_size_cells(fdt, 0);
	if (ac < 1 || sc > 2 || sc < 1 || sc > 2) {
		printf("invalid address/size cells\n");
		return;
	}

	cells = ac + sc;

	len /= sizeof(*val);

	for (i = 0; i < CONFIG_NR_DRAM_BANKS && len >= cells;
	     i++, len -= cells) {
		gd->bd->bi_dram[i].start = fdtdec_get_number(val, ac);
		val += ac;
		gd->bd->bi_dram[i].size = fdtdec_get_number(val, sc);
		val += sc;

		debug("DRAM bank %d: start = %08lx, size = %08lx\n",
		      i, (unsigned long)gd->bd->bi_dram[i].start,
		      (unsigned long)gd->bd->bi_dram[i].size);
	}
}

int arch_cpu_init(void)
{
	/* Nothing to do (yet) */
	return 0;
}

int arch_early_init_r(void)
{
	struct udevice *dev;
	int ret;

	/* Call the comphy code via the MISC uclass driver */
	ret = uclass_get_device(UCLASS_MISC, 0, &dev);
	if (ret) {
		debug("COMPHY init failed: %d\n", ret);
		return -ENODEV;
	}

	/* Cause the SATA device to do its early init */
	uclass_first_device(UCLASS_AHCI, &dev);

	return 0;
}
