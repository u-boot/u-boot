/*
 * (C) Copyright 2000
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
#include "walnut405.h"
#include <asm/processor.h>
#include <spd_sdram.h>

int board_early_init_f (void)
{
   /*-------------------------------------------------------------------------+
   | Interrupt controller setup for the Walnut board.
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

	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (uicer, 0x00000000);	/* disable all ints */
	mtdcr (uiccr, 0x00000020);	/* set all but FPGA SMI to be non-critical */
	mtdcr (uicpr, 0xFFFFFFE0);	/* set int polarities */
	mtdcr (uictr, 0x10000000);	/* set int trigger levels */
	mtdcr (uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */

#define mtebc(reg, data)  mtdcr(ebccfga,reg);mtdcr(ebccfgd,data)
	/* BAS=0xF00,BS=0x0(1MB),BU=0x3(R/W),BW=0x0( 8 bits) */
	mtebc (pb1ap, 0x02815480);
	mtebc (pb1cr, 0xF0018000);

	/* BAS=0xF01,BS=0x0(1MB),BU=0x3(R/W),BW=0x0(8 bits) */
	mtebc (pb2ap, 0x04815A80);
	mtebc (pb2cr, 0xF0118000);

	/* BAS=0xF02,BS=0x0(1MB),BU=0x3(R/W),BW=0x0( 8 bits) */
	mtebc (pb3ap, 0x01815280);
	mtebc (pb3cr, 0xF0218000);

	/* BAS=0xF03,BS=0x0(1MB),BU=0x3(R/W),BW=0x0(8 bits) */
	mtebc (pb7ap, 0x01815280);
	mtebc (pb7cr, 0xF0318000);

	/* set UART1 control to select CTS/RTS */
#define FPGA_BRDC       0xF0300004
	*(volatile char *) (FPGA_BRDC) |= 0x1;

	return 0;
}


/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	unsigned char *s = getenv ("serial#");
	unsigned char *e;

	puts ("Board: ");

	if (!s || strncmp (s, "WALNUT405", 9)) {
		puts ("### No HW ID - assuming WALNUT405");
	} else {
		for (e = s; *e; ++e) {
			if (*e == ' ')
				break;
		}
		for (; s < e; ++s) {
			putc (*s);
		}
	}
	putc ('\n');

	return (0);
}


/* -------------------------------------------------------------------------
  initdram(int board_type) reads EEPROM via I2c. EEPROM contains all of
  the necessary info for SDRAM controller configuration
   ------------------------------------------------------------------------- */
long int initdram (int board_type)
{
	return  spd_sdram (0);
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: xxx MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */
