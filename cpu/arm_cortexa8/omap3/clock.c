/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *      Manikandan Pillai <mani.pillai@ti.com>
 *
 * Derived from Beagle Board and OMAP3 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
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
#include <asm/io.h>
#include <asm/arch/clocks.h>
#include <asm/arch/clocks_omap3.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <environment.h>
#include <command.h>

/******************************************************************************
 * get_sys_clk_speed() - determine reference oscillator speed
 *                       based on known 32kHz clock and gptimer.
 *****************************************************************************/
u32 get_osc_clk_speed(void)
{
	u32 start, cstart, cend, cdiff, val;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct prm *prm_base = (struct prm *)PRM_BASE;
	struct gptimer *gpt1_base = (struct gptimer *)OMAP34XX_GPT1;
	struct s32ktimer *s32k_base = (struct s32ktimer *)SYNC_32KTIMER_BASE;

	val = readl(&prm_base->clksrc_ctrl);

	/* If SYS_CLK is being divided by 2, remove for now */
	val = (val & (~SYSCLKDIV_2)) | SYSCLKDIV_1;
	writel(val, &prm_base->clksrc_ctrl);

	/* enable timer2 */
	val = readl(&prcm_base->clksel_wkup) | CLKSEL_GPT1;

	/* select sys_clk for GPT1 */
	writel(val, &prcm_base->clksel_wkup);

	/* Enable I and F Clocks for GPT1 */
	val = readl(&prcm_base->iclken_wkup) | EN_GPT1 | EN_32KSYNC;
	writel(val, &prcm_base->iclken_wkup);
	val = readl(&prcm_base->fclken_wkup) | EN_GPT1;
	writel(val, &prcm_base->fclken_wkup);

	writel(0, &gpt1_base->tldr);		/* start counting at 0 */
	writel(GPT_EN, &gpt1_base->tclr);	/* enable clock */

	/* enable 32kHz source, determine sys_clk via gauging */

	/* start time in 20 cycles */
	start = 20 + readl(&s32k_base->s32k_cr);

	/* dead loop till start time */
	while (readl(&s32k_base->s32k_cr) < start);

	/* get start sys_clk count */
	cstart = readl(&gpt1_base->tcrr);

	/* wait for 40 cycles */
	while (readl(&s32k_base->s32k_cr) < (start + 20)) ;
	cend = readl(&gpt1_base->tcrr);		/* get end sys_clk count */
	cdiff = cend - cstart;			/* get elapsed ticks */

	/* based on number of ticks assign speed */
	if (cdiff > 19000)
		return S38_4M;
	else if (cdiff > 15200)
		return S26M;
	else if (cdiff > 13000)
		return S24M;
	else if (cdiff > 9000)
		return S19_2M;
	else if (cdiff > 7600)
		return S13M;
	else
		return S12M;
}

/******************************************************************************
 * get_sys_clkin_sel() - returns the sys_clkin_sel field value based on
 *                       input oscillator clock frequency.
 *****************************************************************************/
void get_sys_clkin_sel(u32 osc_clk, u32 *sys_clkin_sel)
{
	switch(osc_clk) {
	case S38_4M:
		*sys_clkin_sel = 4;
		break;
	case S26M:
		*sys_clkin_sel = 3;
		break;
	case S19_2M:
		*sys_clkin_sel = 2;
		break;
	case S13M:
		*sys_clkin_sel = 1;
		break;
	case S12M:
	default:
		*sys_clkin_sel = 0;
	}
}

/******************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h
 *               called from SRAM, or Flash (using temp SRAM stack).
 *****************************************************************************/
void prcm_init(void)
{
	void (*f_lock_pll) (u32, u32, u32, u32);
	int xip_safe, p0, p1, p2, p3;
	u32 osc_clk = 0, sys_clkin_sel;
	u32 clk_index, sil_index = 0;
	struct prm *prm_base = (struct prm *)PRM_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *dpll_param_p;

	f_lock_pll = (void *) ((u32) &_end_vect - (u32) &_start +
				SRAM_VECT_CODE);

	xip_safe = is_running_in_sram();

	/*
	 * Gauge the input clock speed and find out the sys_clkin_sel
	 * value corresponding to the input clock.
	 */
	osc_clk = get_osc_clk_speed();
	get_sys_clkin_sel(osc_clk, &sys_clkin_sel);

	/* set input crystal speed */
	sr32(&prm_base->clksel, 0, 3, sys_clkin_sel);

	/* If the input clock is greater than 19.2M always divide/2 */
	if (sys_clkin_sel > 2) {
		/* input clock divider */
		sr32(&prm_base->clksrc_ctrl, 6, 2, 2);
		clk_index = sys_clkin_sel / 2;
	} else {
		/* input clock divider */
		sr32(&prm_base->clksrc_ctrl, 6, 2, 1);
		clk_index = sys_clkin_sel;
	}

	/*
	 * The DPLL tables are defined according to sysclk value and
	 * silicon revision. The clk_index value will be used to get
	 * the values for that input sysclk from the DPLL param table
	 * and sil_index will get the values for that SysClk for the
	 * appropriate silicon rev.
	 */
	if (get_cpu_rev())
		sil_index = 1;

	/* Unlock MPU DPLL (slows things down, and needed later) */
	sr32(&prcm_base->clken_pll_mpu, 0, 3, PLL_LOW_POWER_BYPASS);
	wait_on_value(ST_MPU_CLK, 0, &prcm_base->idlest_pll_mpu, LDELAY);

	/* Getting the base address of Core DPLL param table */
	dpll_param_p = (dpll_param *) get_core_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	dpll_param_p = dpll_param_p + 3 * clk_index + sil_index;
	if (xip_safe) {
		/*
		 * CORE DPLL
		 * sr32(CM_CLKSEL2_EMU) set override to work when asleep
		 */
		sr32(&prcm_base->clken_pll, 0, 3, PLL_FAST_RELOCK_BYPASS);
		wait_on_value(ST_CORE_CLK, 0, &prcm_base->idlest_ckgen,
				LDELAY);

		/*
		 * For OMAP3 ES1.0 Errata 1.50, default value directly doesn't
		 * work. write another value and then default value.
		 */

		/* m3x2 */
		sr32(&prcm_base->clksel1_emu, 16, 5, CORE_M3X2 + 1);
		/* m3x2 */
		sr32(&prcm_base->clksel1_emu, 16, 5, CORE_M3X2);
		/* Set M2 */
		sr32(&prcm_base->clksel1_pll, 27, 2, dpll_param_p->m2);
		/* Set M */
		sr32(&prcm_base->clksel1_pll, 16, 11, dpll_param_p->m);
		/* Set N */
		sr32(&prcm_base->clksel1_pll, 8, 7, dpll_param_p->n);
		/* 96M Src */
		sr32(&prcm_base->clksel1_pll, 6, 1, 0);
		/* ssi */
		sr32(&prcm_base->clksel_core, 8, 4, CORE_SSI_DIV);
		/* fsusb */
		sr32(&prcm_base->clksel_core, 4, 2, CORE_FUSB_DIV);
		/* l4 */
		sr32(&prcm_base->clksel_core, 2, 2, CORE_L4_DIV);
		/* l3 */
		sr32(&prcm_base->clksel_core, 0, 2, CORE_L3_DIV);
		/* gfx */
		sr32(&prcm_base->clksel_gfx, 0, 3, GFX_DIV);
		/* reset mgr */
		sr32(&prcm_base->clksel_wkup, 1, 2, WKUP_RSM);
		/* FREQSEL */
		sr32(&prcm_base->clken_pll, 4, 4, dpll_param_p->fsel);
		/* lock mode */
		sr32(&prcm_base->clken_pll, 0, 3, PLL_LOCK);

		wait_on_value(ST_CORE_CLK, 1, &prcm_base->idlest_ckgen,
				LDELAY);
	} else if (is_running_in_flash()) {
		/*
		 * if running from flash, jump to small relocated code
		 * area in SRAM.
		 */
		p0 = readl(&prcm_base->clken_pll);
		sr32(&p0, 0, 3, PLL_FAST_RELOCK_BYPASS);
		sr32(&p0, 4, 4, dpll_param_p->fsel);	/* FREQSEL */

		p1 = readl(&prcm_base->clksel1_pll);
		sr32(&p1, 27, 2, dpll_param_p->m2);	/* Set M2 */
		sr32(&p1, 16, 11, dpll_param_p->m);	/* Set M */
		sr32(&p1, 8, 7, dpll_param_p->n);		/* Set N */
		sr32(&p1, 6, 1, 0);	/* set source for 96M */

		p2 = readl(&prcm_base->clksel_core);
		sr32(&p2, 8, 4, CORE_SSI_DIV);	/* ssi */
		sr32(&p2, 4, 2, CORE_FUSB_DIV);	/* fsusb */
		sr32(&p2, 2, 2, CORE_L4_DIV);	/* l4 */
		sr32(&p2, 0, 2, CORE_L3_DIV);	/* l3 */

		p3 = (u32)&prcm_base->idlest_ckgen;

		(*f_lock_pll) (p0, p1, p2, p3);
	}

	/* PER DPLL */
	sr32(&prcm_base->clken_pll, 16, 3, PLL_STOP);
	wait_on_value(ST_PERIPH_CLK, 0, &prcm_base->idlest_ckgen, LDELAY);

	/* Getting the base address to PER DPLL param table */

	/* Set N */
	dpll_param_p = (dpll_param *) get_per_dpll_param();

	/* Moving it to the right sysclk base */
	dpll_param_p = dpll_param_p + clk_index;

	/*
	 * Errata 1.50 Workaround for OMAP3 ES1.0 only
	 * If using default divisors, write default divisor + 1
	 * and then the actual divisor value
	 */
	sr32(&prcm_base->clksel1_emu, 24, 5, PER_M6X2 + 1);	/* set M6 */
	sr32(&prcm_base->clksel1_emu, 24, 5, PER_M6X2);		/* set M6 */
	sr32(&prcm_base->clksel_cam, 0, 5, PER_M5X2 + 1);	/* set M5 */
	sr32(&prcm_base->clksel_cam, 0, 5, PER_M5X2);		/* set M5 */
	sr32(&prcm_base->clksel_dss, 0, 5, PER_M4X2 + 1);	/* set M4 */
	sr32(&prcm_base->clksel_dss, 0, 5, PER_M4X2);		/* set M4 */
	sr32(&prcm_base->clksel_dss, 8, 5, PER_M3X2 + 1);	/* set M3 */
	sr32(&prcm_base->clksel_dss, 8, 5, PER_M3X2);		/* set M3 */
	sr32(&prcm_base->clksel3_pll, 0, 5, dpll_param_p->m2 + 1); /* set M2 */
	sr32(&prcm_base->clksel3_pll, 0, 5, dpll_param_p->m2);	/* set M2 */
	/* Workaround end */

	sr32(&prcm_base->clksel2_pll, 8, 11, dpll_param_p->m);	/* set m */
	sr32(&prcm_base->clksel2_pll, 0, 7, dpll_param_p->n);	/* set n */
	sr32(&prcm_base->clken_pll, 20, 4, dpll_param_p->fsel);	/* FREQSEL */
	sr32(&prcm_base->clken_pll, 16, 3, PLL_LOCK);		/* lock mode */
	wait_on_value(ST_PERIPH_CLK, 2, &prcm_base->idlest_ckgen, LDELAY);

	/* Getting the base address to MPU DPLL param table */
	dpll_param_p = (dpll_param *) get_mpu_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	dpll_param_p = dpll_param_p + 3 * clk_index + sil_index;

	/* MPU DPLL (unlocked already) */

	/* Set M2 */
	sr32(&prcm_base->clksel2_pll_mpu, 0, 5, dpll_param_p->m2);
	/* Set M */
	sr32(&prcm_base->clksel1_pll_mpu, 8, 11, dpll_param_p->m);
	/* Set N */
	sr32(&prcm_base->clksel1_pll_mpu, 0, 7, dpll_param_p->n);
	/* FREQSEL */
	sr32(&prcm_base->clken_pll_mpu, 4, 4, dpll_param_p->fsel);
	/* lock mode */
	sr32(&prcm_base->clken_pll_mpu, 0, 3, PLL_LOCK);
	wait_on_value(ST_MPU_CLK, 1, &prcm_base->idlest_pll_mpu, LDELAY);

	/* Getting the base address to IVA DPLL param table */
	dpll_param_p = (dpll_param *) get_iva_dpll_param();

	/* Moving it to the right sysclk and ES rev base */
	dpll_param_p = dpll_param_p + 3 * clk_index + sil_index;

	/* IVA DPLL (set to 12*20=240MHz) */
	sr32(&prcm_base->clken_pll_iva2, 0, 3, PLL_STOP);
	wait_on_value(ST_IVA2_CLK, 0, &prcm_base->idlest_pll_iva2, LDELAY);
	/* set M2 */
	sr32(&prcm_base->clksel2_pll_iva2, 0, 5, dpll_param_p->m2);
	/* set M */
	sr32(&prcm_base->clksel1_pll_iva2, 8, 11, dpll_param_p->m);
	/* set N */
	sr32(&prcm_base->clksel1_pll_iva2, 0, 7, dpll_param_p->n);
	/* FREQSEL */
	sr32(&prcm_base->clken_pll_iva2, 4, 4, dpll_param_p->fsel);
	/* lock mode */
	sr32(&prcm_base->clken_pll_iva2, 0, 3, PLL_LOCK);
	wait_on_value(ST_IVA2_CLK, 1, &prcm_base->idlest_pll_iva2, LDELAY);

	/* Set up GPTimers to sys_clk source only */
	sr32(&prcm_base->clksel_per, 0, 8, 0xff);
	sr32(&prcm_base->clksel_wkup, 0, 1, 1);

	sdelay(5000);
}

/******************************************************************************
 * peripheral_enable() - Enable the clks & power for perifs (GPT2, UART1,...)
 *****************************************************************************/
void per_clocks_enable(void)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	/* Enable GP2 timer. */
	sr32(&prcm_base->clksel_per, 0, 1, 0x1);	/* GPT2 = sys clk */
	sr32(&prcm_base->iclken_per, 3, 1, 0x1);	/* ICKen GPT2 */
	sr32(&prcm_base->fclken_per, 3, 1, 0x1);	/* FCKen GPT2 */

#ifdef CONFIG_SYS_NS16550
	/* Enable UART1 clocks */
	sr32(&prcm_base->fclken1_core, 13, 1, 0x1);
	sr32(&prcm_base->iclken1_core, 13, 1, 0x1);

	/* UART 3 Clocks */
	sr32(&prcm_base->fclken_per, 11, 1, 0x1);
	sr32(&prcm_base->iclken_per, 11, 1, 0x1);
#endif

#ifdef CONFIG_OMAP3_GPIO_2
	sr32(&prcm_base->fclken_per, 13, 1, 1);
	sr32(&prcm_base->iclken_per, 13, 1, 1);
#endif
#ifdef CONFIG_OMAP3_GPIO_3
	sr32(&prcm_base->fclken_per, 14, 1, 1);
	sr32(&prcm_base->iclken_per, 14, 1, 1);
#endif
#ifdef CONFIG_OMAP3_GPIO_4
	sr32(&prcm_base->fclken_per, 15, 1, 1);
	sr32(&prcm_base->iclken_per, 15, 1, 1);
#endif
#ifdef CONFIG_OMAP3_GPIO_5
	sr32(&prcm_base->fclken_per, 16, 1, 1);
	sr32(&prcm_base->iclken_per, 16, 1, 1);
#endif
#ifdef CONFIG_OMAP3_GPIO_6
	sr32(&prcm_base->fclken_per, 17, 1, 1);
	sr32(&prcm_base->iclken_per, 17, 1, 1);
#endif

#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	/* Turn on all 3 I2C clocks */
	sr32(&prcm_base->fclken1_core, 15, 3, 0x7);
	sr32(&prcm_base->iclken1_core, 15, 3, 0x7);	/* I2C1,2,3 = on */
#endif
	/* Enable the ICLK for 32K Sync Timer as its used in udelay */
	sr32(&prcm_base->iclken_wkup, 2, 1, 0x1);

	sr32(&prcm_base->fclken_iva2, 0, 32, FCK_IVA2_ON);
	sr32(&prcm_base->fclken1_core, 0, 32, FCK_CORE1_ON);
	sr32(&prcm_base->iclken1_core, 0, 32, ICK_CORE1_ON);
	sr32(&prcm_base->iclken2_core, 0, 32, ICK_CORE2_ON);
	sr32(&prcm_base->fclken_wkup, 0, 32, FCK_WKUP_ON);
	sr32(&prcm_base->iclken_wkup, 0, 32, ICK_WKUP_ON);
	sr32(&prcm_base->fclken_dss, 0, 32, FCK_DSS_ON);
	sr32(&prcm_base->iclken_dss, 0, 32, ICK_DSS_ON);
	sr32(&prcm_base->fclken_cam, 0, 32, FCK_CAM_ON);
	sr32(&prcm_base->iclken_cam, 0, 32, ICK_CAM_ON);
	sr32(&prcm_base->fclken_per, 0, 32, FCK_PER_ON);
	sr32(&prcm_base->iclken_per, 0, 32, ICK_PER_ON);

	sdelay(1000);
}
