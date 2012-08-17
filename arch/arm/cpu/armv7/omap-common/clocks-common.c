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
#include <asm/gpio.h>
#include <asm/arch/clocks.h>
#include <asm/arch/sys_proto.h>
#include <asm/utils.h>
#include <asm/omap_gpio.h>

#ifndef CONFIG_SPL_BUILD
/*
 * printing to console doesn't work unless
 * this code is executed from SPL
 */
#define printf(fmt, args...)
#define puts(s)
#endif

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

inline u32 check_for_lock(u32 *const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;
	u32 lock = readl(&dpll_regs->cm_idlest_dpll) & ST_DPLL_CLK_MASK;

	return lock;
}

static void do_setup_dpll(u32 *const base, const struct dpll_params *params,
				u8 lock, char *dpll)
{
	u32 temp, M, N;
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	temp = readl(&dpll_regs->cm_clksel_dpll);

	if (check_for_lock(base)) {
		/*
		 * The Dpll has already been locked by rom code using CH.
		 * Check if M,N are matching with Ideal nominal opp values.
		 * If matches, skip the rest otherwise relock.
		 */
		M = (temp & CM_CLKSEL_DPLL_M_MASK) >> CM_CLKSEL_DPLL_M_SHIFT;
		N = (temp & CM_CLKSEL_DPLL_N_MASK) >> CM_CLKSEL_DPLL_N_SHIFT;
		if ((M != (params->m)) || (N != (params->n))) {
			debug("\n %s Dpll locked, but not for ideal M = %d,"
				"N = %d values, current values are M = %d,"
				"N= %d" , dpll, params->m, params->n,
				M, N);
		} else {
			/* Dpll locked with ideal values for nominal opps. */
			debug("\n %s Dpll already locked with ideal"
						"nominal opp values", dpll);
			goto setup_post_dividers;
		}
	}

	bypass_dpll(base);

	/* Set M & N */
	temp &= ~CM_CLKSEL_DPLL_M_MASK;
	temp |= (params->m << CM_CLKSEL_DPLL_M_SHIFT) & CM_CLKSEL_DPLL_M_MASK;

	temp &= ~CM_CLKSEL_DPLL_N_MASK;
	temp |= (params->n << CM_CLKSEL_DPLL_N_SHIFT) & CM_CLKSEL_DPLL_N_MASK;

	writel(temp, &dpll_regs->cm_clksel_dpll);

	/* Lock */
	if (lock)
		do_lock_dpll(base);

setup_post_dividers:
	setup_post_dividers(base, params);

	/* Wait till the DPLL locks */
	if (lock)
		wait_for_lock(base);
}

u32 omap_ddr_clk(void)
{
	u32 ddr_clk, sys_clk_khz, omap_rev, divider;
	const struct dpll_params *core_dpll_params;

	omap_rev = omap_revision();
	sys_clk_khz = get_sys_clk_freq() / 1000;

	core_dpll_params = get_core_dpll_params();

	debug("sys_clk %d\n ", sys_clk_khz * 1000);

	/* Find Core DPLL locked frequency first */
	ddr_clk = sys_clk_khz * 2 * core_dpll_params->m /
			(core_dpll_params->n + 1);

	if (omap_rev < OMAP5430_ES1_0) {
		/*
		 * DDR frequency is PHY_ROOT_CLK/2
		 * PHY_ROOT_CLK = Fdpll/2/M2
		 */
		divider = 4;
	} else {
		/*
		 * DDR frequency is PHY_ROOT_CLK
		 * PHY_ROOT_CLK = Fdpll/2/M2
		 */
		divider = 2;
	}

	ddr_clk = ddr_clk / divider / core_dpll_params->m2;
	ddr_clk *= 1000;	/* convert to Hz */
	debug("ddr_clk %d\n ", ddr_clk);

	return ddr_clk;
}

/*
 * Lock MPU dpll
 *
 * Resulting MPU frequencies:
 * 4430 ES1.0	: 600 MHz
 * 4430 ES2.x	: 792 MHz (OPP Turbo)
 * 4460		: 920 MHz (OPP Turbo) - DCC disabled
 */
void configure_mpu_dpll(void)
{
	const struct dpll_params *params;
	struct dpll_regs *mpu_dpll_regs;
	u32 omap_rev;
	omap_rev = omap_revision();

	/*
	 * DCC and clock divider settings for 4460.
	 * DCC is required, if more than a certain frequency is required.
	 * For, 4460 > 1GHZ.
	 *     5430 > 1.4GHZ.
	 */
	if ((omap_rev >= OMAP4460_ES1_0) && (omap_rev < OMAP5430_ES1_0)) {
		mpu_dpll_regs =
			(struct dpll_regs *)&prcm->cm_clkmode_dpll_mpu;
		bypass_dpll(&prcm->cm_clkmode_dpll_mpu);
		clrbits_le32(&prcm->cm_mpu_mpu_clkctrl,
			MPU_CLKCTRL_CLKSEL_EMIF_DIV_MODE_MASK);
		setbits_le32(&prcm->cm_mpu_mpu_clkctrl,
			MPU_CLKCTRL_CLKSEL_ABE_DIV_MODE_MASK);
		clrbits_le32(&mpu_dpll_regs->cm_clksel_dpll,
			CM_CLKSEL_DCC_EN_MASK);
	}

	params = get_mpu_dpll_params();

	do_setup_dpll(&prcm->cm_clkmode_dpll_mpu, params, DPLL_LOCK, "mpu");
	debug("MPU DPLL locked\n");
}

#ifdef CONFIG_USB_EHCI_OMAP
static void setup_usb_dpll(void)
{
	const struct dpll_params *params;
	u32 sys_clk_khz, sd_div, num, den;

	sys_clk_khz = get_sys_clk_freq() / 1000;
	/*
	 * USB:
	 * USB dpll is J-type. Need to set DPLL_SD_DIV for jitter correction
	 * DPLL_SD_DIV = CEILING ([DPLL_MULT/(DPLL_DIV+1)]* CLKINP / 250)
	 *      - where CLKINP is sys_clk in MHz
	 * Use CLKINP in KHz and adjust the denominator accordingly so
	 * that we have enough accuracy and at the same time no overflow
	 */
	params = get_usb_dpll_params();
	num = params->m * sys_clk_khz;
	den = (params->n + 1) * 250 * 1000;
	num += den - 1;
	sd_div = num / den;
	clrsetbits_le32(&prcm->cm_clksel_dpll_usb,
			CM_CLKSEL_DPLL_DPLL_SD_DIV_MASK,
			sd_div << CM_CLKSEL_DPLL_DPLL_SD_DIV_SHIFT);

	/* Now setup the dpll with the regular function */
	do_setup_dpll(&prcm->cm_clkmode_dpll_usb, params, DPLL_LOCK, "usb");
}
#endif

static void setup_dplls(void)
{
	u32 temp;
	const struct dpll_params *params;

	debug("setup_dplls\n");

	/* CORE dpll */
	params = get_core_dpll_params();	/* default - safest */
	/*
	 * Do not lock the core DPLL now. Just set it up.
	 * Core DPLL will be locked after setting up EMIF
	 * using the FREQ_UPDATE method(freq_update_core())
	 */
	do_setup_dpll(&prcm->cm_clkmode_dpll_core, params, DPLL_NO_LOCK,
								"core");
	/* Set the ratios for CORE_CLK, L3_CLK, L4_CLK */
	temp = (CLKSEL_CORE_X2_DIV_1 << CLKSEL_CORE_SHIFT) |
	    (CLKSEL_L3_CORE_DIV_2 << CLKSEL_L3_SHIFT) |
	    (CLKSEL_L4_L3_DIV_2 << CLKSEL_L4_SHIFT);
	writel(temp, &prcm->cm_clksel_core);
	debug("Core DPLL configured\n");

	/* lock PER dpll */
	params = get_per_dpll_params();
	do_setup_dpll(&prcm->cm_clkmode_dpll_per,
			params, DPLL_LOCK, "per");
	debug("PER DPLL locked\n");

	/* MPU dpll */
	configure_mpu_dpll();

#ifdef CONFIG_USB_EHCI_OMAP
	setup_usb_dpll();
#endif
}

#ifdef CONFIG_SYS_CLOCKS_ENABLE_ALL
static void setup_non_essential_dplls(void)
{
	u32 abe_ref_clk;
	const struct dpll_params *params;

	/* IVA */
	clrsetbits_le32(&prcm->cm_bypclk_dpll_iva,
		CM_BYPCLK_DPLL_IVA_CLKSEL_MASK, DPLL_IVA_CLKSEL_CORE_X2_DIV_2);

	params = get_iva_dpll_params();
	do_setup_dpll(&prcm->cm_clkmode_dpll_iva, params, DPLL_LOCK, "iva");

	/* Configure ABE dpll */
	params = get_abe_dpll_params();
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
	abe_ref_clk = CM_ABE_PLL_REF_CLKSEL_CLKSEL_SYSCLK;
#else
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
	do_setup_dpll(&prcm->cm_clkmode_dpll_abe, params, DPLL_LOCK, "abe");
}
#endif

void do_scale_tps62361(u32 reg, u32 volt_mv)
{
	u32 temp, step;

	step = volt_mv - TPS62361_BASE_VOLT_MV;
	step /= 10;

	temp = TPS62361_I2C_SLAVE_ADDR |
	    (reg << PRM_VC_VAL_BYPASS_REGADDR_SHIFT) |
	    (step << PRM_VC_VAL_BYPASS_DATA_SHIFT) |
	    PRM_VC_VAL_BYPASS_VALID_BIT;
	debug("do_scale_tps62361: volt - %d step - 0x%x\n", volt_mv, step);

	writel(temp, &prcm->prm_vc_val_bypass);
	if (!wait_on_value(PRM_VC_VAL_BYPASS_VALID_BIT, 0,
				&prcm->prm_vc_val_bypass, LDELAY)) {
		puts("Scaling voltage failed for vdd_mpu from TPS\n");
	}
}

void do_scale_vcore(u32 vcore_reg, u32 volt_mv)
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

static inline void enable_clock_domain(u32 *const clkctrl_reg, u32 enable_mode)
{
	clrsetbits_le32(clkctrl_reg, CD_CLKCTRL_CLKTRCTRL_MASK,
			enable_mode << CD_CLKCTRL_CLKTRCTRL_SHIFT);
	debug("Enable clock domain - %p\n", clkctrl_reg);
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
	debug("Enable clock module - %p\n", clkctrl_addr);
	if (wait_for_enable)
		wait_for_clk_enable(clkctrl_addr);
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

void setup_sri2c(void)
{
	u32 sys_clk_khz, cycles_hi, cycles_low, temp;

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
}

void do_enable_clocks(u32 *const *clk_domains,
			    u32 *const *clk_modules_hw_auto,
			    u32 *const *clk_modules_explicit_en,
			    u8 wait_for_enable)
{
	u32 i, max = 100;

	/* Put the clock domains in SW_WKUP mode */
	for (i = 0; (i < max) && clk_domains[i]; i++) {
		enable_clock_domain(clk_domains[i],
				    CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	}

	/* Clock modules that need to be put in HW_AUTO */
	for (i = 0; (i < max) && clk_modules_hw_auto[i]; i++) {
		enable_clock_module(clk_modules_hw_auto[i],
				    MODULE_CLKCTRL_MODULEMODE_HW_AUTO,
				    wait_for_enable);
	};

	/* Clock modules that need to be put in SW_EXPLICIT_EN mode */
	for (i = 0; (i < max) && clk_modules_explicit_en[i]; i++) {
		enable_clock_module(clk_modules_explicit_en[i],
				    MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN,
				    wait_for_enable);
	};

	/* Put the clock domains in HW_AUTO mode now */
	for (i = 0; (i < max) && clk_domains[i]; i++) {
		enable_clock_domain(clk_domains[i],
				    CD_CLKCTRL_CLKTRCTRL_HW_AUTO);
	}
}

void prcm_init(void)
{
	switch (omap_hw_init_context()) {
	case OMAP_INIT_CONTEXT_SPL:
	case OMAP_INIT_CONTEXT_UBOOT_FROM_NOR:
	case OMAP_INIT_CONTEXT_UBOOT_AFTER_CH:
		enable_basic_clocks();
		scale_vcores();
		setup_dplls();
#ifdef CONFIG_SYS_CLOCKS_ENABLE_ALL
		setup_non_essential_dplls();
		enable_non_essential_clocks();
#endif
		break;
	default:
		break;
	}

	if (OMAP_INIT_CONTEXT_SPL != omap_hw_init_context())
		enable_basic_uboot_clocks();
}
