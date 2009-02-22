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
#include <asm/arch/hardware.h>
#include "../common/psc.h"
#include "../common/misc.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_SONATA;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* Configure AEMIF pins (although this should be configured at boot time
	 * with pull-up/pull-down resistors) */
	REG(PINMUX0) = 0x00000c1f;

	davinci_errata_workarounds();

	/* Power on required peripherals */
	lpsc_on(DAVINCI_LPSC_GPIO);

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
	uint8_t eeprom_enetaddr[6];

	dv_display_clk_infos();

	/* Read Ethernet MAC address from EEPROM if available. */
	if (dvevm_read_mac_address(eeprom_enetaddr))
		dv_configure_mac_address(eeprom_enetaddr);

	if (!eth_hw_init())
		printf("ethernet init failed!\n");

	return(0);
}
