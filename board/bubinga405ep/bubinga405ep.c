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
long int spd_sdram (void);

#include <common.h>
#include <asm/processor.h>


int board_early_init_f (void)
{
	mtdcr (uicsr, 0xFFFFFFFF);      /* clear all ints */
	mtdcr (uicer, 0x00000000);      /* disable all ints */
	mtdcr (uiccr, 0x00000010);
	mtdcr (uicpr, 0xFFFF7FF0);      /* set int polarities */
	mtdcr (uictr, 0x00000010);      /* set int trigger levels */
	mtdcr (uicsr, 0xFFFFFFFF);      /* clear all ints */

#if 0
#define mtebc(reg, data)  mtdcr(ebccfga,reg);mtdcr(ebccfgd,data)
    /* CS1 */
	/* BAS=0xF00,BS=0x0(1MB),BU=0x3(R/W),BW=0x0( 8 bits) */
	mtebc (pb1ap, 0x02815480);
	mtebc (pb1cr, 0xF0018000);

	p = (unsigned int*)0xEF600708;
	t = *p;
	t = t | 0x00000400;
	*p = t;

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

#endif

	return 0;
}


/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	unsigned char *s = getenv ("serial#");

	puts ("Board: IBM 405EP Eval Board");

	if (s != NULL) {
		puts (", serial# ");
		puts (s);
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
	long int ret;

	ret = spd_sdram ();
	return ret;
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: xxx MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */
