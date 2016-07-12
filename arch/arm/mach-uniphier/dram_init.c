/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <linux/err.h>

#include "init.h"
#include "soc-info.h"

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

#ifdef CONFIG_OF_BOARD_SETUP
/*
 * The DRAM PHY requires 64 byte scratch area in each DRAM channel
 * for its dynamic PHY training feature.
 */
int ft_board_setup(void *fdt, bd_t *bd)
{
	const struct uniphier_board_data *param;
	unsigned long rsv_addr;
	const unsigned long rsv_size = 64;
	int ch, ret;

	if (uniphier_get_soc_type() != SOC_UNIPHIER_LD20)
		return 0;

	param = uniphier_get_board_param();
	if (!param) {
		printf("failed to get board parameter\n");
		return -ENODEV;
	}

	for (ch = 0; ch < param->dram_nr_ch; ch++) {
		rsv_addr = param->dram_ch[ch].base + param->dram_ch[ch].size;
		rsv_addr -= rsv_size;

		ret = fdt_add_mem_rsv(fdt, rsv_addr, rsv_size);
		if (ret)
			return -ENOSPC;

		printf("   Reserved memory region for DRAM PHY training: addr=%lx size=%lx\n",
		       rsv_addr, rsv_size);
	}

	return 0;
}
#endif
