/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AT91_PMC_H__
#define __AT91_PMC_H__

struct pmc_platdata {
	struct at91_pmc *reg_base;
};

int at91_pmc_core_probe(struct udevice *dev);
int at91_clk_sub_device_bind(struct udevice *dev, const char *drv_name);

int at91_clk_of_xlate(struct clk *clk, struct fdtdec_phandle_args *args);
int at91_clk_probe(struct udevice *dev);

#endif
