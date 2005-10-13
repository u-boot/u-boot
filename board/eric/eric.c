/*
 * (C) Copyright 2001
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
#include <i2c.h>
#include "eric.h"
#include <asm/processor.h>

#define PPC405GP_GPIO0_OR      0xef600700	/* GPIO Output */
#define PPC405GP_GPIO0_TCR     0xef600704	/* GPIO Three-State Control */
#define PPC405GP_GPIO0_ODR     0xef600718	/* GPIO Open Drain */
#define PPC405GP_GPIO0_IR      0xef60071c	/* GPIO Input */

int board_early_init_f (void)
{

   /*-------------------------------------------------------------------------+
   | Interrupt controller setup for the ERIC board.
   | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
   |       IRQ 16    405GP internally generated; active low; level sensitive
   |       IRQ 17-24 RESERVED
   |       IRQ 25 (EXT IRQ 0) FLASH; active low; level sensitive
   |       IRQ 26 (EXT IRQ 1) PHY ; active low; level sensitive
   |       IRQ 27 (EXT IRQ 2) HOST FAIL, active low; level sensitive
   |                          indicates NO Power or HOST RESET active
   |                          check GPIO7 (HOST RESET#) and GPIO8 (NO Power#)
   |                          for real IRQ source
   |       IRQ 28 (EXT IRQ 3) HOST; active high; level sensitive
   |       IRQ 29 (EXT IRQ 4) PCI INTC#; active low; level sensitive
   |       IRQ 30 (EXT IRQ 5) PCI INTB#; active low; level sensitive
   |       IRQ 31 (EXT IRQ 6) PCI INTA#; active low; level sensitive
   |        -> IRQ6 Pin is NOW GPIO23 and can be activateted by setting
   |           PPC405GP_GPIO0_TCR Bit 0 = 1 (driving the output as defined in PPC405GP_GPIO0_OR,
   |           else tristate)
   | Note for ERIC board:
   |       An interrupt taken for the HOST (IRQ 28) indicates that
   |       the HOST wrote a "1" to one of the following locations
   |       - VGA CRT_GPIO0 (if R1216 is loaded)
   |       - VGA CRT_GPIO1 (if R1217 is loaded)
   |
   +-------------------------------------------------------------------------*/

	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (uicer, 0x00000000);	/* disable all ints */
	mtdcr (uiccr, 0x00000000);	/* set all SMI to be non-critical */
	mtdcr (uicpr, 0xFFFFFF88);	/* set int polarities; IRQ3 to 1 */
	mtdcr (uictr, 0x10000000);	/* set int trigger levels, UART0 is EDGE */
	mtdcr (uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */

	mtdcr (cntrl0, 0x00002000);	/* set IRQ6 as GPIO23 to generate an interrupt request to the PCP2PCI bridge */

	out32 (PPC405GP_GPIO0_OR, 0x60000000);	/*fixme is SMB_INT high or low active??; IRQ6 is GPIO23 output */
	out32 (PPC405GP_GPIO0_TCR, 0x7E400000);

	return 0;
}


/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	char *s = getenv ("serial#");
	char *e;

	puts ("Board: ");

	if (!s || strncmp (s, "ERIC", 9)) {
		puts ("### No HW ID - assuming ERIC");
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


/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/*
  initdram(int board_type) reads EEPROM via I2c. EEPROM contains all of
  the necessary info for SDRAM controller configuration
*/
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
long int initdram (int board_type)
{
#ifndef CONFIG_ERIC
	int i;
	unsigned char datain[128];
	int TotalSize;
#endif


#ifdef CONFIG_ERIC
	/*
	 * we have no EEPROM on ERIC
	 * so let init.S do the init job for SDRAM
	 * and simply return 32MByte here
	 */
	return (CFG_SDRAM_SIZE * 1024 * 1024);
#else

	/* Read Serial Presence Detect Information */
	for (i = 0; i < 128; i++)
		datain[i] = 127;
	i2c_send (SPD_EEPROM_ADDRESS, 0, 1, datain, 128);
	printf ("\nReading DIMM...\n");
#if 0
	for (i = 0; i < 128; i++) {
		printf ("%d=0x%x ", i, datain[i]);
		if (((i + 1) % 10) == 0)
			printf ("\n");
	}
	printf ("\n");
#endif

  /*****************************/
	/* Retrieve interesting data */
  /*****************************/
	/* size of a SDRAM bank */
	/* Number of bytes per side / number of banks per side */
	if (datain[31] == 0x08)
		TotalSize = 32;
	else if (datain[31] == 0x10)
		TotalSize = 64;
	else {
		printf ("IIC READ ERROR!!!\n");
		TotalSize = 32;
	}

	/* single-sided DIMM or double-sided DIMM? */
	if (datain[5] != 1) {
		/* double-sided DIMM => SDRAM banks 0..3 are valid */
		printf ("double-sided DIMM\n");
		TotalSize *= 2;
	}
	/* else single-sided DIMM => SDRAM bank 0 and bank 2 are valid */
	else {
		printf ("single-sided DIMM\n");
	}


	/* return size in Mb unit => *(1024*1024) */
	return (TotalSize * 1024 * 1024);
#endif
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: xxx MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */
