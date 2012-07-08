/*
 *
 * Clock initialization for OMAP5
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 * Sricharan R <r.sricharan@ti.com>
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
#include <asm/omap_gpio.h>

#ifndef CONFIG_SPL_BUILD
/*
 * printing to console doesn't work unless
 * this code is executed from SPL
 */
#define printf(fmt, args...)
#define puts(s)
#endif

struct omap5_prcm_regs *const prcm = (struct omap5_prcm_regs *)0x4A004100;

const u32 sys_clk_array[8] = {
	12000000,	       /* 12 MHz */
	0,		       /* NA */
	16800000,	       /* 16.8 MHz */
	19200000,	       /* 19.2 MHz */
	26000000,	       /* 26 MHz */
	0,		       /* NA */
	38400000,	       /* 38.4 MHz */
};

static const struct dpll_params mpu_dpll_params_1_5ghz[NUM_SYS_CLKS] = {
	{125, 0, 1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{625, 6, 1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{625, 7, 1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{750, 12, 1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{625, 15, 1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

static const struct dpll_params mpu_dpll_params_2ghz[NUM_SYS_CLKS] = {
	{500, 2, 1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{2024, 16, 1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{625, 5, 1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{1000, 12, 1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{625, 11, 1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

static const struct dpll_params mpu_dpll_params_1100mhz[NUM_SYS_CLKS] = {
	{275, 2, 1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1375, 20, 1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{1375, 23, 1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{550, 12, 1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{1375, 47, 1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

static const struct dpll_params mpu_dpll_params_800mhz[NUM_SYS_CLKS] = {
	{200, 2, 1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1000, 20, 1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{375, 8, 1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{400, 12, 1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{375, 17, 1, -1, -1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params mpu_dpll_params_400mhz[NUM_SYS_CLKS] = {
	{200, 2, 2, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1000, 20, 2, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{375, 8, 2, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{400, 12, 2, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{375, 17, 2, -1, -1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params mpu_dpll_params_550mhz[NUM_SYS_CLKS] = {
	{275, 2, 2, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1375, 20, 2, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{1375, 23, 2, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{550, 12, 2, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{1375, 47, 2, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr532[NUM_SYS_CLKS] = {
	{266, 2, 2, 5, 8, 4, 62, 5, 5, 7},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{570, 8, 2, 5, 8, 4, 62, 5, 5, 7},		/* 16.8 MHz */
	{665, 11, 2, 5, 8, 4, 62, 5, 5, 7},		/* 19.2 MHz */
	{532, 12, 2, 5, 8, 4, 62, 5, 5, 7},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{665, 23, 2, 5, 8, 4, 62, 5, 5, 7}		/* 38.4 MHz */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr266[NUM_SYS_CLKS] = {
	{266, 2, 4, 5, 8, 8, 62, 10, 10, 14},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{570, 8, 4, 5, 8, 8, 62, 10, 10, 14},		/* 16.8 MHz */
	{665, 11, 4, 5, 8, 8, 62, 10, 10, 14},		/* 19.2 MHz */
	{532, 12, 4, 8, 8, 8, 62, 10, 10, 14},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{665, 23, 4, 8, 8, 8, 62, 10, 10, 14}		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz[NUM_SYS_CLKS] = {
	{32, 0, 4, 3, 6, 4, -1, 2, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{160, 6, 4, 3, 6, 4, -1, 2, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, 3, 6, 4, -1, 2, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, 3, 6, 4, -1, 2, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, 3, 6, 4, -1, 2, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params iva_dpll_params_2330mhz[NUM_SYS_CLKS] = {
	{1165, 11, -1, -1, 5, 6, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{2011, 28, -1, -1, 5, 6, -1, -1, -1, -1},	/* 16.8 MHz */
	{1881, 30, -1, -1, 5, 6, -1, -1, -1, -1},	/* 19.2 MHz */
	{1165, 25, -1, -1, 5, 6, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{1972, 64, -1, -1, 5, 6, -1, -1, -1, -1}	/* 38.4 MHz */
};

/* ABE M & N values with sys_clk as source */
static const struct dpll_params
		abe_dpll_params_sysclk_196608khz[NUM_SYS_CLKS] = {
	{49, 5, 1, -1, -1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{35, 5, 1, 1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{46, 8, 1, 1, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{34, 8, 1, 1, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{64, 24, 1, 1, -1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

/* ABE M & N values with 32K clock as source */
static const struct dpll_params abe_dpll_params_32k_196608khz = {
	750, 0, 1, 1, -1, -1, -1, -1, -1, -1
};

static const struct dpll_params usb_dpll_params_1920mhz[NUM_SYS_CLKS] = {
	{400, 4, 2, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{400, 6, 2, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{400, 7, 2, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{480, 12, 2, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{400, 15, 2, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

void setup_post_dividers(u32 *const base, const struct dpll_params *params)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	/* Setup post-dividers */
	if (params->m2 >= 0)
		writel(params->m2, &dpll_regs->cm_div_m2_dpll);
	if (params->m3 >= 0)
		writel(params->m3, &dpll_regs->cm_div_m3_dpll);
	if (params->h11 >= 0)
		writel(params->h11, &dpll_regs->cm_div_h11_dpll);
	if (params->h12 >= 0)
		writel(params->h12, &dpll_regs->cm_div_h12_dpll);
	if (params->h13 >= 0)
		writel(params->h13, &dpll_regs->cm_div_h13_dpll);
	if (params->h14 >= 0)
		writel(params->h14, &dpll_regs->cm_div_h14_dpll);
	if (params->h22 >= 0)
		writel(params->h22, &dpll_regs->cm_div_h22_dpll);
	if (params->h23 >= 0)
		writel(params->h23, &dpll_regs->cm_div_h23_dpll);
}

const struct dpll_params *get_mpu_dpll_params(void)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &mpu_dpll_params_800mhz[sysclk_ind];
}

const struct dpll_params *get_core_dpll_params(void)
{
	u32 sysclk_ind = get_sys_clk_index();

	/* Configuring the DDR to be at 532mhz */
	return &core_dpll_params_2128mhz_ddr532[sysclk_ind];
}

const struct dpll_params *get_per_dpll_params(void)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &per_dpll_params_768mhz[sysclk_ind];
}

const struct dpll_params *get_iva_dpll_params(void)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &iva_dpll_params_2330mhz[sysclk_ind];
}

const struct dpll_params *get_usb_dpll_params(void)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &usb_dpll_params_1920mhz[sysclk_ind];
}

const struct dpll_params *get_abe_dpll_params(void)
{
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
	u32 sysclk_ind = get_sys_clk_index();
	return &abe_dpll_params_sysclk_196608khz[sysclk_ind];
#else
	return &abe_dpll_params_32k_196608khz;
#endif
}

/*
 * Setup the voltages for vdd_mpu, vdd_core, and vdd_iva
 * We set the maximum voltages allowed here because Smart-Reflex is not
 * enabled in bootloader. Voltage initialization in the kernel will set
 * these to the nominal values after enabling Smart-Reflex
 */
void scale_vcores(void)
{
	u32 volt_core, volt_mpu, volt_mm;

	omap_vc_init(PRM_VC_I2C_CHANNEL_FREQ_KHZ);

	/* Palmas settings */
	if (omap_revision() != OMAP5432_ES1_0) {
		volt_core = VDD_CORE;
		volt_mpu = VDD_MPU;
		volt_mm = VDD_MM;
	} else {
		volt_core = VDD_CORE_5432;
		volt_mpu = VDD_MPU_5432;
		volt_mm = VDD_MM_5432;
	}

	do_scale_vcore(SMPS_REG_ADDR_8_CORE, volt_core);
	do_scale_vcore(SMPS_REG_ADDR_12_MPU, volt_mpu);
	do_scale_vcore(SMPS_REG_ADDR_45_IVA, volt_mm);

	if (omap_revision() == OMAP5432_ES1_0) {
		/* Configure LDO SRAM "magic" bits */
		writel(2, &prcm->prm_sldo_core_setup);
		writel(2, &prcm->prm_sldo_mpu_setup);
		writel(2, &prcm->prm_sldo_mm_setup);
	}
}

u32 get_offset_code(u32 volt_offset)
{
	u32 offset_code, step = 10000; /* 10 mV represented in uV */

	volt_offset -= PALMAS_SMPS_BASE_VOLT_UV;

	offset_code = (volt_offset + step - 1) / step;

	/*
	 * Offset codes 1-6 all give the base voltage in Palmas
	 * Offset code 0 switches OFF the SMPS
	 */
	return offset_code + 6;
}

/*
 * Enable essential clock domains, modules and
 * do some additional special settings needed
 */
void enable_basic_clocks(void)
{
	u32 *const clk_domains_essential[] = {
		&prcm->cm_l4per_clkstctrl,
		&prcm->cm_l3init_clkstctrl,
		&prcm->cm_memif_clkstctrl,
		&prcm->cm_l4cfg_clkstctrl,
		0
	};

	u32 *const clk_modules_hw_auto_essential[] = {
		&prcm->cm_l3_2_gpmc_clkctrl,
		&prcm->cm_memif_emif_1_clkctrl,
		&prcm->cm_memif_emif_2_clkctrl,
		&prcm->cm_l4cfg_l4_cfg_clkctrl,
		&prcm->cm_wkup_gpio1_clkctrl,
		&prcm->cm_l4per_gpio2_clkctrl,
		&prcm->cm_l4per_gpio3_clkctrl,
		&prcm->cm_l4per_gpio4_clkctrl,
		&prcm->cm_l4per_gpio5_clkctrl,
		&prcm->cm_l4per_gpio6_clkctrl,
		0
	};

	u32 *const clk_modules_explicit_en_essential[] = {
		&prcm->cm_wkup_gptimer1_clkctrl,
		&prcm->cm_l3init_hsmmc1_clkctrl,
		&prcm->cm_l3init_hsmmc2_clkctrl,
		&prcm->cm_l4per_gptimer2_clkctrl,
		&prcm->cm_wkup_wdtimer2_clkctrl,
		&prcm->cm_l4per_uart3_clkctrl,
		&prcm->cm_l4per_i2c1_clkctrl,
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

	/* Set the correct clock dividers for mmc */
	setbits_le32(&prcm->cm_l3init_hsmmc1_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_DIV_MASK);
	setbits_le32(&prcm->cm_l3init_hsmmc2_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_DIV_MASK);

	/* Select 32KHz clock as the source of GPTIMER1 */
	setbits_le32(&prcm->cm_wkup_gptimer1_clkctrl,
			GPTIMER1_CLKCTRL_CLKSEL_MASK);

	do_enable_clocks(clk_domains_essential,
			 clk_modules_hw_auto_essential,
			 clk_modules_explicit_en_essential,
			 1);

	/* Select 384Mhz for GPU as its the POR for ES1.0 */
	setbits_le32(&prcm->cm_sgx_sgx_clkctrl,
			CLKSEL_GPU_HYD_GCLK_MASK);
	setbits_le32(&prcm->cm_sgx_sgx_clkctrl,
			CLKSEL_GPU_CORE_GCLK_MASK);

	/* Enable SCRM OPT clocks for PER and CORE dpll */
	setbits_le32(&prcm->cm_wkupaon_scrm_clkctrl,
			OPTFCLKEN_SCRM_PER_MASK);
	setbits_le32(&prcm->cm_wkupaon_scrm_clkctrl,
			OPTFCLKEN_SCRM_CORE_MASK);
}

void enable_basic_uboot_clocks(void)
{
	u32 *const clk_domains_essential[] = {
		0
	};

	u32 *const clk_modules_hw_auto_essential[] = {
		0
	};

	u32 *const clk_modules_explicit_en_essential[] = {
		&prcm->cm_l4per_mcspi1_clkctrl,
		&prcm->cm_l4per_i2c2_clkctrl,
		&prcm->cm_l4per_i2c3_clkctrl,
		&prcm->cm_l4per_i2c4_clkctrl,
		&prcm->cm_l3init_hsusbtll_clkctrl,
		&prcm->cm_l3init_hsusbhost_clkctrl,
		&prcm->cm_l3init_fsusb_clkctrl,
		0
	};

	do_enable_clocks(clk_domains_essential,
			 clk_modules_hw_auto_essential,
			 clk_modules_explicit_en_essential,
			 1);
}

/*
 * Enable non-essential clock domains, modules and
 * do some additional special settings needed
 */
void enable_non_essential_clocks(void)
{
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
		&prcm->cm_l3instr_l3_3_clkctrl,
		&prcm->cm_l3instr_l3_instr_clkctrl,
		&prcm->cm_l3instr_intrconn_wp1_clkctrl,
		&prcm->cm_l3init_hsi_clkctrl,
		&prcm->cm_l4per_hdq1w_clkctrl,
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
		0
	};

	/* Enable optional functional clock for ISS */
	setbits_le32(&prcm->cm_cam_iss_clkctrl, ISS_CLKCTRL_OPTFCLKEN_MASK);

	/* Enable all optional functional clocks of DSS */
	setbits_le32(&prcm->cm_dss_dss_clkctrl, DSS_CLKCTRL_OPTFCLKEN_MASK);

	do_enable_clocks(clk_domains_non_essential,
			 clk_modules_hw_auto_non_essential,
			 clk_modules_explicit_en_non_essential,
			 0);

	/* Put camera module in no sleep mode */
	clrsetbits_le32(&prcm->cm_cam_clkstctrl, MODULE_CLKCTRL_MODULEMODE_MASK,
			CD_CLKCTRL_CLKTRCTRL_NO_SLEEP <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
}
