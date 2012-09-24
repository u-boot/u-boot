/*
 *
 * Functions for omap5 based boards.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
 *	Sricharan	<r.sricharan@ti.com>
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
#include <asm/utils.h>
#include <asm/arch/gpio.h>
#include <asm/emif.h>

DECLARE_GLOBAL_DATA_PTR;

u32 *const omap_si_rev = (u32 *)OMAP5_SRAM_SCRATCH_OMAP5_REV;

static struct gpio_bank gpio_bank_54xx[6] = {
	{ (void *)OMAP54XX_GPIO1_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO2_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO3_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO4_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO5_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO6_BASE, METHOD_GPIO_24XX },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_54xx;

#ifdef CONFIG_SPL_BUILD
/* LPDDR2 specific IO settings */
static void io_settings_lpddr2(void)
{
	struct omap_sys_ctrl_regs *ioregs_base =
		      (struct omap_sys_ctrl_regs *) SYSCTRL_GENERAL_CORE_BASE;

	writel(DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN,
				&(ioregs_base->control_ddrch1_0));
	writel(DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN,
				&(ioregs_base->control_ddrch1_1));
	writel(DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN,
				&(ioregs_base->control_ddrch2_0));
	writel(DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN,
				&(ioregs_base->control_ddrch2_1));
	writel(DDR_IO_I_34OHM_SR_FASTEST_WD_CK_CKE_NCS_CA_PULL_DOWN,
				&(ioregs_base->control_lpddr2ch1_0));
	writel(DDR_IO_I_34OHM_SR_FASTEST_WD_CK_CKE_NCS_CA_PULL_DOWN,
				&(ioregs_base->control_lpddr2ch1_1));
	writel(DDR_IO_0_DDR2_DQ_INT_EN_ALL_DDR3_CA_DIS_ALL,
				&(ioregs_base->control_ddrio_0));
	writel(DDR_IO_1_DQ_OUT_EN_ALL_DQ_INT_EN_ALL,
				&(ioregs_base->control_ddrio_1));
	writel(DDR_IO_2_CA_OUT_EN_ALL_CA_INT_EN_ALL,
				&(ioregs_base->control_ddrio_2));
}

/* DDR3 specific IO settings */
static void io_settings_ddr3(void)
{
	u32 io_settings = 0;
	struct omap_sys_ctrl_regs *ioregs_base =
		      (struct omap_sys_ctrl_regs *) SYSCTRL_GENERAL_CORE_BASE;

	writel(DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL,
				&(ioregs_base->control_ddr3ch1_0));
	writel(DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL,
				&(ioregs_base->control_ddrch1_0));
	writel(DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL,
				&(ioregs_base->control_ddrch1_1));

	writel(DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL,
				&(ioregs_base->control_ddr3ch2_0));
	writel(DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL,
				&(ioregs_base->control_ddrch2_0));
	writel(DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL,
				&(ioregs_base->control_ddrch2_1));

	writel(DDR_IO_0_VREF_CELLS_DDR3_VALUE,
				&(ioregs_base->control_ddrio_0));
	writel(DDR_IO_1_VREF_CELLS_DDR3_VALUE,
				&(ioregs_base->control_ddrio_1));
	writel(DDR_IO_2_VREF_CELLS_DDR3_VALUE,
				&(ioregs_base->control_ddrio_2));

	/* omap5432 does not use lpddr2 */
	writel(0x0, &(ioregs_base->control_lpddr2ch1_0));
	writel(0x0, &(ioregs_base->control_lpddr2ch1_1));

	writel(SDRAM_CONFIG_EXT_RD_LVL_11_SAMPLES,
			&(ioregs_base->control_emif1_sdram_config_ext));
	writel(SDRAM_CONFIG_EXT_RD_LVL_11_SAMPLES,
			&(ioregs_base->control_emif2_sdram_config_ext));

	/* Disable DLL select */
	io_settings = (readl(&(ioregs_base->control_port_emif1_sdram_config))
							& 0xFFEFFFFF);
	writel(io_settings,
		&(ioregs_base->control_port_emif1_sdram_config));

	io_settings = (readl(&(ioregs_base->control_port_emif2_sdram_config))
							& 0xFFEFFFFF);
	writel(io_settings,
		&(ioregs_base->control_port_emif2_sdram_config));
}

/*
 * Some tuning of IOs for optimal power and performance
 */
void do_io_settings(void)
{
	u32 io_settings = 0, mask = 0;
	struct omap_sys_ctrl_regs *ioregs_base =
		      (struct omap_sys_ctrl_regs *) SYSCTRL_GENERAL_CORE_BASE;

	/* Impedance settings EMMC, C2C 1,2, hsi2 */
	mask = (ds_mask << 2) | (ds_mask << 8) |
		(ds_mask << 16) | (ds_mask << 18);
	io_settings = readl(&(ioregs_base->control_smart1io_padconf_0)) &
				(~mask);
	io_settings |= (ds_60_ohm << 8) | (ds_45_ohm << 16) |
			(ds_45_ohm << 18) | (ds_60_ohm << 2);
	writel(io_settings, &(ioregs_base->control_smart1io_padconf_0));

	/* Impedance settings Mcspi2 */
	mask = (ds_mask << 30);
	io_settings = readl(&(ioregs_base->control_smart1io_padconf_1)) &
			(~mask);
	io_settings |= (ds_60_ohm << 30);
	writel(io_settings, &(ioregs_base->control_smart1io_padconf_1));

	/* Impedance settings C2C 3,4 */
	mask = (ds_mask << 14) | (ds_mask << 16);
	io_settings = readl(&(ioregs_base->control_smart1io_padconf_2)) &
			(~mask);
	io_settings |= (ds_45_ohm << 14) | (ds_45_ohm << 16);
	writel(io_settings, &(ioregs_base->control_smart1io_padconf_2));

	/* Slew rate settings EMMC, C2C 1,2 */
	mask = (sc_mask << 8) | (sc_mask << 16) | (sc_mask << 18);
	io_settings = readl(&(ioregs_base->control_smart2io_padconf_0)) &
			(~mask);
	io_settings |= (sc_fast << 8) | (sc_na << 16) | (sc_na << 18);
	writel(io_settings, &(ioregs_base->control_smart2io_padconf_0));

	/* Slew rate settings hsi2, Mcspi2 */
	mask = (sc_mask << 24) | (sc_mask << 28);
	io_settings = readl(&(ioregs_base->control_smart2io_padconf_1)) &
			(~mask);
	io_settings |= (sc_fast << 28) | (sc_fast << 24);
	writel(io_settings, &(ioregs_base->control_smart2io_padconf_1));

	/* Slew rate settings C2C 3,4 */
	mask = (sc_mask << 16) | (sc_mask << 18);
	io_settings = readl(&(ioregs_base->control_smart2io_padconf_2)) &
			(~mask);
	io_settings |= (sc_na << 16) | (sc_na << 18);
	writel(io_settings, &(ioregs_base->control_smart2io_padconf_2));

	/* impedance and slew rate settings for usb */
	mask = (usb_i_mask << 29) | (usb_i_mask << 26) | (usb_i_mask << 23) |
		(usb_i_mask << 20) | (usb_i_mask << 17) | (usb_i_mask << 14);
	io_settings = readl(&(ioregs_base->control_smart3io_padconf_1)) &
			(~mask);
	io_settings |= (ds_60_ohm << 29) | (ds_60_ohm << 26) |
		       (ds_60_ohm << 23) | (sc_fast << 20) |
		       (sc_fast << 17) | (sc_fast << 14);
	writel(io_settings, &(ioregs_base->control_smart3io_padconf_1));

	if (omap_revision() <= OMAP5430_ES1_0)
		io_settings_lpddr2();
	else
		io_settings_ddr3();

	/* Efuse settings */
	writel(EFUSE_1, &(ioregs_base->control_efuse_1));
	writel(EFUSE_2, &(ioregs_base->control_efuse_2));
	writel(EFUSE_3, &(ioregs_base->control_efuse_3));
	writel(EFUSE_4, &(ioregs_base->control_efuse_4));
}
#endif

void config_data_eye_leveling_samples(u32 emif_base)
{
	struct omap_sys_ctrl_regs *ioregs_base =
		(struct omap_sys_ctrl_regs *) SYSCTRL_GENERAL_CORE_BASE;

	/*EMIF_SDRAM_CONFIG_EXT-Read data eye leveling no of samples =4*/
	if (emif_base == EMIF1_BASE)
		writel(SDRAM_CONFIG_EXT_RD_LVL_4_SAMPLES,
			&(ioregs_base->control_emif1_sdram_config_ext));
	else if (emif_base == EMIF2_BASE)
		writel(SDRAM_CONFIG_EXT_RD_LVL_4_SAMPLES,
			&(ioregs_base->control_emif2_sdram_config_ext));
}

void init_omap_revision(void)
{
	/*
	 * For some of the ES2/ES1 boards ID_CODE is not reliable:
	 * Also, ES1 and ES2 have different ARM revisions
	 * So use ARM revision for identification
	 */
	unsigned int rev = cortex_rev();

	switch (rev) {
	case MIDR_CORTEX_A15_R0P0:
		switch (readl(CONTROL_ID_CODE)) {
		case OMAP5430_CONTROL_ID_CODE_ES1_0:
			*omap_si_rev = OMAP5430_ES1_0;
			break;
		case OMAP5432_CONTROL_ID_CODE_ES1_0:
		default:
			*omap_si_rev = OMAP5432_ES1_0;
			break;
		}
		break;
	default:
		*omap_si_rev = OMAP5430_SILICON_ID_INVALID;
	}
}

void reset_cpu(ulong ignored)
{
	u32 omap_rev = omap_revision();

	/*
	 * WARM reset is not functional in case of OMAP5430 ES1.0 soc.
	 * So use cold reset in case instead.
	 */
	if (omap_rev == OMAP5430_ES1_0)
		writel(PRM_RSTCTRL_RESET << 0x1, PRM_RSTCTRL);
	else
		writel(PRM_RSTCTRL_RESET, PRM_RSTCTRL);
}
