/*
 * (C) Copyright 2015 Chen-Yu Tsai <wens@csie.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/tzpc.h>

/* Configure Trust Zone Protection Controller */
void tzpc_init(void)
{
	struct sunxi_tzpc *tzpc = (struct sunxi_tzpc *)SUNXI_TZPC_BASE;

	/* Enable non-secure access to the RTC */
	writel(SUNXI_TZPC_DECPORT0_RTC, &tzpc->decport0_set);
}
