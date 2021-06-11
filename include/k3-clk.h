/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2020 - Texas Instruments Incorporated - http://www.ti.com
 *      Tero Kristo <t-kristo@ti.com>
 */

#ifndef __K3_CLK_H__
#define __K3_CLK_H__

#include <linux/clk-provider.h>

struct clk *clk_register_ti_pll(const char *name, const char *parent_name,
				void __iomem *reg);

#endif /* __K3_CLK_H__ */
