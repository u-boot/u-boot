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
	u32 start, cstart, cend, cdiff, cdiv, val;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct prm *prm_base = (struct prm *)PRM_BASE;
	struct gptimer *gpt1_base = (struct gptimer *)OMAP34XX_GPT1;
	struct s32ktimer *s32k_base = (struct s32ktimer *)SYNC_32KTIMER_BASE;

	val = readl(&prm_base->clksrc_ctrl);

	if (val & SYSCLKDIV_2)
		cdiv = 2;
	else
		cdiv = 1;

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
	cdiff *= cdiv;

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

/*
 * OMAP34XX/35XX specific functions
 */

static void dpll3_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_core_dpll_param();
	void (*f_lock_pll) (u32, u32, u32, u32);
	int xip_safe, p0, p1, p2, p3;

	xip_safe = is_running_in_sram();

	/* Moving to the right sysclk and ES rev base */
	ptr = ptr + (3 * clk_index) + sil_index;

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

		/* CM_CLKSEL1_EMU[DIV_DPLL3] */
		sr32(&prcm_base->clksel1_emu, 16, 5, (CORE_M3X2 + 1)) ;
		sr32(&prcm_base->clksel1_emu, 16, 5, CORE_M3X2);

		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		sr32(&prcm_base->clksel1_pll, 27, 5, ptr->m2);

		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		sr32(&prcm_base->clksel1_pll, 16, 11, ptr->m);

		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		sr32(&prcm_base->clksel1_pll, 8, 7, ptr->n);

		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		sr32(&prcm_base->clksel1_pll, 6, 1, 0);

		/* SSI */
		sr32(&prcm_base->clksel_core, 8, 4, CORE_SSI_DIV);
		/* FSUSB */
		sr32(&prcm_base->clksel_core, 4, 2, CORE_FUSB_DIV);
		/* L4 */
		sr32(&prcm_base->clksel_core, 2, 2, CORE_L4_DIV);
		/* L3 */
		sr32(&prcm_base->clksel_core, 0, 2, CORE_L3_DIV);
		/* GFX */
		sr32(&prcm_base->clksel_gfx,  0, 3, GFX_DIV);
		/* RESET MGR */
		sr32(&prcm_base->clksel_wkup, 1, 2, WKUP_RSM);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		sr32(&prcm_base->clken_pll,   4, 4, ptr->fsel);
		/* LOCK MODE */
		sr32(&prcm_base->clken_pll,   0, 3, PLL_LOCK);

		wait_on_value(ST_CORE_CLK, 1, &prcm_base->idlest_ckgen,
				LDELAY);
	} else if (is_running_in_flash()) {
		/*
		 * if running from flash, jump to small relocated code
		 * area in SRAM.
		 */
		f_lock_pll = (void *) ((u32) &_end_vect - (u32) &_start +
				SRAM_VECT_CODE);

		p0 = readl(&prcm_base->clken_pll);
		sr32(&p0, 0, 3, PLL_FAST_RELOCK_BYPASS);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		sr32(&p0, 4, 4, ptr->fsel);

		p1 = readl(&prcm_base->clksel1_pll);
		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		sr32(&p1, 27, 5, ptr->m2);
		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		sr32(&p1, 16, 11, ptr->m);
		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		sr32(&p1, 8, 7, ptr->n);
		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		sr32(&p1, 6, 1, 0);

		p2 = readl(&prcm_base->clksel_core);
		/* SSI */
		sr32(&p2, 8, 4, CORE_SSI_DIV);
		/* FSUSB */
		sr32(&p2, 4, 2, CORE_FUSB_DIV);
		/* L4 */
		sr32(&p2, 2, 2, CORE_L4_DIV);
		/* L3 */
		sr32(&p2, 0, 2, CORE_L3_DIV);

		p3 = (u32)&prcm_base->idlest_ckgen;

		(*f_lock_pll) (p0, p1, p2, p3);
	}
}

static void dpll4_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_per_dpll_param();

	/* Moving it to the right sysclk base */
	ptr = ptr + clk_index;

	/* EN_PERIPH_DPLL: CM_CLKEN_PLL[16:18] */
	sr32(&prcm_base->clken_pll, 16, 3, PLL_STOP);
	wait_on_value(ST_PERIPH_CLK, 0, &prcm_base->idlest_ckgen, LDELAY);

	/*
	 * Errata 1.50 Workaround for OMAP3 ES1.0 only
	 * If using default divisors, write default divisor + 1
	 * and then the actual divisor value
	 */
	/* M6 */
	sr32(&prcm_base->clksel1_emu, 24, 5, (PER_M6X2 + 1));
	sr32(&prcm_base->clksel1_emu, 24, 5, PER_M6X2);
	/* M5 */
	sr32(&prcm_base->clksel_cam, 0, 5, (PER_M5X2 + 1));
	sr32(&prcm_base->clksel_cam, 0, 5, PER_M5X2);
	/* M4 */
	sr32(&prcm_base->clksel_dss, 0, 5, (PER_M4X2 + 1));
	sr32(&prcm_base->clksel_dss, 0, 5, PER_M4X2);
	/* M3 */
	sr32(&prcm_base->clksel_dss, 8, 5, (PER_M3X2 + 1));
	sr32(&prcm_base->clksel_dss, 8, 5, PER_M3X2);
	/* M2 (DIV_96M): CM_CLKSEL3_PLL[0:4] */
	sr32(&prcm_base->clksel3_pll, 0, 5, (ptr->m2 + 1));
	sr32(&prcm_base->clksel3_pll, 0, 5, ptr->m2);
	/* Workaround end */

	/* M (PERIPH_DPLL_MULT): CM_CLKSEL2_PLL[8:18] */
	sr32(&prcm_base->clksel2_pll, 8, 11, ptr->m);

	/* N (PERIPH_DPLL_DIV): CM_CLKSEL2_PLL[0:6] */
	sr32(&prcm_base->clksel2_pll, 0, 7, ptr->n);

	/* FREQSEL (PERIPH_DPLL_FREQSEL): CM_CLKEN_PLL[20:23] */
	sr32(&prcm_base->clken_pll, 20, 4, ptr->fsel);

	/* LOCK MODE (EN_PERIPH_DPLL): CM_CLKEN_PLL[16:18] */
	sr32(&prcm_base->clken_pll, 16, 3, PLL_LOCK);
	wait_on_value(ST_PERIPH_CLK, 2, &prcm_base->idlest_ckgen, LDELAY);
}

static void mpu_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_mpu_dpll_param();

	/* Moving to the right sysclk and ES rev base */
	ptr = ptr + (3 * clk_index) + sil_index;

	/* MPU DPLL (unlocked already) */

	/* M2 (MPU_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_MPU[0:4] */
	sr32(&prcm_base->clksel2_pll_mpu, 0, 5, ptr->m2);

	/* M (MPU_DPLL_MULT) : CM_CLKSEL2_PLL_MPU[8:18] */
	sr32(&prcm_base->clksel1_pll_mpu, 8, 11, ptr->m);

	/* N (MPU_DPLL_DIV) : CM_CLKSEL2_PLL_MPU[0:6] */
	sr32(&prcm_base->clksel1_pll_mpu, 0, 7, ptr->n);

	/* FREQSEL (MPU_DPLL_FREQSEL) : CM_CLKEN_PLL_MPU[4:7] */
	sr32(&prcm_base->clken_pll_mpu, 4, 4, ptr->fsel);
}

static void iva_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_iva_dpll_param();

	/* Moving to the right sysclk and ES rev base */
	ptr = ptr + (3 * clk_index) + sil_index;

	/* IVA DPLL */
	/* EN_IVA2_DPLL : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(&prcm_base->clken_pll_iva2, 0, 3, PLL_STOP);
	wait_on_value(ST_IVA2_CLK, 0, &prcm_base->idlest_pll_iva2, LDELAY);

	/* M2 (IVA2_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_IVA2[0:4] */
	sr32(&prcm_base->clksel2_pll_iva2, 0, 5, ptr->m2);

	/* M (IVA2_DPLL_MULT) : CM_CLKSEL1_PLL_IVA2[8:18] */
	sr32(&prcm_base->clksel1_pll_iva2, 8, 11, ptr->m);

	/* N (IVA2_DPLL_DIV) : CM_CLKSEL1_PLL_IVA2[0:6] */
	sr32(&prcm_base->clksel1_pll_iva2, 0, 7, ptr->n);

	/* FREQSEL (IVA2_DPLL_FREQSEL) : CM_CLKEN_PLL_IVA2[4:7] */
	sr32(&prcm_base->clken_pll_iva2, 4, 4, ptr->fsel);

	/* LOCK MODE (EN_IVA2_DPLL) : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(&prcm_base->clken_pll_iva2, 0, 3, PLL_LOCK);

	wait_on_value(ST_IVA2_CLK, 1, &prcm_base->idlest_pll_iva2, LDELAY);
}

/*
 * OMAP3630 specific functions
 */

static void dpll3_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_36x_core_dpll_param();
	void (*f_lock_pll) (u32, u32, u32, u32);
	int xip_safe, p0, p1, p2, p3;

	xip_safe = is_running_in_sram();

	/* Moving it to the right sysclk base */
	ptr += clk_index;

	if (xip_safe) {
		/* CORE DPLL */

		/* Select relock bypass: CM_CLKEN_PLL[0:2] */
		sr32(&prcm_base->clken_pll, 0, 3, PLL_FAST_RELOCK_BYPASS);
		wait_on_value(ST_CORE_CLK, 0, &prcm_base->idlest_ckgen,
				LDELAY);

		/* CM_CLKSEL1_EMU[DIV_DPLL3] */
		sr32(&prcm_base->clksel1_emu, 16, 5, CORE_M3X2);

		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		sr32(&prcm_base->clksel1_pll, 27, 5, ptr->m2);

		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		sr32(&prcm_base->clksel1_pll, 16, 11, ptr->m);

		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		sr32(&prcm_base->clksel1_pll, 8, 7, ptr->n);

		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		sr32(&prcm_base->clksel1_pll, 6, 1, 0);

		/* SSI */
		sr32(&prcm_base->clksel_core, 8, 4, CORE_SSI_DIV);
		/* FSUSB */
		sr32(&prcm_base->clksel_core, 4, 2, CORE_FUSB_DIV);
		/* L4 */
		sr32(&prcm_base->clksel_core, 2, 2, CORE_L4_DIV);
		/* L3 */
		sr32(&prcm_base->clksel_core, 0, 2, CORE_L3_DIV);
		/* GFX */
		sr32(&prcm_base->clksel_gfx,  0, 3, GFX_DIV);
		/* RESET MGR */
		sr32(&prcm_base->clksel_wkup, 1, 2, WKUP_RSM);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		sr32(&prcm_base->clken_pll,   4, 4, ptr->fsel);
		/* LOCK MODE */
		sr32(&prcm_base->clken_pll,   0, 3, PLL_LOCK);

		wait_on_value(ST_CORE_CLK, 1, &prcm_base->idlest_ckgen,
				LDELAY);
	} else if (is_running_in_flash()) {
		/*
		 * if running from flash, jump to small relocated code
		 * area in SRAM.
		 */
		f_lock_pll = (void *) ((u32) &_end_vect - (u32) &_start +
				SRAM_VECT_CODE);

		p0 = readl(&prcm_base->clken_pll);
		sr32(&p0, 0, 3, PLL_FAST_RELOCK_BYPASS);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		sr32(&p0, 4, 4, ptr->fsel);

		p1 = readl(&prcm_base->clksel1_pll);
		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		sr32(&p1, 27, 5, ptr->m2);
		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		sr32(&p1, 16, 11, ptr->m);
		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		sr32(&p1, 8, 7, ptr->n);
		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		sr32(&p1, 6, 1, 0);

		p2 = readl(&prcm_base->clksel_core);
		/* SSI */
		sr32(&p2, 8, 4, CORE_SSI_DIV);
		/* FSUSB */
		sr32(&p2, 4, 2, CORE_FUSB_DIV);
		/* L4 */
		sr32(&p2, 2, 2, CORE_L4_DIV);
		/* L3 */
		sr32(&p2, 0, 2, CORE_L3_DIV);

		p3 = (u32)&prcm_base->idlest_ckgen;

		(*f_lock_pll) (p0, p1, p2, p3);
	}
}

static void dpll4_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct dpll_per_36x_param *ptr;

	ptr = (struct dpll_per_36x_param *)get_36x_per_dpll_param();

	/* Moving it to the right sysclk base */
	ptr += clk_index;

	/* EN_PERIPH_DPLL: CM_CLKEN_PLL[16:18] */
	sr32(&prcm_base->clken_pll, 16, 3, PLL_STOP);
	wait_on_value(ST_PERIPH_CLK, 0, &prcm_base->idlest_ckgen, LDELAY);

	/* M6 (DIV_DPLL4): CM_CLKSEL1_EMU[24:29] */
	sr32(&prcm_base->clksel1_emu, 24, 6, ptr->m6);

	/* M5 (CLKSEL_CAM): CM_CLKSEL1_EMU[0:5] */
	sr32(&prcm_base->clksel_cam, 0, 6, ptr->m5);

	/* M4 (CLKSEL_DSS1): CM_CLKSEL_DSS[0:5] */
	sr32(&prcm_base->clksel_dss, 0, 6, ptr->m4);

	/* M3 (CLKSEL_DSS1): CM_CLKSEL_DSS[8:13] */
	sr32(&prcm_base->clksel_dss, 8, 6, ptr->m3);

	/* M2 (DIV_96M): CM_CLKSEL3_PLL[0:4] */
	sr32(&prcm_base->clksel3_pll, 0, 5, ptr->m2);

	/* M (PERIPH_DPLL_MULT): CM_CLKSEL2_PLL[8:19] */
	sr32(&prcm_base->clksel2_pll, 8, 12, ptr->m);

	/* N (PERIPH_DPLL_DIV): CM_CLKSEL2_PLL[0:6] */
	sr32(&prcm_base->clksel2_pll, 0, 7, ptr->n);

	/* M2DIV (CLKSEL_96M): CM_CLKSEL_CORE[12:13] */
	sr32(&prcm_base->clksel_core, 12, 2, ptr->m2div);

	/* LOCK MODE (EN_PERIPH_DPLL): CM_CLKEN_PLL[16:18] */
	sr32(&prcm_base->clken_pll, 16, 3, PLL_LOCK);
	wait_on_value(ST_PERIPH_CLK, 2, &prcm_base->idlest_ckgen, LDELAY);
}

static void mpu_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_36x_mpu_dpll_param();

	/* Moving to the right sysclk */
	ptr += clk_index;

	/* MPU DPLL (unlocked already */

	/* M2 (MPU_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_MPU[0:4] */
	sr32(&prcm_base->clksel2_pll_mpu, 0, 5, ptr->m2);

	/* M (MPU_DPLL_MULT) : CM_CLKSEL2_PLL_MPU[8:18] */
	sr32(&prcm_base->clksel1_pll_mpu, 8, 11, ptr->m);

	/* N (MPU_DPLL_DIV) : CM_CLKSEL2_PLL_MPU[0:6] */
	sr32(&prcm_base->clksel1_pll_mpu, 0, 7, ptr->n);
}

static void iva_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *)get_36x_iva_dpll_param();

	/* Moving to the right sysclk */
	ptr += clk_index;

	/* IVA DPLL */
	/* EN_IVA2_DPLL : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(&prcm_base->clken_pll_iva2, 0, 3, PLL_STOP);
	wait_on_value(ST_IVA2_CLK, 0, &prcm_base->idlest_pll_iva2, LDELAY);

	/* M2 (IVA2_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_IVA2[0:4] */
	sr32(&prcm_base->clksel2_pll_iva2, 0, 5, ptr->m2);

	/* M (IVA2_DPLL_MULT) : CM_CLKSEL1_PLL_IVA2[8:18] */
	sr32(&prcm_base->clksel1_pll_iva2, 8, 11, ptr->m);

	/* N (IVA2_DPLL_DIV) : CM_CLKSEL1_PLL_IVA2[0:6] */
	sr32(&prcm_base->clksel1_pll_iva2, 0, 7, ptr->n);

	/* LOCK (MODE (EN_IVA2_DPLL) : CM_CLKEN_PLL_IVA2[0:2] */
	sr32(&prcm_base->clken_pll_iva2, 0, 3, PLL_LOCK);

	wait_on_value(ST_IVA2_CLK, 1, &prcm_base->idlest_pll_iva2, LDELAY);
}

/******************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h
 *               called from SRAM, or Flash (using temp SRAM stack).
 *****************************************************************************/
void prcm_init(void)
{
	u32 osc_clk = 0, sys_clkin_sel;
	u32 clk_index, sil_index = 0;
	struct prm *prm_base = (struct prm *)PRM_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

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

	if (get_cpu_family() == CPU_OMAP36XX) {
		/* Unlock MPU DPLL (slows things down, and needed later) */
		sr32(&prcm_base->clken_pll_mpu, 0, 3, PLL_LOW_POWER_BYPASS);
		wait_on_value(ST_MPU_CLK, 0, &prcm_base->idlest_pll_mpu,
				LDELAY);

		dpll3_init_36xx(0, clk_index);
		dpll4_init_36xx(0, clk_index);
		iva_init_36xx(0, clk_index);
		mpu_init_36xx(0, clk_index);

		/* Lock MPU DPLL to set frequency */
		sr32(&prcm_base->clken_pll_mpu, 0, 3, PLL_LOCK);
		wait_on_value(ST_MPU_CLK, 1, &prcm_base->idlest_pll_mpu,
				LDELAY);
	} else {
		/*
		 * The DPLL tables are defined according to sysclk value and
		 * silicon revision. The clk_index value will be used to get
		 * the values for that input sysclk from the DPLL param table
		 * and sil_index will get the values for that SysClk for the
		 * appropriate silicon rev.
		 */
		if (((get_cpu_family() == CPU_OMAP34XX)
				&& (get_cpu_rev() >= CPU_3XX_ES20)) ||
			(get_cpu_family() == CPU_AM35XX))
			sil_index = 1;

		/* Unlock MPU DPLL (slows things down, and needed later) */
		sr32(&prcm_base->clken_pll_mpu, 0, 3, PLL_LOW_POWER_BYPASS);
		wait_on_value(ST_MPU_CLK, 0, &prcm_base->idlest_pll_mpu,
				LDELAY);

		dpll3_init_34xx(sil_index, clk_index);
		dpll4_init_34xx(sil_index, clk_index);
		iva_init_34xx(sil_index, clk_index);
		mpu_init_34xx(sil_index, clk_index);

		/* Lock MPU DPLL to set frequency */
		sr32(&prcm_base->clken_pll_mpu, 0, 3, PLL_LOCK);
		wait_on_value(ST_MPU_CLK, 1, &prcm_base->idlest_pll_mpu,
				LDELAY);
	}

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
