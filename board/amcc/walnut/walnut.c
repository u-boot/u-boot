/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <asm/processor.h>
#include <spd_sdram.h>

int board_early_init_f(void)
{
	/*-------------------------------------------------------------------------+
	  | Interrupt controller setup for the Walnut/Sycamore board.
	  | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
	  |       IRQ 16    405GP internally generated; active low; level sensitive
	  |       IRQ 17-24 RESERVED
	  |       IRQ 25 (EXT IRQ 0) FPGA; active high; level sensitive
	  |       IRQ 26 (EXT IRQ 1) SMI; active high; level sensitive
	  |       IRQ 27 (EXT IRQ 2) Not Used
	  |       IRQ 28 (EXT IRQ 3) PCI SLOT 3; active low; level sensitive
	  |       IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
	  |       IRQ 30 (EXT IRQ 5) PCI SLOT 1; active low; level sensitive
	  |       IRQ 31 (EXT IRQ 6) PCI SLOT 0; active low; level sensitive
	  | Note for Walnut board:
	  |       An interrupt taken for the FPGA (IRQ 25) indicates that either
	  |       the Mouse, Keyboard, IRDA, or External Expansion caused the
	  |       interrupt. The FPGA must be read to determine which device
	  |       caused the interrupt. The default setting of the FPGA clears
	  |
	  +-------------------------------------------------------------------------*/

	mtdcr(uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(uicer, 0x00000000);	/* disable all ints */
	mtdcr(uiccr, 0x00000020);	/* set all but FPGA SMI to be non-critical */
	mtdcr(uicpr, 0xFFFFFFE0);	/* set int polarities */
	mtdcr(uictr, 0x10000000);	/* set int trigger levels */
	mtdcr(uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr(uicsr, 0xFFFFFFFF);	/* clear all ints */

	/* set UART1 control to select CTS/RTS */
#define FPGA_BRDC       0xF0300004
	*(volatile char *)(FPGA_BRDC) |= 0x1;

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char *s = getenv("serial#");
	uint pvr = get_pvr();

	if (pvr == PVR_405GPR_RB) {
		puts("Board: Sycamore - AMCC PPC405GPr Evaluation Board");
	} else {
		puts("Board: Walnut - AMCC PPC405GP Evaluation Board");
	}

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

/*
 * initdram(int board_type) reads EEPROM via I2c. EEPROM contains all of
 * the necessary info for SDRAM controller configuration
 */
phys_size_t initdram(int board_type)
{
	return spd_sdram();
}
