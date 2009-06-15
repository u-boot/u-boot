/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts are shamelessly stolen from various TI sources, original copyright
 * follows:
 * -----------------------------------------------------------------
 *
 * Copyright (C) 2004 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 */

#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include "../common/misc.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* Configure AEMIF pins (although this should be configured at boot time
	 * with pull-up/pull-down resistors) */
	REG(PINMUX0) = 0x00000c1f;

	davinci_errata_workarounds();

	/* Power on required peripherals */
	lpsc_on(DAVINCI_LPSC_GPIO);
	lpsc_on(DAVINCI_LPSC_USB);

#if !defined(CONFIG_SYS_USE_DSPLINK)
	/* Powerup the DSP */
	dsp_on();
#endif /* CONFIG_SYS_USE_DSPLINK */

	davinci_enable_uart0();
	davinci_enable_emac();
	davinci_enable_i2c();

	lpsc_on(DAVINCI_LPSC_TIMER1);
	timer_init();

	return(0);
}

int misc_init_r(void)
{
	uint8_t video_mode;
	uint8_t eeprom_enetaddr[6];

	/* Read Ethernet MAC address from EEPROM if available. */
	if (dvevm_read_mac_address(eeprom_enetaddr))
		dv_configure_mac_address(eeprom_enetaddr);

	i2c_read(0x39, 0x00, 1, &video_mode, 1);

	setenv("videostd", ((video_mode & 0x80) ? "pal" : "ntsc"));

	return(0);
}

#ifdef CONFIG_USB_DAVINCI

/* IO Expander I2C address and USB VBUS enable mask */
#define IOEXP_I2C_ADDR 0x3A
#define IOEXP_VBUSEN_MASK 1

/*
 * This function enables USB VBUS by writting to IO expander using I2C.
 * Note that the I2C is already initialized at this stage. This
 * function is used by davinci specific USB wrapper code.
 */
void enable_vbus(void)
{
	uchar data;  /* IO Expander data to enable VBUS */

	/* Write to IO expander to enable VBUS */
	i2c_read(IOEXP_I2C_ADDR, 0, 0, &data, 1);
	data &= ~IOEXP_VBUSEN_MASK;
	i2c_write(IOEXP_I2C_ADDR, 0, 0, &data, 1);
}
#endif
