/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 *	Aneesh V <aneesh@ti.com>
 *	Sricharan R <r.sricharan@ti.com>
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
#ifndef _CLOCKS_OMAP5_H_
#define _CLOCKS_OMAP5_H_
#include <common.h>
#include <asm/omap_common.h>

/*
 * Assuming a maximum of 1.5 GHz ARM speed and a minimum of 2 cycles per
 * loop, allow for a minimum of 2 ms wait (in reality the wait will be
 * much more than that)
 */
#define LDELAY		1000000

/* CM_DLL_CTRL */
#define CM_DLL_CTRL_OVERRIDE_SHIFT		0
#define CM_DLL_CTRL_OVERRIDE_MASK		(1 << 0)
#define CM_DLL_CTRL_NO_OVERRIDE			0

/* CM_CLKMODE_DPLL */
#define CM_CLKMODE_DPLL_REGM4XEN_SHIFT		11
#define CM_CLKMODE_DPLL_REGM4XEN_MASK		(1 << 11)
#define CM_CLKMODE_DPLL_LPMODE_EN_SHIFT		10
#define CM_CLKMODE_DPLL_LPMODE_EN_MASK		(1 << 10)
#define CM_CLKMODE_DPLL_RELOCK_RAMP_EN_SHIFT	9
#define CM_CLKMODE_DPLL_RELOCK_RAMP_EN_MASK	(1 << 9)
#define CM_CLKMODE_DPLL_DRIFTGUARD_EN_SHIFT	8
#define CM_CLKMODE_DPLL_DRIFTGUARD_EN_MASK	(1 << 8)
#define CM_CLKMODE_DPLL_RAMP_RATE_SHIFT		5
#define CM_CLKMODE_DPLL_RAMP_RATE_MASK		(0x7 << 5)
#define CM_CLKMODE_DPLL_EN_SHIFT		0
#define CM_CLKMODE_DPLL_EN_MASK			(0x7 << 0)

#define CM_CLKMODE_DPLL_DPLL_EN_SHIFT		0
#define CM_CLKMODE_DPLL_DPLL_EN_MASK		7

#define DPLL_EN_STOP			1
#define DPLL_EN_MN_BYPASS		4
#define DPLL_EN_LOW_POWER_BYPASS	5
#define DPLL_EN_FAST_RELOCK_BYPASS	6
#define DPLL_EN_LOCK			7

/* CM_IDLEST_DPLL fields */
#define ST_DPLL_CLK_MASK		1

/* SGX */
#define CLKSEL_GPU_HYD_GCLK_MASK		(1 << 25)
#define CLKSEL_GPU_CORE_GCLK_MASK		(1 << 24)

/* CM_CLKSEL_DPLL */
#define CM_CLKSEL_DPLL_DPLL_SD_DIV_SHIFT	24
#define CM_CLKSEL_DPLL_DPLL_SD_DIV_MASK		(0xFF << 24)
#define CM_CLKSEL_DPLL_M_SHIFT			8
#define CM_CLKSEL_DPLL_M_MASK			(0x7FF << 8)
#define CM_CLKSEL_DPLL_N_SHIFT			0
#define CM_CLKSEL_DPLL_N_MASK			0x7F
#define CM_CLKSEL_DCC_EN_SHIFT			22
#define CM_CLKSEL_DCC_EN_MASK			(1 << 22)

/* CM_SYS_CLKSEL */
#define CM_SYS_CLKSEL_SYS_CLKSEL_MASK	7

/* CM_CLKSEL_CORE */
#define CLKSEL_CORE_SHIFT	0
#define CLKSEL_L3_SHIFT		4
#define CLKSEL_L4_SHIFT		8

#define CLKSEL_CORE_X2_DIV_1	0
#define CLKSEL_L3_CORE_DIV_2	1
#define CLKSEL_L4_L3_DIV_2	1

/* CM_ABE_PLL_REF_CLKSEL */
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_SHIFT	0
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_MASK	1
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_SYSCLK	0
#define CM_ABE_PLL_REF_CLKSEL_CLKSEL_32KCLK	1

/* CM_BYPCLK_DPLL_IVA */
#define CM_BYPCLK_DPLL_IVA_CLKSEL_SHIFT		0
#define CM_BYPCLK_DPLL_IVA_CLKSEL_MASK		3

#define DPLL_IVA_CLKSEL_CORE_X2_DIV_2		1

/* CM_SHADOW_FREQ_CONFIG1 */
#define SHADOW_FREQ_CONFIG1_FREQ_UPDATE_MASK	1
#define SHADOW_FREQ_CONFIG1_DLL_OVERRIDE_MASK	4
#define SHADOW_FREQ_CONFIG1_DLL_RESET_MASK	8

#define SHADOW_FREQ_CONFIG1_DPLL_EN_SHIFT	8
#define SHADOW_FREQ_CONFIG1_DPLL_EN_MASK	(7 << 8)

#define SHADOW_FREQ_CONFIG1_M2_DIV_SHIFT	11
#define SHADOW_FREQ_CONFIG1_M2_DIV_MASK		(0x1F << 11)

/*CM_<clock_domain>__CLKCTRL */
#define CD_CLKCTRL_CLKTRCTRL_SHIFT		0
#define CD_CLKCTRL_CLKTRCTRL_MASK		3

#define CD_CLKCTRL_CLKTRCTRL_NO_SLEEP		0
#define CD_CLKCTRL_CLKTRCTRL_SW_SLEEP		1
#define CD_CLKCTRL_CLKTRCTRL_SW_WKUP		2
#define CD_CLKCTRL_CLKTRCTRL_HW_AUTO		3


/* CM_<clock_domain>_<module>_CLKCTRL */
#define MODULE_CLKCTRL_MODULEMODE_SHIFT		0
#define MODULE_CLKCTRL_MODULEMODE_MASK		3
#define MODULE_CLKCTRL_IDLEST_SHIFT		16
#define MODULE_CLKCTRL_IDLEST_MASK		(3 << 16)

#define MODULE_CLKCTRL_MODULEMODE_SW_DISABLE		0
#define MODULE_CLKCTRL_MODULEMODE_HW_AUTO		1
#define MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN	2

#define MODULE_CLKCTRL_IDLEST_FULLY_FUNCTIONAL	0
#define MODULE_CLKCTRL_IDLEST_TRANSITIONING	1
#define MODULE_CLKCTRL_IDLEST_IDLE		2
#define MODULE_CLKCTRL_IDLEST_DISABLED		3

/* CM_L4PER_GPIO4_CLKCTRL */
#define GPIO4_CLKCTRL_OPTFCLKEN_MASK		(1 << 8)

/* CM_L3INIT_HSMMCn_CLKCTRL */
#define HSMMC_CLKCTRL_CLKSEL_MASK		(1 << 24)
#define HSMMC_CLKCTRL_CLKSEL_DIV_MASK		(1 << 25)

/* CM_WKUP_GPTIMER1_CLKCTRL */
#define GPTIMER1_CLKCTRL_CLKSEL_MASK		(1 << 24)

/* CM_CAM_ISS_CLKCTRL */
#define ISS_CLKCTRL_OPTFCLKEN_MASK		(1 << 8)

/* CM_DSS_DSS_CLKCTRL */
#define DSS_CLKCTRL_OPTFCLKEN_MASK		0xF00

/* CM_L3INIT_USBPHY_CLKCTRL */
#define USBPHY_CLKCTRL_OPTFCLKEN_PHY_48M_MASK	8

/* CM_MPU_MPU_CLKCTRL */
#define MPU_CLKCTRL_CLKSEL_EMIF_DIV_MODE_SHIFT	24
#define MPU_CLKCTRL_CLKSEL_EMIF_DIV_MODE_MASK	(3 << 24)
#define MPU_CLKCTRL_CLKSEL_ABE_DIV_MODE_SHIFT	26
#define MPU_CLKCTRL_CLKSEL_ABE_DIV_MODE_MASK	(1 << 26)

/* CM_WKUPAON_SCRM_CLKCTRL */
#define OPTFCLKEN_SCRM_PER_SHIFT		9
#define OPTFCLKEN_SCRM_PER_MASK			(1 << 9)
#define OPTFCLKEN_SCRM_CORE_SHIFT		8
#define OPTFCLKEN_SCRM_CORE_MASK		(1 << 8)

/* CM_COREAON_IO_SRCOMP_CLKCTRL */
#define OPTFCLKEN_SRCOMP_FCLK_SHIFT		8
#define OPTFCLKEN_SRCOMP_FCLK_MASK		(1 << 8)

/* PRM_RSTTIME */
#define RSTTIME1_SHIFT				0
#define RSTTIME1_MASK				(0x3ff << 0)

/* Clock frequencies */
#define OMAP_SYS_CLK_IND_38_4_MHZ	6

/* PRM_VC_VAL_BYPASS */
#define PRM_VC_I2C_CHANNEL_FREQ_KHZ	400

/* SMPS */
#define SMPS_I2C_SLAVE_ADDR	0x12
#define SMPS_REG_ADDR_12_MPU	0x23
#define SMPS_REG_ADDR_45_IVA	0x2B
#define SMPS_REG_ADDR_8_CORE	0x37

/* PALMAS VOLTAGE SETTINGS in mv for OPP_NOMINAL */
/* ES1.0 settings */
#define VDD_MPU		1040
#define VDD_MM		1040
#define VDD_CORE	1040

#define VDD_MPU_LOW	890
#define VDD_MM_LOW	890
#define VDD_CORE_LOW	890

/* ES2.0 settings */
#define VDD_MPU_ES2	1060
#define VDD_MM_ES2	1025
#define VDD_CORE_ES2	1040

#define VDD_MPU_ES2_HIGH 1250
#define VDD_MM_ES2_OD  1120

#define VDD_MPU_ES2_LOW 880
#define VDD_MM_ES2_LOW 880

/* Standard offset is 0.5v expressed in uv */
#define PALMAS_SMPS_BASE_VOLT_UV 500000

/* TPS */
#define TPS62361_I2C_SLAVE_ADDR		0x60
#define TPS62361_REG_ADDR_SET0		0x0
#define TPS62361_REG_ADDR_SET1		0x1
#define TPS62361_REG_ADDR_SET2		0x2
#define TPS62361_REG_ADDR_SET3		0x3
#define TPS62361_REG_ADDR_CTRL		0x4
#define TPS62361_REG_ADDR_TEMP		0x5
#define TPS62361_REG_ADDR_RMP_CTRL	0x6
#define TPS62361_REG_ADDR_CHIP_ID	0x8
#define TPS62361_REG_ADDR_CHIP_ID_2	0x9

#define TPS62361_BASE_VOLT_MV	500
#define TPS62361_VSEL0_GPIO	7

#define DPLL_NO_LOCK	0
#define DPLL_LOCK	1

/*
 * MAX value for PRM_RSTTIME[9:0]RSTTIME1 stored is 0x3ff.
 * 0x3ff is in the no of FUNC_32K_CLK cycles. Converting cycles
 * into microsec and passing the value.
 */
#define CONFIG_DEFAULT_OMAP_RESET_TIME_MAX_USEC	31219
#endif /* _CLOCKS_OMAP5_H_ */
