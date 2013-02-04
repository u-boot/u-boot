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
#endif /* !CONFIG_SPL_BUILD */

/*
 * Setup the voltages for vdd_mpu, vdd_core, and vdd_iva
 * We set the maximum voltages allowed here because Smart-Reflex is not
 * enabled in bootloader. Voltage initialization in the kernel will set
 * these to the nominal values after enabling Smart-Reflex
 */
void scale_vcores(void)
{
	u32 volt, omap_rev;

	omap_vc_init(PRM_VC_I2C_CHANNEL_FREQ_KHZ);

	omap_rev = omap_revision();

	/*
	 * Scale Voltage rails:
	 * 1. VDD_CORE
	 * 3. VDD_MPU
	 * 3. VDD_IVA
	 */
	if (omap_rev < OMAP4460_ES1_0) {
		/*
		 * OMAP4430:
		 * VDD_CORE = TWL6030 VCORE3
		 * VDD_MPU = TWL6030 VCORE1
		 * VDD_IVA = TWL6030 VCORE2
		 */
		volt = 1200;
		do_scale_vcore(SMPS_REG_ADDR_VCORE3, volt);

		/*
		 * note on VDD_MPU:
		 * Setting a high voltage for Nitro mode as smart reflex is not
		 * enabled. We use the maximum possible value in the AVS range
		 * because the next higher voltage in the discrete range
		 * (code >= 0b111010) is way too high.
		 */
		volt = 1325;
		do_scale_vcore(SMPS_REG_ADDR_VCORE1, volt);
		volt = 1200;
		do_scale_vcore(SMPS_REG_ADDR_VCORE2, volt);

	} else {
		/*
		 * OMAP4460:
		 * VDD_CORE = TWL6030 VCORE1
		 * VDD_MPU = TPS62361
		 * VDD_IVA = TWL6030 VCORE2
		 */
		volt = 1200;
		do_scale_vcore(SMPS_REG_ADDR_VCORE1, volt);
		/* TPS62361 */
		volt = 1203;
		do_scale_tps62361(TPS62361_VSEL0_GPIO,
				  TPS62361_REG_ADDR_SET1, volt);
		/* VCORE 2 - supplies vdd_iva */
		volt = 1200;
		do_scale_vcore(SMPS_REG_ADDR_VCORE2, volt);
	}
}

u32 get_offset_code(u32 offset)
{
	u32 offset_code, step = 12660; /* 12.66 mV represented in uV */

	if (omap_revision() == OMAP4430_ES1_0)
		offset -= PHOENIX_SMPS_BASE_VOLT_STD_MODE_UV;
	else
		offset -= PHOENIX_SMPS_BASE_VOLT_STD_MODE_WITH_OFFSET_UV;

	offset_code = (offset + step - 1) / step;

	/* The code starts at 1 not 0 */
	return ++offset_code;
}
