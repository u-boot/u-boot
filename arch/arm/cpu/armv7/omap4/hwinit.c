/*
 *
 * Common functions for OMAP4 based boards
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
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
#include <asm/armv7.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <asm/sizes.h>
#include <asm/emif.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

u32 *const omap_si_rev = (u32 *)OMAP4_SRAM_SCRATCH_OMAP4_REV;

static const struct gpio_bank gpio_bank_44xx[6] = {
	{ (void *)OMAP44XX_GPIO1_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP44XX_GPIO2_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP44XX_GPIO3_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP44XX_GPIO4_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP44XX_GPIO5_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP44XX_GPIO6_BASE, METHOD_GPIO_24XX },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_44xx;

#ifdef CONFIG_SPL_BUILD
/*
 * Some tuning of IOs for optimal power and performance
 */
void do_io_settings(void)
{
	u32 lpddr2io;
	struct control_lpddr2io_regs *lpddr2io_regs =
		(struct control_lpddr2io_regs *)LPDDR2_IO_REGS_BASE;
	struct omap_sys_ctrl_regs *const ctrl =
		(struct omap_sys_ctrl_regs *)SYSCTRL_GENERAL_CORE_BASE;

	u32 omap4_rev = omap_revision();

	if (omap4_rev == OMAP4430_ES1_0)
		lpddr2io = CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN;
	else if (omap4_rev == OMAP4430_ES2_0)
		lpddr2io = CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER;
	else
		lpddr2io = CONTROL_LPDDR2IO_SLEW_315PS_DRV12_PULL_DOWN;

	/* EMIF1 */
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io1_0);
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io1_1);
	/* No pull for GR10 as per hw team's recommendation */
	writel(lpddr2io & ~LPDDR2IO_GR10_WD_MASK,
		&lpddr2io_regs->control_lpddr2io1_2);
	writel(CONTROL_LPDDR2IO_3_VAL, &lpddr2io_regs->control_lpddr2io1_3);

	/* EMIF2 */
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io2_0);
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io2_1);
	/* No pull for GR10 as per hw team's recommendation */
	writel(lpddr2io & ~LPDDR2IO_GR10_WD_MASK,
		&lpddr2io_regs->control_lpddr2io2_2);
	writel(CONTROL_LPDDR2IO_3_VAL, &lpddr2io_regs->control_lpddr2io2_3);

	/*
	 * Some of these settings (TRIM values) come from eFuse and are
	 * in turn programmed in the eFuse at manufacturing time after
	 * calibration of the device. Do the software over-ride only if
	 * the device is not correctly trimmed
	 */
	if (!(readl(&ctrl->control_std_fuse_opp_bgap) & 0xFFFF)) {

		writel(LDOSRAM_VOLT_CTRL_OVERRIDE,
			&ctrl->control_ldosram_iva_voltage_ctrl);

		writel(LDOSRAM_VOLT_CTRL_OVERRIDE,
			&ctrl->control_ldosram_mpu_voltage_ctrl);

		writel(LDOSRAM_VOLT_CTRL_OVERRIDE,
			&ctrl->control_ldosram_core_voltage_ctrl);
	}

	/*
	 * Over-ride the register
	 *	i. unconditionally for all 4430
	 *	ii. only if un-trimmed for 4460
	 */
	if (!readl(&ctrl->control_efuse_1))
		writel(CONTROL_EFUSE_1_OVERRIDE, &ctrl->control_efuse_1);

	if ((omap4_rev < OMAP4460_ES1_0) || !readl(&ctrl->control_efuse_2))
		writel(CONTROL_EFUSE_2_OVERRIDE, &ctrl->control_efuse_2);
}
#endif

/* dummy fuction for omap4 */
void config_data_eye_leveling_samples(u32 emif_base)
{
}

void init_omap_revision(void)
{
	/*
	 * For some of the ES2/ES1 boards ID_CODE is not reliable:
	 * Also, ES1 and ES2 have different ARM revisions
	 * So use ARM revision for identification
	 */
	unsigned int arm_rev = cortex_rev();

	switch (arm_rev) {
	case MIDR_CORTEX_A9_R0P1:
		*omap_si_rev = OMAP4430_ES1_0;
		break;
	case MIDR_CORTEX_A9_R1P2:
		switch (readl(CONTROL_ID_CODE)) {
		case OMAP4_CONTROL_ID_CODE_ES2_0:
			*omap_si_rev = OMAP4430_ES2_0;
			break;
		case OMAP4_CONTROL_ID_CODE_ES2_1:
			*omap_si_rev = OMAP4430_ES2_1;
			break;
		case OMAP4_CONTROL_ID_CODE_ES2_2:
			*omap_si_rev = OMAP4430_ES2_2;
			break;
		default:
			*omap_si_rev = OMAP4430_ES2_0;
			break;
		}
		break;
	case MIDR_CORTEX_A9_R1P3:
		*omap_si_rev = OMAP4430_ES2_3;
		break;
	case MIDR_CORTEX_A9_R2P10:
		switch (readl(CONTROL_ID_CODE)) {
		case OMAP4460_CONTROL_ID_CODE_ES1_1:
			*omap_si_rev = OMAP4460_ES1_1;
			break;
		case OMAP4460_CONTROL_ID_CODE_ES1_0:
		default:
			*omap_si_rev = OMAP4460_ES1_0;
			break;
		}
		break;
	default:
		*omap_si_rev = OMAP4430_SILICON_ID_INVALID;
		break;
	}
}

#ifndef CONFIG_SYS_L2CACHE_OFF
void v7_outer_cache_enable(void)
{
	set_pl310_ctrl_reg(1);
}

void v7_outer_cache_disable(void)
{
	set_pl310_ctrl_reg(0);
}
#endif
