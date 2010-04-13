/*
 * SoC-specific code for tms320dm355 and similar chips
 *
 * Copyright (C) 2009 David Brownell
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <asm/arch/hardware.h>


void davinci_enable_uart0(void)
{
	lpsc_on(DAVINCI_LPSC_UART0);

	/* Bringup UART0 out of reset */
	REG(UART0_PWREMU_MGMT) = 0x00006001;
}


#ifdef CONFIG_DRIVER_DAVINCI_I2C
void davinci_enable_i2c(void)
{
	lpsc_on(DAVINCI_LPSC_I2C);

	/* Enable I2C pin Mux */
	REG(PINMUX3) |= (1 << 20) | (1 << 19);
}
#endif
