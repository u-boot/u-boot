/*
 * (C) Copyright 2005-2008 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Derived from x-loader
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
#include <version.h>

#include "onenand_ipl.h"

#ifdef CFG_PRINTF
int print_info(void)
{
	printf(XLOADER_VERSION);

	return 0;
}
#endif

typedef int (init_fnc_t)(void);

init_fnc_t *init_sequence[] = {
	board_init,		/* basic board dependent setup */
#ifdef CFG_PRINTF
	serial_init,		/* serial communications setup */
	print_info,
#endif
	NULL,
};

void start_oneboot(void)
{
	init_fnc_t **init_fnc_ptr;
	uchar *buf;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0)
			hang();
	}

	buf = (uchar *) CFG_LOAD_ADDR;

	if (!onenand_read_block0(buf))
		buf += ONENAND_BLOCK_SIZE;

	if (buf == (uchar *)CFG_LOAD_ADDR)
		hang();

	/* go run U-Boot and never return */
	printf("Starting OS Bootloader...\n");
	((init_fnc_t *)CFG_LOAD_ADDR)();

	/* should never come here */
}

void hang(void)
{
	/* if board_hang() returns, hange here */
	printf("X-Loader hangs\n");
	for (;;);
}
