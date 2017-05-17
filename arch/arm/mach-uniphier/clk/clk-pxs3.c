/*
 * Copyright (C) 2017 Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"

#define SDCTRL_EMMC_HW_RESET	0x59810280

void uniphier_pxs3_clk_init(void)
{
	/* TODO: use "mmc-pwrseq-emmc" */
	writel(1, SDCTRL_EMMC_HW_RESET);
}
