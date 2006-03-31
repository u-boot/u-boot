/*
 * (C) Copyright 2001
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
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
#include "canbt.h"
#include <asm/processor.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);


/* ------------------------------------------------------------------------- */

#if 0
#define FPGA_DEBUG
#endif

/* fpga configuration data */
const unsigned char fpgadata[] = {
#include "fpgadata.c"
};

/*
 * include common fpga code (for esd boards)
 */
#include "../common/fpga.c"


int board_early_init_f (void)
{
	unsigned long cntrl0Reg;
	int index, len, i;
	int status;

	/*
	 * Setup GPIO pins
	 */
	cntrl0Reg = mfdcr (cntrl0) & 0xf0001fff;
	cntrl0Reg |= 0x0070f000;
	mtdcr (cntrl0, cntrl0Reg);

#ifdef FPGA_DEBUG
	/* set up serial port with default baudrate */
	(void) get_clocks ();
	gd->baudrate = CONFIG_BAUDRATE;
	serial_init ();
	console_init_f ();
#endif

	/*
	 * Boot onboard FPGA
	 */
	status = fpga_boot ((unsigned char *) fpgadata, sizeof (fpgadata));
	if (status != 0) {
		/* booting FPGA failed */
#ifndef FPGA_DEBUG
		/* set up serial port with default baudrate */
		(void) get_clocks ();
		gd->baudrate = CONFIG_BAUDRATE;
		serial_init ();
		console_init_f ();
#endif
		printf ("\nFPGA: Booting failed ");
		switch (status) {
		case ERROR_FPGA_PRG_INIT_LOW:
			printf ("(Timeout: INIT not low after asserting PROGRAM*)\n ");
			break;
		case ERROR_FPGA_PRG_INIT_HIGH:
			printf ("(Timeout: INIT not high after deasserting PROGRAM*)\n ");
			break;
		case ERROR_FPGA_PRG_DONE:
			printf ("(Timeout: DONE not high after programming FPGA)\n ");
			break;
		}

		/* display infos on fpgaimage */
		index = 15;
		for (i = 0; i < 4; i++) {
			len = fpgadata[index];
			printf ("FPGA: %s\n", &(fpgadata[index + 1]));
			index += len + 3;
		}
		putc ('\n');
		/* delayed reboot */
		for (i = 20; i > 0; i--) {
			printf ("Rebooting in %2d seconds \r", i);
			for (index = 0; index < 1000; index++)
				udelay (1000);
		}
		putc ('\n');
		do_reset (NULL, 0, 0, NULL);
	}

	/*
	 * Setup port pins for normal operation
	 */
	out32 (GPIO0_ODR, 0x00000000);	/* no open drain pins */
	out32 (GPIO0_TCR, 0x07038100);	/* setup for output */
	out32 (GPIO0_OR, 0x07030100);	/* set output pins to high (default) */

	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) CAN0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) CAN1; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) PCI SLOT 0; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) PCI SLOT 1; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) PCI SLOT 3; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) COMPACT FLASH; active high; level sensitive
	 */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (uicer, 0x00000000);	/* disable all ints */
	mtdcr (uiccr, 0x00000000);	/* set all to be non-critical */
	mtdcr (uicpr, 0xFFFFFF81);	/* set int polarities */
	mtdcr (uictr, 0x10000000);	/* set int trigger levels */
	mtdcr (uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */

	return 0;
}


/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	int index;
	int len;
	char str[64];
	int i = getenv_r ("serial#", str, sizeof (str));

	puts ("Board: ");

	if (!i || strncmp (str, "CANBT", 5)) {
		puts ("### No HW ID - assuming CANBT\n");
		return (0);
	}

	puts (str);

	puts ("\nFPGA:  ");

	/* display infos on fpgaimage */
	index = 15;
	for (i = 0; i < 4; i++) {
		len = fpgadata[index];
		printf ("%s ", &(fpgadata[index + 1]));
		index += len + 3;
	}

	putc ('\n');

	return 0;
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	return (16 * 1024 * 1024);
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: 16 MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */
