/*
 * (C) Copyright 2005-2008 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
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
#include <asm/arch/mux.h>

#define write_config_reg(reg, value)                                    \
do {                                                                    \
	writeb(value, reg);                                             \
} while (0)

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init(void)
{
	return 0;
}

#ifdef CONFIG_SYS_PRINTF
/* Pin Muxing registers used for UART1 */
/****************************************
 * Routine: muxSetupUART1  (ostboot)
 * Description: Set up uart1 muxing
 *****************************************/
static void muxSetupUART1(void)
{
	/* UART1_CTS pin configuration, PIN = D21 */
	write_config_reg(CONTROL_PADCONF_UART1_CTS, 0);
	/* UART1_RTS pin configuration, PIN = H21 */
	write_config_reg(CONTROL_PADCONF_UART1_RTS, 0);
	/* UART1_TX pin configuration, PIN = L20 */
	write_config_reg(CONTROL_PADCONF_UART1_TX, 0);
	/* UART1_RX pin configuration, PIN = T21 */
	write_config_reg(CONTROL_PADCONF_UART1_RX, 0);
}
#endif

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called at time when only stack is available.
 **********************************************************/
int s_init(int skip)
{
#ifdef CONFIG_SYS_PRINTF
	muxSetupUART1();
#endif
	return 0;
}
