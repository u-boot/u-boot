/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>

/******************************************************************************
 * Routine: power_init_r
 * Description: Configure power supply
 *****************************************************************************/
void power_init_r(void)
{
	unsigned char byte;

#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

	/*
	 * Configure OMAP3 supply voltages in power management
	 * companion chip.
	 */

	/* set VAUX3 to 2.8V */
	byte = DEV_GRP_P1;
	i2c_write(PWRMGT_ADDR_ID4, VAUX3_DEV_GRP, 1, &byte, 1);
	byte = VAUX3_VSEL_28;
	i2c_write(PWRMGT_ADDR_ID4, VAUX3_DEDICATED, 1, &byte, 1);

	/* set VPLL2 to 1.8V */
	byte = DEV_GRP_ALL;
	i2c_write(PWRMGT_ADDR_ID4, VPLL2_DEV_GRP, 1, &byte, 1);
	byte = VPLL2_VSEL_18;
	i2c_write(PWRMGT_ADDR_ID4, VPLL2_DEDICATED, 1, &byte, 1);

	/* set VDAC to 1.8V */
	byte = DEV_GRP_P1;
	i2c_write(PWRMGT_ADDR_ID4, VDAC_DEV_GRP, 1, &byte, 1);
	byte = VDAC_VSEL_18;
	i2c_write(PWRMGT_ADDR_ID4, VDAC_DEDICATED, 1, &byte, 1);

	/* enable LED */
	byte = LEDBPWM | LEDAPWM | LEDBON | LEDAON;
	i2c_write(PWRMGT_ADDR_ID3, LEDEN, 1, &byte, 1);
}
