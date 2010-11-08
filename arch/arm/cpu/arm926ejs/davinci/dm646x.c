/*
 * SoC-specific code for TMS320DM646x chips
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

#include <asm/arch/hardware.h>

void davinci_enable_uart0(void)
{
	lpsc_on(DAVINCI_DM646X_LPSC_UART0);
}

#ifdef CONFIG_DRIVER_TI_EMAC
void davinci_enable_emac(void)
{
	lpsc_on(DAVINCI_DM646X_LPSC_EMAC);
}
#endif

#ifdef CONFIG_DRIVER_DAVINCI_I2C
void davinci_enable_i2c(void)
{
	lpsc_on(DAVINCI_DM646X_LPSC_I2C);
}
#endif
