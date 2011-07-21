/*
 *
 * Clock initialization for OMAP4
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 *
 * Based on previous work by:
 *	Santosh Shilimkar <santosh.shilimkar@ti.com>
 *	Rajendra Nayak <rnayak@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/omap_common.h>
#include <asm/arch/clocks.h>
#include <asm/arch/sys_proto.h>
#include <asm/utils.h>

#ifndef CONFIG_SPL_BUILD
/*
 * printing to console doesn't work unless
 * this code is executed from SPL
 */
#define printf(fmt, args...)
#define puts(s)
#endif

#define abs(x) (((x) < 0) ? ((x)*-1) : (x))

struct omap4_prcm_regs *const prcm = (struct omap4_prcm_regs *)0x4A004100;

static const u32 sys_clk_array[8] = {
	12000000,	       /* 12 MHz */
	13000000,	       /* 13 MHz */
	16800000,	       /* 16.8 MHz */
	19200000,	       /* 19.2 MHz */
	26000000,	       /* 26 MHz */
	27000000,	       /* 27 MHz */
	38400000,	       /* 38.4 MHz */
};

/*
 * The M & N values in the following tables are created using the
 * following tool:
 * tools/omap/clocks_get_m_n.c
 * Please use this tool for creating the table for any new frequency.
 */

/* dpll locked at 1584 MHz - MPU clk at 792 MHz(OPP Turbo) */
static const struct dpll_params mpu_dpll_params_1584mhz[NUM_SYS_CLKS] = {
	{66, 0, 1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{792, 12, 1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{330, 6, 1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{165, 3, 1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{396, 12, 1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{88, 2, 1, -1, -1, -1, -1, -1},		/* 27 MHz   */
	{165, 7, 1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

/* dpll locked at 1200 MHz - MPU clk at 600 MHz */
static const struct dpll_params mpu_dpll_params_1200mhz[NUM_SYS_CLKS] = {
	{50, 0, 1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{600, 12, 1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{250, 6, 1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{125, 3, 1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{300, 12, 1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{200, 8, 1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{125, 7, 1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params core_dpll_params_1600mhz[NUM_SYS_CLKS] = {
	{200, 2, 1, 5, 8, 4, 6, 5},	/* 12 MHz   */
	{800, 12, 1, 5, 8, 4, 6, 5},	/* 13 MHz   */
	{619, 12, 1, 5, 8, 4, 6, 5},	/* 16.8 MHz */
	{125, 2, 1, 5, 8, 4, 6, 5},	/* 19.2 MHz */
	{400, 12, 1, 5, 8, 4, 6, 5},	/* 26 MHz   */
	{800, 26, 1, 5, 8, 4, 6, 5},	/* 27 MHz   */
	{125, 5, 1, 5, 8, 4, 6, 5}	/* 38.4 MHz */
};

static const struct dpll_params core_dpll_params_es1_1524mhz[NUM_SYS_CLKS] = {
	{127, 1, 1, 5, 8, 4, 6, 5},	/* 12 MHz   */
	{762, 12, 1, 5, 8, 4, 6, 5},	/* 13 MHz   */
	{635, 13, 1, 5, 8, 4, 6, 5},	/* 16.8 MHz */
	{635, 15, 1, 5, 8, 4, 6, 5},	/* 19.2 MHz */
	{381, 12, 1, 5, 8, 4, 6, 5},	/* 26 MHz   */
	{254, 8, 1, 5, 8, 4, 6, 5},	/* 27 MHz   */
	{496, 24, 1, 5, 8, 4, 6, 5}	/* 38.4 MHz */
};

static const struct dpll_params
		core_dpll_params_es2_1600mhz_ddr200mhz[NUM_SYS_CLKS] = {
	{200, 2, 2, 5, 8, 4, 6, 5},	/* 12 MHz   */
	{800, 12, 2, 5, 8, 4, 6, 5},	/* 13 MHz   */
	{619, 12, 2, 5, 8, 4, 6, 5},	/* 16.8 MHz */
	{125, 2, 2, 5, 8, 4, 6, 5},	/* 19.2 MHz */
	{400, 12, 2, 5, 8, 4, 6, 5},	/* 26 MHz   */
	{800, 26, 2, 5, 8, 4, 6, 5},	/* 27 MHz   */
	{125, 5, 2, 5, 8, 4, 6, 5}	/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_1536mhz[NUM_SYS_CLKS] = {
	{64, 0, 8, 6, 12, 9, 4, 5},	/* 12 MHz   */
	{768, 12, 8, 6, 12, 9, 4, 5},	/* 13 MHz   */
	{320, 6, 8, 6, 12, 9, 4, 5},	/* 16.8 MHz */
	{40, 0, 8, 6, 12, 9, 4, 5},	/* 19.2 MHz */
	{384, 12, 8, 6, 12, 9, 4, 5},	/* 26 MHz   */
	{256, 8, 8, 6, 12, 9, 4, 5},	/* 27 MHz   */
	{20, 0, 8, 6, 12, 9, 4, 5}	/* 38.4 MHz */
};

static const struct dpll_params iva_dpll_params_1862mhz[NUM_SYS_CLKS] = {
	{931, 11, -1, -1, 4, 7, -1, -1},	/* 12 MHz   */
	{931, 12, -1, -1, 4, 7, -1, -1},	/* 13 MHz   */
	{665, 11, -1, -1, 4, 7, -1, -1},	/* 16.8 MHz */
	{727, 14, -1, -1, 4, 7, -1, -1},	/* 19.2 MHz */
	{931, 25, -1, -1, 4, 7, -1, -1},	/* 26 MHz   */
	{931, 26, -1, -1, 4, 7, -1, -1},	/* 27 MHz   */
	{412, 16, -1, -1, 4, 7, -1, -1}		/* 38.4 MHz */
};

/* ABE M & N values with sys_clk as source */
static const struct dpll_params
		abe_dpll_params_sysclk_196608khz[NUM_SYS_CLKS] = {
	{49, 5, 1, 1, -1, -1, -1, -1},	/* 12 MHz   */
	{68, 8, 1, 1, -1, -1, -1, -1},	/* 13 MHz   */
	{35, 5, 1, 1, -1, -1, -1, -1},	/* 16.8 MHz */
	{46, 8, 1, 1, -1, -1, -1, -1},	/* 19.2 MHz */
	{34, 8, 1, 1, -1, -1, -1, -1},	/* 26 MHz   */
	{29, 7, 1, 1, -1, -1, -1, -1},	/* 27 MHz   */
	{64, 24, 1, 1, -1, -1, -1, -1}	/* 38.4 MHz */
};

/* ABE M & N values with 32K clock as source */
static const struct dpll_params abe_dpll_params_32k_196608khz = {
	750, 0, 1, 1, -1, -1, -1, -1
};


static const struct dpll_params usb_dpll_params_1920mhz[NUM_SYS_CLKS] = {
	{80, 0, 2, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{960, 12, 2, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{400, 6, 2, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{50, 0, 2, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{480, 12, 2, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{320, 8, 2, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{25, 0, 2, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

static inline u32 __get_sys_clk_index(void)
{
	u32 ind;
	/*
	 * For ES1 the ROM code calibration of sys clock is not reliable
	 * due to hw issue. So, use hard-coded value. If this value is not
	 * correct for any board over-ride this function in board file
	 * From ES2.0 onwards you will get this information from
	 * CM_SYS_CLKSEL
	 */
	if (omap_revision() == OMAP4430_ES1_0)
		ind = OMAP_SYS_CLK_IND_38_4_MHZ;
	else {
		/* SYS_CLKSEL - 1 to match the dpll param array indices */
		ind = (readl(&prcm->cm_sys_clksel) &
			CM_SYS_CLKSEL_SYS_CLKSEL_MASK) - 1;
	}
	return ind;
}

u32 get_sys_clk_index(void)
	__attribute__ ((weak, alias("__get_sys_clk_index")));

u32 get_sys_clk_freq(void)
{
	u8 index = get_sys_clk_index();
	return sys_clk_array[index];
}

static inline void do_bypass_dpll(u32 *const base)
{
	struct dpll_regs *dpll_regs = (struct dpll_regs *)base;

	clrsetbits_le32(&dpll_regs->cm_clkmode_dpll,
			CM_CLKMODE_DPLL_DPLL_EN_MASK,
			DPLL_EN_FAST_RELOCK_BYPASS <<
			CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_bypass(u32 *const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	if (!wait_on_value(ST_DPLL_CLK_MASK, 0, &dpll_regs->cm_idlest_dpll,
				LDELAY)) {
		printf("Bypassing DPLL failed %p\n", base);
	}
}

static inline void do_lock_dpll(u32 *const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	clrsetbits_le32(&dpll_regs->cm_clkmode_dpll,
		      CM_CLKMODE_DPLL_DPLL_EN_MASK,
		      DPLL_EN_LOCK << CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_lock(u32 *const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	if (!wait_on_value(ST_DPLL_CLK_MASK, ST_DPLL_CLK_MASK,
		&dpll_regs->cm_idlest_dpll, LDELAY)) {
		printf("DPLL locking failed for %p\n", base);
		hang();
	}
}

static void do_setup_dpll(u32 *const base, const struct dpll_params *params,
				u8 lock)
{
	u32 temp;
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	bypass_dpll(base);

	/* Set M & N */
	temp = readl(&dpll_regs->cm_clksel_dpll);

	temp &= ~CM_CLKSEL_DPLL_M_MASK;
	temp |= (params->m << CM_CLKSEL_DPLL_M_SHIFT) & CM_CLKSEL_DPLL_M_MASK;

	temp &= ~CM_CLKSEL_DPLL_N_MASK;
	temp |= (params->n << CM_CLKSEL_DPLL_N_SHIFT) & CM_CLKSEL_DPLL_N_MASK;

	writel(temp, &dpll_regs->cm_clksel_dpll);

	/* Lock */
	if (lock)
		do_lock_dpll(base);

	/* Setup post-dividers */
	if (params->m2 >= 0)
		writel(params->m2, &dpll_regs->cm_div_m2_dpll);
	if (params->m3 >= 0)
		writel(params->m3, &dpll_regs->cm_div_m3_dpll);
	if (params->m4 >= 0)
		writel(params->m4, &dpll_regs->cm_div_m4_dpll);
	if (params->m5 >= 0)
		writel(params->m5, &dpll_regs->cm_div_m5_dpll);
	if (params->m6 >= 0)
		writel(params->m6, &dpll_regs->cm_div_m6_dpll);
	if (params->m7 >= 0)
		writel(params->m7, &dpll_regs->cm_div_m7_dpll);

	/* Wait till the DPLL locks */
	if (lock)
		wait_for_lock(base);
}

const struct dpll_params *get_core_dpll_params(void)
{
	u32 sysclk_ind = get_sys_clk_index();

	switch (omap_revision()) {
	case OMAP4430_ES1_0:
		return &core_dpll_params_es1_1524mhz[sysclk_ind];
	case OMAP4430_ES2_0:
	case OMAP4430_SILICON_ID_INVALID:
		 /* safest */
		return &core_dpll_params_es2_1600mhz_ddr200mhz[sysclk_ind];
	default:
		return &core_dpll_params_1600mhz[sysclk_ind];
	}
}

u32 omap4_ddr_clk(void)
{
	u32 ddr_clk, sys_clk_khz;
	const struct dpll_params *core_dpll_params;

	sys_clk_khz = get_sys_clk_freq() / 1000;

	core_dpll_params = get_core_dpll_params();

	debug("sys_clk %d\n ", sys_clk_khz * 1000);

	/* Find Core DPLL locked frequency first */
	ddr_clk = sys_clk_khz * 2 * core_dpll_params->m /
			(core_dpll_params->n + 1);
	/*
	 * DDR frequency is PHY_ROOT_CLK/2
	 * PHY_ROOT_CLK = Fdpll/2/M2
	 */
	ddr_clk = ddr_clk / 4 / core_dpll_params->m2;

	ddr_clk *= 1000;	/* convert to Hz */
	debug("ddr_clk %d\n ", ddr_clk);

	return ddr_clk;
}

static void setup_dplls(void)
{
	u32 sysclk_ind, temp;
	const struct dpll_params *params;
	debug("setup_dplls\n");

	sysclk_ind = get_sys_clk_index();

	/* CORE dpll */
	params = get_core_dpll_params();	/* default - safest */
	/*
	 * Do not lock the core DPLL now. Just set it up.
	 * Core DPLL will be locked after setting up EMIF
	 * using the FREQ_UPDATE method(freq_update_core())
	 */
	do_setup_dpll(&prcm->cm_clkmode_dpll_core, params, DPLL_NO_LOCK);
	/* Set the ratios for CORE_CLK, L3_CLK, L4_CLK */
	temp = (CLKSEL_CORE_X2_DIV_1 << CLKSEL_CORE_SHIFT) |
	    (CLKSEL_L3_CORE_DIV_2 << CLKSEL_L3_SHIFT) |
	    (CLKSEL_L4_L3_DIV_2 << CLKSEL_L4_SHIFT);
	writel(temp, &prcm->cm_clksel_core);
	debug("Core DPLL configured\n");

	/* lock PER dpll */
	do_setup_dpll(&prcm->cm_clkmode_dpll_per,
			&per_dpll_params_1536mhz[sysclk_ind], DPLL_LOCK);
	debug("PER DPLL locked\n");

	/* MPU dpll */
	if (omap_revision() == OMAP4430_ES1_0)
		params = &mpu_dpll_params_1200mhz[sysclk_ind];
	else
		params = &mpu_dpll_params_1584mhz[sysclk_ind];
	do_setup_dpll(&prcm->cm_clkmode_dpll_mpu, params, DPLL_LOCK);
	debug("MPU DPLL locked\n");
}

static void setup_non_essential_dplls(void)
{
	u32 sys_clk_khz, abe_ref_clk;
	u32 sysclk_ind, sd_div, num, den;
	const struct dpll_params *params;

	sysclk_ind = get_sys_clk_index();
	sys_clk_khz = get_sys_clk_freq() / 1000;

	/* IVA */
	clrsetbits_le32(&prcm->cm_bypclk_dpll_iva,
		CM_BYPCLK_DPLL_IVA_CLKSEL_MASK, DPLL_IVA_CLKSEL_CORE_X2_DIV_2);

	do_setup_dpll(&prcm->cm_clkmode_dpll_iva,
			&iva_dpll_params_1862mhz[sysclk_ind], DPLL_LOCK);

	/*
	 * USB:
	 * USB dpll is J-type. Need to set DPLL_SD_DIV for jitter correction
	 * DPLL_SD_DIV = CEILING ([DPLL_MULT/(DPLL_DIV+1)]* CLKINP / 250)
	 *      - where CLKINP is sys_clk in MHz
	 * Use CLKINP in KHz and adjust the denominator accordingly so
	 * that we have enough accuracy and at the same time no overflow
	 */
	params = &usb_dpll_params_1920mhz[sysclk_ind];
	num = params->m * sys_clk_khz;
	den = (params->n + 1) * 250 * 1000;
	num += den - 1;
	sd_div = num / den;
	clrsetbits_le32(&prcm->cm_clksel_dpll_usb,
			CM_CLKSEL_DPLL_DPLL_SD_DIV_MASK,
			sd_div << CM_CLKSEL_DPLL_DPLL_SD_DIV_SHIFT);

	/* Now setup the dpll with the regular function */
	do_setup_dpll(&prcm->cm_clkmode_dpll_usb, params, DPLL_LOCK);

#ifdef CONFIG_SYS_OMAP4_ABE_SYSCK
	params = &abe_dpll_params_sysclk_196608khz[sysclk_ind];
	abe_ref_clk = CM_ABE_PLL_REF_CLKSEL_CLKSEL_SYSCLK;
#else
	params = &abe_dpll_params_32k_196608khz;
	abe_ref_clk = CM_ABE_PLL_REF_CLKSEL_CLKSEL_32KCLK;
	/*
	 * We need to enable some additional options to achieve
	 * 196.608MHz from 32768 Hz
	 */
	setbits_le32(&prcm->cm_clkmode_dpll_abe,
			CM_CLKMODE_DPLL_DRIFTGUARD_EN_MASK|
			CM_CLKMODE_DPLL_RELOCK_RAMP_EN_MASK|
			CM_CLKMODE_DPLL_LPMODE_EN_MASK|
			CM_CLKMODE_DPLL_REGM4XEN_MASK);
	/* Spend 4 REFCLK cycles at each stage */
	clrsetbits_le32(&prcm->cm_clkmode_dpll_abe,
			CM_CLKMODE_DPLL_RAMP_RATE_MASK,
			1 << CM_CLKMODE_DPLL_RAMP_RATE_SHIFT);
#endif

	/* Select the right reference clk */
	clrsetbits_le32(&prcm->cm_abe_pll_ref_clksel,
			CM_ABE_PLL_REF_CLKSEL_CLKSEL_MASK,
			abe_ref_clk << CM_ABE_PLL_REF_CLKSEL_CLKSEL_SHIFT);
	/* Lock the dpll */
	do_setup_dpll(&prcm->cm_clkmode_dpll_abe, params, DPLL_LOCK);
}

static void do_scale_vcore(u32 vcore_reg, u32 volt_mv)
{
	u32 temp, offset_code;
	u32 step = 12660; /* 12.66 mV represented in uV */
	u32 offset = volt_mv;

	/* convert to uV for better accuracy in the calculations */
	offset *= 1000;

	if (omap_revision() == OMAP4430_ES1_0)
		offset -= PHOENIX_SMPS_BASE_VOLT_STD_MODE_UV;
	else
		offset -= PHOENIX_SMPS_BASE_VOLT_STD_MODE_WITH_OFFSET_UV;

	offset_code = (offset + step - 1) / step;
	/* The code starts at 1 not 0 */
	offset_code++;

	debug("do_scale_vcore: volt - %d offset_code - 0x%x\n", volt_mv,
		offset_code);

	temp = SMPS_I2C_SLAVE_ADDR |
	    (vcore_reg << PRM_VC_VAL_BYPASS_REGADDR_SHIFT) |
	    (offset_code << PRM_VC_VAL_BYPASS_DATA_SHIFT) |
	    PRM_VC_VAL_BYPASS_VALID_BIT;
	writel(temp, &prcm->prm_vc_val_bypass);
	if (!wait_on_value(PRM_VC_VAL_BYPASS_VALID_BIT, 0,
				&prcm->prm_vc_val_bypass, LDELAY)) {
		printf("Scaling voltage failed for 0x%x\n", vcore_reg);
	}
}

/*
 * Setup the voltages for vdd_mpu, vdd_core, and vdd_iva
 * We set the maximum voltages allowed here because Smart-Reflex is not
 * enabled in bootloader. Voltage initialization in the kernel will set
 * these to the nominal values after enabling Smart-Reflex
 */
static void scale_vcores(void)
{
	u32 volt, sys_clk_khz, cycles_hi, cycles_low, temp;

	sys_clk_khz = get_sys_clk_freq() / 1000;

	/*
	 * Setup the dedicated I2C controller for Voltage Control
	 * I2C clk - high period 40% low period 60%
	 */
	cycles_hi = sys_clk_khz * 4 / PRM_VC_I2C_CHANNEL_FREQ_KHZ / 10;
	cycles_low = sys_clk_khz * 6 / PRM_VC_I2C_CHANNEL_FREQ_KHZ / 10;
	/* values to be set in register - less by 5 & 7 respectively */
	cycles_hi -= 5;
	cycles_low -= 7;
	temp = (cycles_hi << PRM_VC_CFG_I2C_CLK_SCLH_SHIFT) |
	       (cycles_low << PRM_VC_CFG_I2C_CLK_SCLL_SHIFT);
	writel(temp, &prcm->prm_vc_cfg_i2c_clk);

	/* Disable high speed mode and all advanced features */
	writel(0x0, &prcm->prm_vc_cfg_i2c_mode);

	/*
	 * VCORE 1 - 4430 : supplies vdd_mpu
	 * Setting a high voltage for Nitro mode as smart reflex is not enabled.
	 * We use the maximum possible value in the AVS range because the next
	 * higher voltage in the discrete range (code >= 0b111010) is way too
	 * high
	 */
	volt = 1417;
	do_scale_vcore(SMPS_REG_ADDR_VCORE1, volt);

	/* VCORE 2 - supplies vdd_iva */
	volt = 1200;
	do_scale_vcore(SMPS_REG_ADDR_VCORE2, volt);

	/* VCORE 3 - supplies vdd_core */
	volt = 1200;
	do_scale_vcore(SMPS_REG_ADDR_VCORE3, volt);
}

static inline void enable_clock_domain(u32 *const clkctrl_reg, u32 enable_mode)
{
	clrsetbits_le32(clkctrl_reg, CD_CLKCTRL_CLKTRCTRL_MASK,
			enable_mode << CD_CLKCTRL_CLKTRCTRL_SHIFT);
	debug("Enable clock domain - 0x%08x\n", clkctrl_reg);
}

static inline void wait_for_clk_enable(u32 *clkctrl_addr)
{
	u32 clkctrl, idlest = MODULE_CLKCTRL_IDLEST_DISABLED;
	u32 bound = LDELAY;

	while ((idlest == MODULE_CLKCTRL_IDLEST_DISABLED) ||
		(idlest == MODULE_CLKCTRL_IDLEST_TRANSITIONING)) {

		clkctrl = readl(clkctrl_addr);
		idlest = (clkctrl & MODULE_CLKCTRL_IDLEST_MASK) >>
			 MODULE_CLKCTRL_IDLEST_SHIFT;
		if (--bound == 0) {
			printf("Clock enable failed for 0x%p idlest 0x%x\n",
				clkctrl_addr, clkctrl);
			return;
		}
	}
}

static inline void enable_clock_module(u32 *const clkctrl_addr, u32 enable_mode,
				u32 wait_for_enable)
{
	clrsetbits_le32(clkctrl_addr, MODULE_CLKCTRL_MODULEMODE_MASK,
			enable_mode << MODULE_CLKCTRL_MODULEMODE_SHIFT);
	debug("Enable clock module - 0x%08x\n", clkctrl_addr);
	if (wait_for_enable)
		wait_for_clk_enable(clkctrl_addr);
}

/*
 * Enable essential clock domains, modules and
 * do some additional special settings needed
 */
static void enable_basic_clocks(void)
{
	u32 i, max = 100, wait_for_enable = 1;
	u32 *const clk_domains_essential[] = {
		&prcm->cm_l4per_clkstctrl,
		&prcm->cm_l3init_clkstctrl,
		&prcm->cm_memif_clkstctrl,
		&prcm->cm_l4cfg_clkstctrl,
		0
	};

	u32 *const clk_modules_hw_auto_essential[] = {
		&prcm->cm_wkup_gpio1_clkctrl,
		&prcm->cm_l4per_gpio2_clkctrl,
		&prcm->cm_l4per_gpio3_clkctrl,
		&prcm->cm_l4per_gpio4_clkctrl,
		&prcm->cm_l4per_gpio5_clkctrl,
		&prcm->cm_l4per_gpio6_clkctrl,
		&prcm->cm_memif_emif_1_clkctrl,
		&prcm->cm_memif_emif_2_clkctrl,
		&prcm->cm_l3init_hsusbotg_clkctrl,
		&prcm->cm_l3init_usbphy_clkctrl,
		&prcm->cm_l4cfg_l4_cfg_clkctrl,
		0
	};

	u32 *const clk_modules_explicit_en_essential[] = {
		&prcm->cm_l4per_gptimer2_clkctrl,
		&prcm->cm_l3init_hsmmc1_clkctrl,
		&prcm->cm_l3init_hsmmc2_clkctrl,
		&prcm->cm_l4per_mcspi1_clkctrl,
		&prcm->cm_wkup_gptimer1_clkctrl,
		&prcm->cm_l4per_i2c1_clkctrl,
		&prcm->cm_l4per_i2c2_clkctrl,
		&prcm->cm_l4per_i2c3_clkctrl,
		&prcm->cm_l4per_i2c4_clkctrl,
		&prcm->cm_wkup_wdtimer2_clkctrl,
		&prcm->cm_l4per_uart3_clkctrl,
		0
	};

	/* Enable optional additional functional clock for GPIO4 */
	setbits_le32(&prcm->cm_l4per_gpio4_clkctrl,
			GPIO4_CLKCTRL_OPTFCLKEN_MASK);

	/* Enable 96 MHz clock for MMC1 & MMC2 */
	setbits_le32(&prcm->cm_l3init_hsmmc1_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_MASK);
	setbits_le32(&prcm->cm_l3init_hsmmc2_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_MASK);

	/* Select 32KHz clock as the source of GPTIMER1 */
	setbits_le32(&prcm->cm_wkup_gptimer1_clkctrl,
			GPTIMER1_CLKCTRL_CLKSEL_MASK);

	/* Enable optional 48M functional clock for USB  PHY */
	setbits_le32(&prcm->cm_l3init_usbphy_clkctrl,
			USBPHY_CLKCTRL_OPTFCLKEN_PHY_48M_MASK);

	/* Put the clock domains in SW_WKUP mode */
	for (i = 0; (i < max) && clk_domains_essential[i]; i++) {
		enable_clock_domain(clk_domains_essential[i],
				    CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	}

	/* Clock modules that need to be put in HW_AUTO */
	for (i = 0; (i < max) && clk_modules_hw_auto_essential[i]; i++) {
		enable_clock_module(clk_modules_hw_auto_essential[i],
				    MODULE_CLKCTRL_MODULEMODE_HW_AUTO,
				    wait_for_enable);
	};

	/* Clock modules that need to be put in SW_EXPLICIT_EN mode */
	for (i = 0; (i < max) && clk_modules_explicit_en_essential[i]; i++) {
		enable_clock_module(clk_modules_explicit_en_essential[i],
				    MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN,
				    wait_for_enable);
	};

	/* Put the clock domains in HW_AUTO mode now */
	for (i = 0; (i < max) && clk_domains_essential[i]; i++) {
		enable_clock_domain(clk_domains_essential[i],
				    CD_CLKCTRL_CLKTRCTRL_HW_AUTO);
	}
}

/*
 * Enable non-essential clock domains, modules and
 * do some additional special settings needed
 */
static void enable_non_essential_clocks(void)
{
	u32 i, max = 100, wait_for_enable = 0;
	u32 *const clk_domains_non_essential[] = {
		&prcm->cm_mpu_m3_clkstctrl,
		&prcm->cm_ivahd_clkstctrl,
		&prcm->cm_dsp_clkstctrl,
		&prcm->cm_dss_clkstctrl,
		&prcm->cm_sgx_clkstctrl,
		&prcm->cm1_abe_clkstctrl,
		&prcm->cm_c2c_clkstctrl,
		&prcm->cm_cam_clkstctrl,
		&prcm->cm_dss_clkstctrl,
		&prcm->cm_sdma_clkstctrl,
		0
	};

	u32 *const clk_modules_hw_auto_non_essential[] = {
		&prcm->cm_mpu_m3_mpu_m3_clkctrl,
		&prcm->cm_ivahd_ivahd_clkctrl,
		&prcm->cm_ivahd_sl2_clkctrl,
		&prcm->cm_dsp_dsp_clkctrl,
		&prcm->cm_l3_2_gpmc_clkctrl,
		&prcm->cm_l3instr_l3_3_clkctrl,
		&prcm->cm_l3instr_l3_instr_clkctrl,
		&prcm->cm_l3instr_intrconn_wp1_clkctrl,
		&prcm->cm_l3init_hsi_clkctrl,
		&prcm->cm_l3init_hsusbtll_clkctrl,
		0
	};

	u32 *const clk_modules_explicit_en_non_essential[] = {
		&prcm->cm1_abe_aess_clkctrl,
		&prcm->cm1_abe_pdm_clkctrl,
		&prcm->cm1_abe_dmic_clkctrl,
		&prcm->cm1_abe_mcasp_clkctrl,
		&prcm->cm1_abe_mcbsp1_clkctrl,
		&prcm->cm1_abe_mcbsp2_clkctrl,
		&prcm->cm1_abe_mcbsp3_clkctrl,
		&prcm->cm1_abe_slimbus_clkctrl,
		&prcm->cm1_abe_timer5_clkctrl,
		&prcm->cm1_abe_timer6_clkctrl,
		&prcm->cm1_abe_timer7_clkctrl,
		&prcm->cm1_abe_timer8_clkctrl,
		&prcm->cm1_abe_wdt3_clkctrl,
		&prcm->cm_l4per_gptimer9_clkctrl,
		&prcm->cm_l4per_gptimer10_clkctrl,
		&prcm->cm_l4per_gptimer11_clkctrl,
		&prcm->cm_l4per_gptimer3_clkctrl,
		&prcm->cm_l4per_gptimer4_clkctrl,
		&prcm->cm_l4per_hdq1w_clkctrl,
		&prcm->cm_l4per_mcbsp4_clkctrl,
		&prcm->cm_l4per_mcspi2_clkctrl,
		&prcm->cm_l4per_mcspi3_clkctrl,
		&prcm->cm_l4per_mcspi4_clkctrl,
		&prcm->cm_l4per_mmcsd3_clkctrl,
		&prcm->cm_l4per_mmcsd4_clkctrl,
		&prcm->cm_l4per_mmcsd5_clkctrl,
		&prcm->cm_l4per_uart1_clkctrl,
		&prcm->cm_l4per_uart2_clkctrl,
		&prcm->cm_l4per_uart4_clkctrl,
		&prcm->cm_wkup_keyboard_clkctrl,
		&prcm->cm_wkup_wdtimer2_clkctrl,
		&prcm->cm_cam_iss_clkctrl,
		&prcm->cm_cam_fdif_clkctrl,
		&prcm->cm_dss_dss_clkctrl,
		&prcm->cm_sgx_sgx_clkctrl,
		&prcm->cm_l3init_hsusbhost_clkctrl,
		&prcm->cm_l3init_fsusb_clkctrl,
		0
	};

	/* Enable optional functional clock for ISS */
	setbits_le32(&prcm->cm_cam_iss_clkctrl, ISS_CLKCTRL_OPTFCLKEN_MASK);

	/* Enable all optional functional clocks of DSS */
	setbits_le32(&prcm->cm_dss_dss_clkctrl, DSS_CLKCTRL_OPTFCLKEN_MASK);


	/* Put the clock domains in SW_WKUP mode */
	for (i = 0; (i < max) && clk_domains_non_essential[i]; i++) {
		enable_clock_domain(clk_domains_non_essential[i],
				    CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	}

	/* Clock modules that need to be put in HW_AUTO */
	for (i = 0; (i < max) && clk_modules_hw_auto_non_essential[i]; i++) {
		enable_clock_module(clk_modules_hw_auto_non_essential[i],
				    MODULE_CLKCTRL_MODULEMODE_HW_AUTO,
				    wait_for_enable);
	};

	/* Clock modules that need to be put in SW_EXPLICIT_EN mode */
	for (i = 0; (i < max) && clk_modules_explicit_en_non_essential[i];
	     i++) {
		enable_clock_module(clk_modules_explicit_en_non_essential[i],
				    MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN,
				    wait_for_enable);
	};

	/* Put the clock domains in HW_AUTO mode now */
	for (i = 0; (i < max) && clk_domains_non_essential[i]; i++) {
		enable_clock_domain(clk_domains_non_essential[i],
				    CD_CLKCTRL_CLKTRCTRL_HW_AUTO);
	}

	/* Put camera module in no sleep mode */
	clrsetbits_le32(&prcm->cm_cam_clkstctrl, MODULE_CLKCTRL_MODULEMODE_MASK,
			CD_CLKCTRL_CLKTRCTRL_NO_SLEEP <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
}


void freq_update_core(void)
{
	u32 freq_config1 = 0;
	const struct dpll_params *core_dpll_params;

	core_dpll_params = get_core_dpll_params();
	/* Put EMIF clock domain in sw wakeup mode */
	enable_clock_domain(&prcm->cm_memif_clkstctrl,
				CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	wait_for_clk_enable(&prcm->cm_memif_emif_1_clkctrl);
	wait_for_clk_enable(&prcm->cm_memif_emif_2_clkctrl);

	freq_config1 = SHADOW_FREQ_CONFIG1_FREQ_UPDATE_MASK |
	    SHADOW_FREQ_CONFIG1_DLL_RESET_MASK;

	freq_config1 |= (DPLL_EN_LOCK << SHADOW_FREQ_CONFIG1_DPLL_EN_SHIFT) &
				SHADOW_FREQ_CONFIG1_DPLL_EN_MASK;

	freq_config1 |= (core_dpll_params->m2 <<
			SHADOW_FREQ_CONFIG1_M2_DIV_SHIFT) &
			SHADOW_FREQ_CONFIG1_M2_DIV_MASK;

	writel(freq_config1, &prcm->cm_shadow_freq_config1);
	if (!wait_on_value(SHADOW_FREQ_CONFIG1_FREQ_UPDATE_MASK, 0,
				&prcm->cm_shadow_freq_config1, LDELAY)) {
		puts("FREQ UPDATE procedure failed!!");
		hang();
	}

	/* Put EMIF clock domain back in hw auto mode */
	enable_clock_domain(&prcm->cm_memif_clkstctrl,
				CD_CLKCTRL_CLKTRCTRL_HW_AUTO);
	wait_for_clk_enable(&prcm->cm_memif_emif_1_clkctrl);
	wait_for_clk_enable(&prcm->cm_memif_emif_2_clkctrl);
}

void bypass_dpll(u32 *const base)
{
	do_bypass_dpll(base);
	wait_for_bypass(base);
}

void lock_dpll(u32 *const base)
{
	do_lock_dpll(base);
	wait_for_lock(base);
}

void setup_clocks_for_console(void)
{
	/* Do not add any spl_debug prints in this function */
	clrsetbits_le32(&prcm->cm_l4per_clkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);

	/* Enable all UARTs - console will be on one of them */
	clrsetbits_le32(&prcm->cm_l4per_uart1_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32(&prcm->cm_l4per_uart2_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32(&prcm->cm_l4per_uart3_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32(&prcm->cm_l4per_uart3_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32(&prcm->cm_l4per_clkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_HW_AUTO <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);
}

void prcm_init(void)
{
	switch (omap4_hw_init_context()) {
	case OMAP_INIT_CONTEXT_SPL:
	case OMAP_INIT_CONTEXT_UBOOT_FROM_NOR:
	case OMAP_INIT_CONTEXT_UBOOT_AFTER_CH:
		scale_vcores();
		setup_dplls();
		enable_basic_clocks();
		setup_non_essential_dplls();
		enable_non_essential_clocks();
		break;
	default:
		break;
	}
}
