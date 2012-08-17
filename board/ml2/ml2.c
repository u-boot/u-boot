/*
 * ml2.c: U-Boot platform support for Xilinx ML2 board
 *
 * Copyright 2002 Mind NV
 *
 * http://www.mind.be/
 *
 * Author : Peter De Schrijver (p2@mind.be)
 *
 * Derived from : Other platform support files in this tree
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License (GPL) version 2, incorporated herein by
 * reference. Drivers based on or derived from this code fall under the GPL
 * and must retain the authorship, copyright and this license notice. This
 * file is not a complete program and may only be used when the entire
 * program is licensed under the GPL.
 *
 */

#include <common.h>
#include <asm/processor.h>


int board_early_init_f (void)
{
	return 0;
}


int checkboard (void)
{
	char buf[64];
	int i;
	int l = getenv_f("serial#", buf, sizeof(buf));

	if (l < 0 || strncmp(buf, "ML2", 9)) {
		printf ("### No HW ID - assuming ML2");
	} else {
		for (i = 0; i < l; i++) {
			if (buf[i] == ' ')
				break;
			putc(buf[i]);
		}
	}
	putc ('\n');

	return (0);
}


phys_size_t initdram (int board_type)
{
	return 32 * 1024 * 1024;
}

int testdram (void)
{
	printf ("test: xxx MB - ok\n");

	return (0);
}
