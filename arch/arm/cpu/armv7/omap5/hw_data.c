/*
 *
 * HW data initialization for OMAP5
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Sricharan R <r.sricharan@ti.com>
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
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>
#include <asm/arch/clocks.h>
#include <asm/omap_gpio.h>
#include <asm/io.h>
#include <asm/emif.h>

struct prcm_regs const **prcm =
			(struct prcm_regs const **) OMAP_SRAM_SCRATCH_PRCM_PTR;
struct dplls const **dplls_data =
			(struct dplls const **) OMAP_SRAM_SCRATCH_DPLLS_PTR;
struct vcores_data const **omap_vcores =
		(struct vcores_data const **) OMAP_SRAM_SCRATCH_VCORES_PTR;
struct omap_sys_ctrl_regs const **ctrl =
	(struct omap_sys_ctrl_regs const **)OMAP_SRAM_SCRATCH_SYS_CTRL;

/* OPP HIGH FREQUENCY for ES2.0 */
static const struct dpll_params mpu_dpll_params_1_5ghz[NUM_SYS_CLKS] = {
	{125, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{625, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{625, 7, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{750, 12, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{625, 15, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

/* OPP NOM FREQUENCY for ES2.0, OPP HIGH for ES1.0 */
static const struct dpll_params mpu_dpll_params_1100mhz[NUM_SYS_CLKS] = {
	{275, 2, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1375, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{1375, 23, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{550, 12, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{1375, 47, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

/* OPP NOM FREQUENCY for ES1.0 */
static const struct dpll_params mpu_dpll_params_800mhz[NUM_SYS_CLKS] = {
	{200, 2, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1000, 20, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{375, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{400, 12, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{375, 17, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

/* OPP LOW FREQUENCY for ES1.0 */
static const struct dpll_params mpu_dpll_params_400mhz[NUM_SYS_CLKS] = {
	{200, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{1000, 20, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{375, 8, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{400, 12, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{375, 17, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

/* OPP LOW FREQUENCY for ES2.0 */
static const struct dpll_params mpu_dpll_params_499mhz[NUM_SYS_CLKS] = {
	{499, 11, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{297, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{493, 18, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{499, 25, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{493, 37, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1}	/* 38.4 MHz */
};

static const struct dpll_params mpu_dpll_params_1ghz[NUM_SYS_CLKS] = {
	{250, 2, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{119, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{625, 11, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{500, 12, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{625, 23, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 38.4 MHz */
	{50, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1}		/* 20 MHz   */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr532[NUM_SYS_CLKS] = {
	{266, 2, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{443, 6, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 16.8 MHz */
	{277, 4, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 19.2 MHz */
	{368, 8, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 2, 5, 8, 4, 62, 5, -1, 5, 7, -1}		/* 38.4 MHz */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr532_es2[NUM_SYS_CLKS] = {
	{266, 2, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{443, 6, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 16.8 MHz */
	{277, 4, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 19.2 MHz */
	{368, 8, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 2, 5, 8, 4, 62, 63, 6, 5, 7, 6}		/* 38.4 MHz */
};

static const struct dpll_params
		core_dpll_params_2128mhz_ddr532_dra7xx[NUM_SYS_CLKS] = {
	{266, 2, 2, -1, -1, 4, 62, 5, -1, 5, 7, 6},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{443, 6, 2, -1, -1, 4, 62, 5, -1, 5, 7, 6},		/* 16.8 MHz */
	{277, 4, 2, -1, -1, 4, 62, 5, -1, 5, 7, 6},		/* 19.2 MHz */
	{368, 8, 2, -1, -1, 4, 62, 5, -1, 5, 7, 6},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 2, -1, -1, 4, 62, 5, -1, 5, 7, 6},		/* 38.4 MHz */
	{266, 4, 2, -1, -1, 4, 62, 5, -1, 5, 7, 6}		/* 20 MHz   */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr266[NUM_SYS_CLKS] = {
	{266, 2, 4, 8, 8, 8, 62, 10, -1, 10, 14, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{443, 6, 4, 8, 8, 8, 62, 10, -1, 10, 14, -1},		/* 16.8 MHz */
	{277, 4, 4, 8, 8, 8, 62, 10, -1, 10, 14, -1},		/* 19.2 MHz */
	{368, 8, 4, 8, 8, 8, 62, 10, -1, 10, 14, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 4, 8, 8, 8, 62, 10, -1, 10, 14, -1}		/* 38.4 MHz */
};

static const struct dpll_params
			core_dpll_params_2128mhz_ddr266_es2[NUM_SYS_CLKS] = {
	{266, 2, 4, 8, 8, 8, 62, 5, 12, 10, 14, 12},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{443, 6, 4, 8, 8, 8, 62, 5, 12, 10, 14, 12},		/* 16.8 MHz */
	{277, 4, 4, 8, 8, 8, 62, 5, 12, 10, 14, 12},		/* 19.2 MHz */
	{368, 8, 4, 8, 8, 8, 62, 5, 12, 10, 14, 12},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{277, 9, 4, 8, 8, 8, 62, 5, 12, 10, 14, 12}		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz[NUM_SYS_CLKS] = {
	{32, 0, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{160, 6, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, 3, 6, 4, -1, 2, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz_es2[NUM_SYS_CLKS] = {
	{32, 0, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{160, 6, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, 3, 3, 4, -1, 2, -1, -1, -1, -1}		/* 38.4 MHz */
};

static const struct dpll_params per_dpll_params_768mhz_dra7xx[NUM_SYS_CLKS] = {
	{32, 0, 4, -1, 3, 4, 10, 2, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{160, 6, 4, -1, 3, 4, 10, 2, -1, -1, -1, -1},		/* 16.8 MHz */
	{20, 0, 4, -1, 3, 4, 10, 2, -1, -1, -1, -1},		/* 19.2 MHz */
	{192, 12, 4, -1, 3, 4, 10, 2, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{10, 0, 4, -1, 3, 4, 10, 2, -1, -1, -1, -1},		/* 38.4 MHz */
	{96, 4, 4, -1, 3, 4, 10, 2, -1, -1, -1, -1}		/* 20 MHz   */
};

static const struct dpll_params iva_dpll_params_2330mhz[NUM_SYS_CLKS] = {
	{1165, 11, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{208, 2, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{182, 2, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{224, 4, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{91, 2, -1, -1, 5, 6, -1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

/* ABE M & N values with sys_clk as source */
static const struct dpll_params
		abe_dpll_params_sysclk_196608khz[NUM_SYS_CLKS] = {
	{49, 5, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{35, 5, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{46, 8, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{34, 8, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{64, 24, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1}		/* 38.4 MHz */
};

/* ABE M & N values with 32K clock as source */
static const struct dpll_params abe_dpll_params_32k_196608khz = {
	750, 0, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1
};

static const struct dpll_params usb_dpll_params_1920mhz[NUM_SYS_CLKS] = {
	{400, 4, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{400, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 16.8 MHz */
	{400, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 19.2 MHz */
	{480, 12, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{400, 15, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 38.4 MHz */
	{48, 0, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1}		/* 20 MHz   */
};

static const struct dpll_params ddr_dpll_params_1066mhz[NUM_SYS_CLKS] = {
	{533, 11, 1, 1, 4, -1, -1, -1, -1, -1, -1, -1},		/* 12 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 13 MHz   */
	{222, 6, 1, 1, 4, -1, -1, -1, -1, -1, -1, -1},		/* 16.8 MHz */
	{111, 3, 1, 1, 4, -1, -1, -1, -1, -1, -1, -1},		/* 19.2 MHz */
	{41, 1, 1, 1, 4, -1, -1, -1, -1, -1, -1, -1},		/* 26 MHz   */
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	/* 27 MHz   */
	{347, 24, 1, 1, 4, -1, -1, -1, -1, -1, -1, -1},		/* 38.4 MHz */
	{533, 19, 1, 1, 4, -1, -1, -1, -1, -1, -1, -1}		/* 20 MHz   */
};

struct dplls omap5_dplls_es1 = {
	.mpu = mpu_dpll_params_800mhz,
	.core = core_dpll_params_2128mhz_ddr532,
	.per = per_dpll_params_768mhz,
	.iva = iva_dpll_params_2330mhz,
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
	.abe = abe_dpll_params_sysclk_196608khz,
#else
	.abe = &abe_dpll_params_32k_196608khz,
#endif
	.usb = usb_dpll_params_1920mhz,
	.ddr = NULL
};

struct dplls omap5_dplls_es2 = {
	.mpu = mpu_dpll_params_1100mhz,
	.core = core_dpll_params_2128mhz_ddr532_es2,
	.per = per_dpll_params_768mhz_es2,
	.iva = iva_dpll_params_2330mhz,
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
	.abe = abe_dpll_params_sysclk_196608khz,
#else
	.abe = &abe_dpll_params_32k_196608khz,
#endif
	.usb = usb_dpll_params_1920mhz,
	.ddr = NULL
};

struct dplls dra7xx_dplls = {
	.mpu = mpu_dpll_params_1ghz,
	.core = core_dpll_params_2128mhz_ddr532_dra7xx,
	.per = per_dpll_params_768mhz_dra7xx,
	.usb = usb_dpll_params_1920mhz,
	.ddr = ddr_dpll_params_1066mhz,
};

struct pmic_data palmas = {
	.base_offset = PALMAS_SMPS_BASE_VOLT_UV,
	.step = 10000, /* 10 mV represented in uV */
	/*
	 * Offset codes 1-6 all give the base voltage in Palmas
	 * Offset code 0 switches OFF the SMPS
	 */
	.start_code = 6,
};

struct vcores_data omap5430_volts = {
	.mpu.value = VDD_MPU,
	.mpu.addr = SMPS_REG_ADDR_12_MPU,
	.mpu.pmic = &palmas,

	.core.value = VDD_CORE,
	.core.addr = SMPS_REG_ADDR_8_CORE,
	.core.pmic = &palmas,

	.mm.value = VDD_MM,
	.mm.addr = SMPS_REG_ADDR_45_IVA,
	.mm.pmic = &palmas,
};

struct vcores_data omap5430_volts_es2 = {
	.mpu.value = VDD_MPU_ES2,
	.mpu.addr = SMPS_REG_ADDR_12_MPU,
	.mpu.pmic = &palmas,

	.core.value = VDD_CORE_ES2,
	.core.addr = SMPS_REG_ADDR_8_CORE,
	.core.pmic = &palmas,

	.mm.value = VDD_MM_ES2,
	.mm.addr = SMPS_REG_ADDR_45_IVA,
	.mm.pmic = &palmas,
};

/*
 * Enable essential clock domains, modules and
 * do some additional special settings needed
 */
void enable_basic_clocks(void)
{
	u32 const clk_domains_essential[] = {
		(*prcm)->cm_l4per_clkstctrl,
		(*prcm)->cm_l3init_clkstctrl,
		(*prcm)->cm_memif_clkstctrl,
		(*prcm)->cm_l4cfg_clkstctrl,
		0
	};

	u32 const clk_modules_hw_auto_essential[] = {
		(*prcm)->cm_l3_gpmc_clkctrl,
		(*prcm)->cm_memif_emif_1_clkctrl,
		(*prcm)->cm_memif_emif_2_clkctrl,
		(*prcm)->cm_l4cfg_l4_cfg_clkctrl,
		(*prcm)->cm_wkup_gpio1_clkctrl,
		(*prcm)->cm_l4per_gpio2_clkctrl,
		(*prcm)->cm_l4per_gpio3_clkctrl,
		(*prcm)->cm_l4per_gpio4_clkctrl,
		(*prcm)->cm_l4per_gpio5_clkctrl,
		(*prcm)->cm_l4per_gpio6_clkctrl,
		0
	};

	u32 const clk_modules_explicit_en_essential[] = {
		(*prcm)->cm_wkup_gptimer1_clkctrl,
		(*prcm)->cm_l3init_hsmmc1_clkctrl,
		(*prcm)->cm_l3init_hsmmc2_clkctrl,
		(*prcm)->cm_l4per_gptimer2_clkctrl,
		(*prcm)->cm_wkup_wdtimer2_clkctrl,
		(*prcm)->cm_l4per_uart3_clkctrl,
		(*prcm)->cm_l4per_i2c1_clkctrl,
		0
	};

	/* Enable optional additional functional clock for GPIO4 */
	setbits_le32((*prcm)->cm_l4per_gpio4_clkctrl,
			GPIO4_CLKCTRL_OPTFCLKEN_MASK);

	/* Enable 96 MHz clock for MMC1 & MMC2 */
	setbits_le32((*prcm)->cm_l3init_hsmmc1_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_MASK);
	setbits_le32((*prcm)->cm_l3init_hsmmc2_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_MASK);

	/* Set the correct clock dividers for mmc */
	setbits_le32((*prcm)->cm_l3init_hsmmc1_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_DIV_MASK);
	setbits_le32((*prcm)->cm_l3init_hsmmc2_clkctrl,
			HSMMC_CLKCTRL_CLKSEL_DIV_MASK);

	/* Select 32KHz clock as the source of GPTIMER1 */
	setbits_le32((*prcm)->cm_wkup_gptimer1_clkctrl,
			GPTIMER1_CLKCTRL_CLKSEL_MASK);

	do_enable_clocks(clk_domains_essential,
			 clk_modules_hw_auto_essential,
			 clk_modules_explicit_en_essential,
			 1);

	/* Enable SCRM OPT clocks for PER and CORE dpll */
	setbits_le32((*prcm)->cm_wkupaon_scrm_clkctrl,
			OPTFCLKEN_SCRM_PER_MASK);
	setbits_le32((*prcm)->cm_wkupaon_scrm_clkctrl,
			OPTFCLKEN_SCRM_CORE_MASK);
}

void enable_basic_uboot_clocks(void)
{
	u32 const clk_domains_essential[] = {
		0
	};

	u32 const clk_modules_hw_auto_essential[] = {
		(*prcm)->cm_l3init_hsusbtll_clkctrl,
		0
	};

	u32 const clk_modules_explicit_en_essential[] = {
		(*prcm)->cm_l4per_mcspi1_clkctrl,
		(*prcm)->cm_l4per_i2c2_clkctrl,
		(*prcm)->cm_l4per_i2c3_clkctrl,
		(*prcm)->cm_l4per_i2c4_clkctrl,
		(*prcm)->cm_l4per_i2c5_clkctrl,
		(*prcm)->cm_l3init_hsusbhost_clkctrl,
		(*prcm)->cm_l3init_fsusb_clkctrl,
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
	u32 const clk_domains_non_essential[] = {
		(*prcm)->cm_mpu_m3_clkstctrl,
		(*prcm)->cm_ivahd_clkstctrl,
		(*prcm)->cm_dsp_clkstctrl,
		(*prcm)->cm_dss_clkstctrl,
		(*prcm)->cm_sgx_clkstctrl,
		(*prcm)->cm1_abe_clkstctrl,
		(*prcm)->cm_c2c_clkstctrl,
		(*prcm)->cm_cam_clkstctrl,
		(*prcm)->cm_dss_clkstctrl,
		(*prcm)->cm_sdma_clkstctrl,
		0
	};

	u32 const clk_modules_hw_auto_non_essential[] = {
		(*prcm)->cm_mpu_m3_mpu_m3_clkctrl,
		(*prcm)->cm_ivahd_ivahd_clkctrl,
		(*prcm)->cm_ivahd_sl2_clkctrl,
		(*prcm)->cm_dsp_dsp_clkctrl,
		(*prcm)->cm_l3instr_l3_3_clkctrl,
		(*prcm)->cm_l3instr_l3_instr_clkctrl,
		(*prcm)->cm_l3instr_intrconn_wp1_clkctrl,
		(*prcm)->cm_l3init_hsi_clkctrl,
		(*prcm)->cm_l4per_hdq1w_clkctrl,
		0
	};

	u32 const clk_modules_explicit_en_non_essential[] = {
		(*prcm)->cm1_abe_aess_clkctrl,
		(*prcm)->cm1_abe_pdm_clkctrl,
		(*prcm)->cm1_abe_dmic_clkctrl,
		(*prcm)->cm1_abe_mcasp_clkctrl,
		(*prcm)->cm1_abe_mcbsp1_clkctrl,
		(*prcm)->cm1_abe_mcbsp2_clkctrl,
		(*prcm)->cm1_abe_mcbsp3_clkctrl,
		(*prcm)->cm1_abe_slimbus_clkctrl,
		(*prcm)->cm1_abe_timer5_clkctrl,
		(*prcm)->cm1_abe_timer6_clkctrl,
		(*prcm)->cm1_abe_timer7_clkctrl,
		(*prcm)->cm1_abe_timer8_clkctrl,
		(*prcm)->cm1_abe_wdt3_clkctrl,
		(*prcm)->cm_l4per_gptimer9_clkctrl,
		(*prcm)->cm_l4per_gptimer10_clkctrl,
		(*prcm)->cm_l4per_gptimer11_clkctrl,
		(*prcm)->cm_l4per_gptimer3_clkctrl,
		(*prcm)->cm_l4per_gptimer4_clkctrl,
		(*prcm)->cm_l4per_mcspi2_clkctrl,
		(*prcm)->cm_l4per_mcspi3_clkctrl,
		(*prcm)->cm_l4per_mcspi4_clkctrl,
		(*prcm)->cm_l4per_mmcsd3_clkctrl,
		(*prcm)->cm_l4per_mmcsd4_clkctrl,
		(*prcm)->cm_l4per_mmcsd5_clkctrl,
		(*prcm)->cm_l4per_uart1_clkctrl,
		(*prcm)->cm_l4per_uart2_clkctrl,
		(*prcm)->cm_l4per_uart4_clkctrl,
		(*prcm)->cm_wkup_keyboard_clkctrl,
		(*prcm)->cm_wkup_wdtimer2_clkctrl,
		(*prcm)->cm_cam_iss_clkctrl,
		(*prcm)->cm_cam_fdif_clkctrl,
		(*prcm)->cm_dss_dss_clkctrl,
		(*prcm)->cm_sgx_sgx_clkctrl,
		0
	};

	/* Enable optional functional clock for ISS */
	setbits_le32((*prcm)->cm_cam_iss_clkctrl, ISS_CLKCTRL_OPTFCLKEN_MASK);

	/* Enable all optional functional clocks of DSS */
	setbits_le32((*prcm)->cm_dss_dss_clkctrl, DSS_CLKCTRL_OPTFCLKEN_MASK);

	do_enable_clocks(clk_domains_non_essential,
			 clk_modules_hw_auto_non_essential,
			 clk_modules_explicit_en_non_essential,
			 0);

	/* Put camera module in no sleep mode */
	clrsetbits_le32((*prcm)->cm_cam_clkstctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			CD_CLKCTRL_CLKTRCTRL_NO_SLEEP <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
}

const struct ctrl_ioregs ioregs_omap5430 = {
	.ctrl_ddrch = DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN,
	.ctrl_lpddr2ch = DDR_IO_I_34OHM_SR_FASTEST_WD_CK_CKE_NCS_CA_PULL_DOWN,
	.ctrl_ddrio_0 = DDR_IO_0_DDR2_DQ_INT_EN_ALL_DDR3_CA_DIS_ALL,
	.ctrl_ddrio_1 = DDR_IO_1_DQ_OUT_EN_ALL_DQ_INT_EN_ALL,
	.ctrl_ddrio_2 = DDR_IO_2_CA_OUT_EN_ALL_CA_INT_EN_ALL,
};

const struct ctrl_ioregs ioregs_omap5432_es1 = {
	.ctrl_ddrch = DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL,
	.ctrl_lpddr2ch = 0x0,
	.ctrl_ddr3ch = DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL,
	.ctrl_ddrio_0 = DDR_IO_0_VREF_CELLS_DDR3_VALUE,
	.ctrl_ddrio_1 = DDR_IO_1_VREF_CELLS_DDR3_VALUE,
	.ctrl_ddrio_2 = DDR_IO_2_VREF_CELLS_DDR3_VALUE,
	.ctrl_emif_sdram_config_ext = SDRAM_CONFIG_EXT_RD_LVL_11_SAMPLES,
};

const struct ctrl_ioregs ioregs_omap5432_es2 = {
	.ctrl_ddrch = DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL_ES2,
	.ctrl_lpddr2ch = 0x0,
	.ctrl_ddr3ch = DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL_ES2,
	.ctrl_ddrio_0 = DDR_IO_0_VREF_CELLS_DDR3_VALUE_ES2,
	.ctrl_ddrio_1 = DDR_IO_1_VREF_CELLS_DDR3_VALUE_ES2,
	.ctrl_ddrio_2 = DDR_IO_2_VREF_CELLS_DDR3_VALUE_ES2,
	.ctrl_emif_sdram_config_ext = SDRAM_CONFIG_EXT_RD_LVL_11_SAMPLES,
};

void hw_data_init(void)
{
	u32 omap_rev = omap_revision();

	switch (omap_rev) {

	case OMAP5430_ES1_0:
	case OMAP5432_ES1_0:
	*prcm = &omap5_es1_prcm;
	*dplls_data = &omap5_dplls_es1;
	*omap_vcores = &omap5430_volts;
	*ctrl = &omap5_ctrl;
	break;

	case OMAP5430_ES2_0:
	case OMAP5432_ES2_0:
	*prcm = &omap5_es2_prcm;
	*dplls_data = &omap5_dplls_es2;
	*omap_vcores = &omap5430_volts_es2;
	*ctrl = &omap5_ctrl;
	break;

	case DRA752_ES1_0:
	*prcm = &dra7xx_prcm;
	*dplls_data = &dra7xx_dplls;
	*omap_vcores = &omap5430_volts_es2;
	*ctrl = &dra7xx_ctrl;
	break;

	default:
		printf("\n INVALID OMAP REVISION ");
	}
}

void get_ioregs(const struct ctrl_ioregs **regs)
{
	u32 omap_rev = omap_revision();

	switch (omap_rev) {
	case OMAP5430_ES1_0:
	case OMAP5430_ES2_0:
		*regs = &ioregs_omap5430;
	break;
	case OMAP5432_ES1_0:
		*regs = &ioregs_omap5432_es1;
	break;
	case OMAP5432_ES2_0:
	case DRA752_ES1_0:
		*regs = &ioregs_omap5432_es2;
	break;

	default:
		printf("\n INVALID OMAP REVISION ");
	}
}
