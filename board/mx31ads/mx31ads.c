/*
 * Copyright (C) 2008, Guennadi Liakhovetski <lg@denx.de>
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
#include <asm/io.h>
#include <asm/arch/mx31.h>
#include <asm/arch/mx31-regs.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

int board_init(void)
{
	int i;
#if 0
	/* CS0: Nor Flash */
	/*
	 * These are values from the RedBoot sources by Freescale. However,
	 * under U-Boot with this configuration 32-bit accesses don't work,
	 * lower 16 bits of data are read twice for each 32-bit read.
	 */
	__REG(CSCR_U(0)) = 0x23524E80;
	__REG(CSCR_L(0)) = 0x10000D03; /* WRAP bit (1) is suspicious here, but
					* disabling it doesn't help either */
	__REG(CSCR_A(0)) = 0x00720900;
#endif

	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_RTS1__UART1_CTS_B);

	/* PBC setup */
	/* Enable UART transceivers also reset the Ethernet/external UART */
	readw(CS4_BASE + 4);

	writew(0x8023, CS4_BASE + 4);

	/* RedBoot also has an empty loop with 100000 iterations here -
	 * clock doesn't run yet */
	for (i = 0; i < 100000; i++)
		;

	/* Clear the reset, toggle the LEDs */
	writew(0xDF, CS4_BASE + 6);

	/* clock still doesn't run */
	for (i = 0; i < 100000; i++)
		;

	/* See 1.5.4 in IMX31ADSE_PERI_BUS_CNTRL_CPLD_RM.pdf */
	readb(CS4_BASE + 8);
	readb(CS4_BASE + 7);
	readb(CS4_BASE + 8);
	readb(CS4_BASE + 7);

	gd->bd->bi_arch_number = 447;		/* board id for linux */
	gd->bd->bi_boot_params = 0x80000100;	/* adress of boot parameters */

	return 0;
}

int checkboard(void)
{
	printf("Board: MX31ADS\n");
	return 0;
}
