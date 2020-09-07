/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#ifndef __AT91_PMC_H__
#define __AT91_PMC_H__

#include <regmap.h>
#include <linux/bitops.h>
#include <linux/io.h>

/* Keep a range of 256 available clocks for every clock type. */
#define AT91_TO_CLK_ID(_t, _i)		(((_t) << 8) | ((_i) & 0xff))
#define AT91_CLK_ID_TO_DID(_i)		((_i) & 0xff)

struct pmc_platdata {
	struct at91_pmc *reg_base;
	struct regmap *regmap_sfr;
};

int at91_pmc_core_probe(struct udevice *dev);
int at91_clk_sub_device_bind(struct udevice *dev, const char *drv_name);

int at91_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args);
int at91_clk_probe(struct udevice *dev);

int at91_clk_mux_val_to_index(const u32 *table, u32 num_parents, u32 val);
int at91_clk_mux_index_to_val(const u32 *table, u32 num_parents, u32 index);

void pmc_read(void __iomem *base, unsigned int off, unsigned int *val);
void pmc_write(void __iomem *base, unsigned int off, unsigned int val);
void pmc_update_bits(void __iomem *base, unsigned int off, unsigned int mask,
			unsigned int bits);
#endif
