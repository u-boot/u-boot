/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#ifndef __AT91_PMC_H__
#define __AT91_PMC_H__

#include <linux/bitops.h>
#include <linux/io.h>

/* Keep a range of 256 available clocks for every clock type. */
#define AT91_TO_CLK_ID(_t, _i)		(((_t) << 8) | ((_i) & 0xff))
#define AT91_CLK_ID_TO_DID(_i)		((_i) & 0xff)

struct clk *at91_clk_main_rc(void __iomem *reg, const char *name,
			const char *parent_name);
struct clk *at91_clk_main_osc(void __iomem *reg, const char *name,
			const char *parent_name, bool bypass);
struct clk *at91_clk_rm9200_main(void __iomem *reg, const char *name,
			const char *parent_name);
struct clk *at91_clk_sam9x5_main(void __iomem *reg, const char *name,
			const char * const *parent_names, int num_parents,
			const u32 *mux_table, int type);

int at91_clk_mux_val_to_index(const u32 *table, u32 num_parents, u32 val);
int at91_clk_mux_index_to_val(const u32 *table, u32 num_parents, u32 index);

void pmc_read(void __iomem *base, unsigned int off, unsigned int *val);
void pmc_write(void __iomem *base, unsigned int off, unsigned int val);
void pmc_update_bits(void __iomem *base, unsigned int off, unsigned int mask,
			unsigned int bits);

#endif
