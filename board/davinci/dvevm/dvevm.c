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
#include <asm/arch/emac_defs.h>
#include "../common/psc.h"
#include "../common/misc.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* Workaround for TMS320DM6446 errata 1.3.22 */
	REG(PSC_SILVER_BULLET) = 0;

	/* Power on required peripherals */
	lpsc_on(DAVINCI_LPSC_EMAC);
	lpsc_on(DAVINCI_LPSC_EMAC_WRAPPER);
	lpsc_on(DAVINCI_LPSC_MDIO);
	lpsc_on(DAVINCI_LPSC_I2C);
	lpsc_on(DAVINCI_LPSC_UART0);
	lpsc_on(DAVINCI_LPSC_TIMER1);
	lpsc_on(DAVINCI_LPSC_GPIO);

#if !defined(CONFIG_SYS_USE_DSPLINK)
	/* Powerup the DSP */
	dsp_on();
#endif /* CONFIG_SYS_USE_DSPLINK */

	/* Bringup UART0 out of reset */
	REG(UART0_PWREMU_MGMT) = 0x0000e003;

	/* Enable GIO3.3V cells used for EMAC */
	REG(VDD3P3V_PWDN) = 0;

	/* Enable UART0 MUX lines */
	REG(PINMUX1) |= 1;

	/* Enable EMAC and AEMIF pins */
	REG(PINMUX0) = 0x80000c1f;

	/* Enable I2C pin Mux */
	REG(PINMUX1) |= (1 << 7);

	/* Set the Bus Priority Register to appropriate value */
	REG(VBPR) = 0x20;

	timer_init();

	return(0);
}

int misc_init_r(void)
{
	uint8_t video_mode;
	uint8_t eeprom_enetaddr[6];

	dv_display_clk_infos();

	/* Read Ethernet MAC address from EEPROM if available. */
	if (dvevm_read_mac_address(eeprom_enetaddr))
		dv_configure_mac_address(eeprom_enetaddr);

	if (!eth_hw_init())
		printf("ethernet init failed!\n");

	i2c_read(0x39, 0x00, 1, &video_mode, 1);

	setenv("videostd", ((video_mode & 0x80) ? "pal" : "ntsc"));

	return(0);
}
