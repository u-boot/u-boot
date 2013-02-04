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
#include <asm/emif.h>

#ifndef CONFIG_SPL_BUILD
/*
 * printing to console doesn't work unless
 * this code is executed from SPL
 */
#define printf(fmt, args...)
#define puts(s)
#endif

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

	if (emif_sdram_type() == EMIF_SDRAM_TYPE_DDR3) {
		/* Configure LDO SRAM "magic" bits */
		writel(2, (*prcm)->prm_sldo_core_setup);
		writel(2, (*prcm)->prm_sldo_mpu_setup);
		writel(2, (*prcm)->prm_sldo_mm_setup);
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
