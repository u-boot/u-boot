/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

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
	const fdt32_t *val;
	int len;

	val = get_memory_reg_prop(gd->fdt_blob, &len);
	if (len < sizeof(*val))
		return -EINVAL;

	gd->ram_size = fdt32_to_cpu(*(val + 1));

	debug("DRAM size = %08lx\n", gd->ram_size);

	return 0;
}

void dram_init_banksize(void)
{
	const fdt32_t *val;
	int len, i;

	val = get_memory_reg_prop(gd->fdt_blob, &len);
	if (len < 0)
		return;

	len /= sizeof(*val);
	len /= 2;

	for (i = 0; i < len; i++) {
		gd->bd->bi_dram[i].start = fdt32_to_cpu(*val++);
		gd->bd->bi_dram[i].size = fdt32_to_cpu(*val++);

		debug("DRAM bank %d: start = %08lx, size = %08lx\n",
		      i, gd->bd->bi_dram[i].start, gd->bd->bi_dram[i].size);
	}
}
