/* U-Boot - bf525-ucr2.c  board specific routines
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>

int checkboard(void)
{
	printf("Board: bf525-ucr2\n");
	printf("Support: http://www.ucrobotics.com/\n");
	return 0;
}
