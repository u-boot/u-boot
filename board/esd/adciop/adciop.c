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
#include "adciop.h"

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

/* ------------------------------------------------------------------------- */


int board_early_init_f (void)
{
	/*
	 * Set port pin in escc2 to keep living, and configure user led output
	 */
	*(unsigned char *) 0x2000033e = 0x77;	/* ESCC2: PCR bit3=pwr on, bit7=led out */
	*(unsigned char *) 0x2000033c = 0x88;	/* ESCC2: PVR pwr on, led off */

	/*
	 * Init pci regs
	 */
	*(unsigned long *) 0x50000304 = 0x02900007;	/* enable mem/io/master bits */
	*(unsigned long *) 0x500001b4 = 0x00000000;	/* disable pci interrupt output enable */
	*(unsigned long *) 0x50000354 = 0x00c05800;	/* disable emun interrupt output enable */
	*(unsigned long *) 0x50000344 = 0x00000000;	/* disable pme interrupt output enable */
	*(unsigned long *) 0x50000310 = 0x00000000;	/* pcibar0 */
	*(unsigned long *) 0x50000314 = 0x00000000;	/* pcibar1 */
	*(unsigned long *) 0x50000318 = 0x00000000;	/* pcibar2 */

	return 0;
}


/*
 * Check Board Identity:
 */

int checkboard (void)
{
	char str[64];
	int i = getenv_r ("serial#", str, sizeof (str));

	puts ("Board: ");

	if (!i || strncmp (str, "ADCIOP", 6)) {
		puts ("### No HW ID - assuming ADCIOP\n");
		return (1);
	}

	puts (str);

	putc ('\n');

	return 0;
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
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
