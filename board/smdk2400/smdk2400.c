/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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
#include <s3c2400.h>

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_MODEM_SUPPORT
static int key_pressed(void);
int mdm_init (bd_t *);
extern void disable_putc(void);
extern void enable_putc(void);
extern int hwflow_onoff(int);
extern int do_mdm_init; /* defined in common/main.c */
#endif /* CONFIG_MODEM_SUPPORT */

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* memory and cpu-speed are setup before relocation */
	/* change the clock to be 50 MHz 1:1:1 */
	rMPLLCON = 0x5c042;
	rCLKDIVN = 0;
	/* set up the I/O ports */
	rPACON = 0x3ffff;
	rPBCON = 0xaaaaaaaa;
	rPBUP = 0xffff;
	rPECON = 0x0;
	rPEUP = 0x0;
#ifdef CONFIG_HWFLOW
	/*CTS[0] RTS[0] INPUT INPUT TXD[0] INPUT RXD[0] */
	/*   10,   10,   00,   00,    10,   00,    10 */
	rPFCON=0xa22;
	/* Disable pull-up on Rx, Tx, CTS and RTS pins */
	rPFUP=0x35;
#else
	/*INPUT INPUT INPUT INPUT TXD[0] INPUT RXD[0] */
	/*   00,   00,   00,   00,    10,   00,    10 */
	rPFCON = 0x22;
	/* Disable pull-up on Rx and Tx pins */
	rPFUP = 0x5;
#endif	/* CONFIG_HWFLOW */
	rPGCON = 0x0;
	rPGUP = 0x0;
	rOPENCR = 0x0;

	/* arch number of SAMSUNG-Board to MACH_TYPE_SMDK2400 */
	gd->bd->bi_arch_number = 145;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x0C000100;

#ifdef CONFIG_MODEM_SUPPORT
	if (key_pressed()) {
		disable_putc();	/* modem doesn't understand banner etc */
		do_mdm_init = 1;
	}
#endif	/* CONFIG_MODEM_SUPPORT */

	return 0;
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef CONFIG_MODEM_SUPPORT
static int key_pressed(void)
{
	int rc;
	if (1) {	/* check for button push here, now just return 1 */
		rc = 1;
	}

	return rc;
}
#endif	/* CONFIG_MODEM_SUPPORT */
