// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 SpacemiT Technology Co. Ltd
 * Copyright (c) 2024-2025 Haylen Chu <heylenay@4d2.org>
 * Copyright (c) 2025 Junhui Liu <junhui.liu@pigmoral.tech>
 *  Authors: Haylen Chu <heylenay@4d2.org>
 *
 * DDN stands for "Divider Denominator Numerator", it's M/N clock with a
 * constant x2 factor. This clock hardware follows the equation below,
 *
 *	      numerator       Fin
 *	2 * ------------- = -------
 *	     denominator      Fout
 *
 * Thus, Fout could be calculated with,
 *
 *		Fin	denominator
 *	Fout = ----- * -------------
 *		 2	 numerator
 */

#include <dm/device.h>
#include <regmap.h>
#include <linux/clk-provider.h>
#include <linux/rational.h>

#include "clk_ddn.h"

#define UBOOT_DM_SPACEMIT_CLK_DDN "spacemit_clk_ddn"

static unsigned long ccu_ddn_calc_rate(unsigned long prate, unsigned long num,
				       unsigned long den, unsigned int pre_div)
{
	return prate * den / pre_div / num;
}

static unsigned long ccu_ddn_calc_best_rate(struct ccu_ddn *ddn,
					    unsigned long rate, unsigned long prate,
					    unsigned long *num, unsigned long *den)
{
	rational_best_approximation(rate, prate / ddn->pre_div,
				    ddn->den_mask >> ddn->den_shift,
				    ddn->num_mask >> ddn->num_shift,
				    den, num);
	return ccu_ddn_calc_rate(prate, *num, *den, ddn->pre_div);
}

static unsigned long ccu_ddn_recalc_rate(struct clk *clk)
{
	struct ccu_ddn *ddn = clk_to_ccu_ddn(clk);
	unsigned int val, num, den;

	val = ccu_read(&ddn->common, ctrl);

	num = (val & ddn->num_mask) >> ddn->num_shift;
	den = (val & ddn->den_mask) >> ddn->den_shift;

	return ccu_ddn_calc_rate(clk_get_parent_rate(clk), num, den, ddn->pre_div);
}

static unsigned long ccu_ddn_set_rate(struct clk *clk, unsigned long rate)
{
	struct ccu_ddn *ddn = clk_to_ccu_ddn(clk);
	unsigned long num, den;

	ccu_ddn_calc_best_rate(ddn, rate, clk_get_parent_rate(clk), &num, &den);

	ccu_update(&ddn->common, ctrl,
		   ddn->num_mask | ddn->den_mask,
		   (num << ddn->num_shift) | (den << ddn->den_shift));

	return 0;
}

static const struct clk_ops spacemit_clk_ddn_ops = {
	.get_rate = ccu_ddn_recalc_rate,
	.set_rate = ccu_ddn_set_rate,
};

int spacemit_ddn_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_DDN,
			    common->name, common->parents[0]);
}

U_BOOT_DRIVER(spacemit_clk_ddn) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_DDN,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_ddn_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
