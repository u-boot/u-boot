// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 */

#include "clk.h"

static ulong adi_get_rate(struct clk *clk)
{
	struct clk *c;
	int ret;

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	return clk_get_rate(c);
}

static ulong adi_set_rate(struct clk *clk, ulong rate)
{
	//Not yet implemented
	return 0;
}

static int adi_enable(struct clk *clk)
{
	//Not yet implemented
	return 0;
}

static int adi_disable(struct clk *clk)
{
	//Not yet implemented
	return 0;
}

const struct clk_ops adi_clk_ops = {
	.set_rate = adi_set_rate,
	.get_rate = adi_get_rate,
	.enable = adi_enable,
	.disable = adi_disable,
};

