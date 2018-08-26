/*
 * Clock Initialization for board based on EXYNOS4210
 *
 * Copyright (C) 2013 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include "common_setup.h"
#ifdef CONFIG_EXYNOS4x12
#include "exynos4x12_setup.h"
#else
#include "exynos4_setup.h"
#endif

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
#ifndef CONFIG_EXYNOS4x12
void system_clock_init(void)
{
	struct exynos4_clock *clk =
			(struct exynos4_clock *)samsung_get_base_clock();

	writel(CLK_SRC_CPU_VAL, &clk->src_cpu);

	sdelay(0x10000);

	writel(CLK_SRC_TOP0_VAL, &clk->src_top0);
	writel(CLK_SRC_TOP1_VAL, &clk->src_top1);
	writel(CLK_SRC_DMC_VAL, &clk->src_dmc);
	writel(CLK_SRC_LEFTBUS_VAL, &clk->src_leftbus);
	writel(CLK_SRC_RIGHTBUS_VAL, &clk->src_rightbus);
	writel(CLK_SRC_FSYS_VAL, &clk->src_fsys);
	writel(CLK_SRC_PERIL0_VAL, &clk->src_peril0);
	writel(CLK_SRC_CAM_VAL, &clk->src_cam);
	writel(CLK_SRC_MFC_VAL, &clk->src_mfc);
	writel(CLK_SRC_G3D_VAL, &clk->src_g3d);
	writel(CLK_SRC_LCD0_VAL, &clk->src_lcd0);

	sdelay(0x10000);

	writel(CLK_DIV_CPU0_VAL, &clk->div_cpu0);
	writel(CLK_DIV_CPU1_VAL, &clk->div_cpu1);
	writel(CLK_DIV_DMC0_VAL, &clk->div_dmc0);
	writel(CLK_DIV_DMC1_VAL, &clk->div_dmc1);
	writel(CLK_DIV_LEFTBUS_VAL, &clk->div_leftbus);
	writel(CLK_DIV_RIGHTBUS_VAL, &clk->div_rightbus);
	writel(CLK_DIV_TOP_VAL, &clk->div_top);
	writel(CLK_DIV_FSYS1_VAL, &clk->div_fsys1);
	writel(CLK_DIV_FSYS2_VAL, &clk->div_fsys2);
	writel(CLK_DIV_FSYS3_VAL, &clk->div_fsys3);
	writel(CLK_DIV_PERIL0_VAL, &clk->div_peril0);
	writel(CLK_DIV_CAM_VAL, &clk->div_cam);
	writel(CLK_DIV_MFC_VAL, &clk->div_mfc);
	writel(CLK_DIV_G3D_VAL, &clk->div_g3d);
	writel(CLK_DIV_LCD0_VAL, &clk->div_lcd0);

	/* Set PLL locktime */
	writel(PLL_LOCKTIME, &clk->apll_lock);
	writel(PLL_LOCKTIME, &clk->mpll_lock);
	writel(PLL_LOCKTIME, &clk->epll_lock);
	writel(PLL_LOCKTIME, &clk->vpll_lock);

	writel(APLL_CON1_VAL, &clk->apll_con1);
	writel(APLL_CON0_VAL, &clk->apll_con0);
	writel(MPLL_CON1_VAL, &clk->mpll_con1);
	writel(MPLL_CON0_VAL, &clk->mpll_con0);
	writel(EPLL_CON1_VAL, &clk->epll_con1);
	writel(EPLL_CON0_VAL, &clk->epll_con0);
	writel(VPLL_CON1_VAL, &clk->vpll_con1);
	writel(VPLL_CON0_VAL, &clk->vpll_con0);

	sdelay(0x30000);
}
#else
static inline void wait_mux_state(unsigned int status, void *addr)
{
    unsigned int res = 0;

    do {
        res = readl(addr);
    }while(status != res);
}

static inline void wait_pll_lock(void *addr)
{
    unsigned int res = 0;

    do {
        res = readl(addr);
        res >>= 29;
    }while(!(res & 0x1));
}

void system_clock_init(void)
{
    struct exynos4x12_clock *clk =
        (struct exynos4x12_clock *)samsung_get_base_clock();

    writel(CLK_SRC_CPU_RESET, &clk->src_cpu);
    wait_mux_state(CLK_MUX_STAT_CPU_RESET, &clk->mux_stat_cpu);
    writel(CLK_DIV_DMC0_VAL, &clk->div_dmc0);
    writel(CLK_DIV_DMC1_VAL, &clk->div_dmc1);

	writel(CLK_SRC_TOP0_RESET, &clk->src_top0);
    wait_mux_state(CLK_MUX_STAT_TOP0_RESET, &clk->mux_stat_top0);
	writel(CLK_SRC_TOP1_RESET, &clk->src_top1);
    wait_mux_state(CLK_MUX_STAT_TOP1_RESET, &clk->mux_stat_top1);
    writel(CLK_DIV_TOP_VAL, &clk->div_top);

    writel(CLK_SRC_LEFTBUS_VAL, &clk->src_leftbus);
    wait_mux_state(CLK_MUX_STAT_LEFTBUS_VAL, &clk->mux_stat_leftbus);
    writel(CLK_DIV_LEFTBUS_VAL, &clk->div_leftbus);

    writel(CLK_SRC_RIGHTBUS_VAL, &clk->src_rightbus);
    wait_mux_state(CLK_MUX_STAT_RIGHTBUS_VAL, &clk->mux_stat_rightbus);
    writel(CLK_DIV_RIGHTBUS_VAL, &clk->div_rightbus);

    writel(APLL_LOCK_VAL, &clk->apll_lock);
    writel(MPLL_LOCK_VAL, &clk->mpll_lock);
    writel(EPLL_LOCK_VAL, &clk->epll_lock);
    writel(VPLL_LOCK_VAL, &clk->vpll_lock);

    writel(CLK_DIV_CPU0_VAL, &clk->div_cpu0);
    writel(CLK_DIV_CPU1_VAL, &clk->div_cpu1);

    writel(APLL_CON1_VAL, &clk->apll_con1);
    writel(APLL_CON0_VAL, &clk->apll_con0);
    writel(MPLL_CON1_VAL, &clk->mpll_con1);
    writel(MPLL_CON0_VAL, &clk->mpll_con0);
    writel(EPLL_CON2_VAL, &clk->epll_con2);
    writel(EPLL_CON1_VAL, &clk->epll_con1);
    writel(EPLL_CON0_VAL, &clk->epll_con0);
    writel(VPLL_CON2_VAL, &clk->vpll_con2);
    writel(VPLL_CON1_VAL, &clk->vpll_con1);
    writel(VPLL_CON0_VAL, &clk->vpll_con0);

    wait_pll_lock(&clk->apll_con0);
    wait_pll_lock(&clk->mpll_con0);
    wait_pll_lock(&clk->epll_con0);
    wait_pll_lock(&clk->vpll_con0);

    writel(CLK_SRC_CPU_VAL, &clk->src_cpu);
    wait_mux_state(CLK_MUX_STAT_CPU_VAL, &clk->mux_stat_cpu);
    writel(CLK_SRC_DMC_VAL, &clk->src_dmc);
    wait_mux_state(CLK_MUX_STAT_DMC_VAL, &clk->mux_stat_dmc);
    writel(CLK_SRC_TOP0_VAL, &clk->src_top0);
    wait_mux_state(CLK_MUX_STAT_TOP0_VAL, &clk->mux_stat_top0);
    writel(CLK_SRC_TOP1_VAL, &clk->src_top1);
    wait_mux_state(CLK_MUX_STAT_TOP1_VAL, &clk->mux_stat_top1);

    /* UART clock
     *
     * UART0-3 = 100MHz
     *
     */
    writel(CLK_SRC_PERIL0_VAL, &clk->src_peril0);
    writel(CLK_DIV_PERIL0_VAL, &clk->div_peril0);

    /* MMC clock
     *
     * MMC2, MMC4 = 50MHz
     *
     */
    writel(CLK_SRC_FSYS_VAL, &clk->src_fsys);
    writel(CLK_DIV_FSYS2_VAL, &clk->div_fsys2);
    writel(CLK_DIV_FSYS3_VAL, &clk->div_fsys3);
}
#endif
