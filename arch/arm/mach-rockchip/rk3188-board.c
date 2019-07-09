// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3188.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/pmu_rk3288.h>
#include <asm/arch-rockchip/boot_mode.h>

__weak int rk_board_late_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct rk3188_grf *grf;

	setup_boot_mode();
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(grf)) {
		pr_err("grf syscon returned %ld\n", PTR_ERR(grf));
	} else {
		/* enable noc remap to mimic legacy loaders */
		rk_clrsetreg(&grf->soc_con0,
			NOC_REMAP_MASK << NOC_REMAP_SHIFT,
			NOC_REMAP_MASK << NOC_REMAP_SHIFT);
	}

	return rk_board_late_init();
}

int board_init(void)
{
	return 0;
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
