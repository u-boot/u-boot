/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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
#if	defined(CONFIG_SEVENSEG)
#include "../common/sevenseg.h"
#endif

void _default_hdlr (void)
{
	printf ("default_hdlr\n");
}

int board_pre_init (void)
{
	/* init seven segment led display and switch off */
	sevenseg_set(SEVENSEG_OFF);
	return 0;
}

int checkboard (void)
{
	puts ("Board: Altera Nios 1S10 Development Kit\n");
	return 0;
}

long int initdram (int board_type)
{
	return (0);
}
