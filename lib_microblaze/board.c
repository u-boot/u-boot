/*
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Yasushi SHOJI <yashi@atmark-techno.com>
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
#include <command.h>
#include <malloc.h>
#include <version.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")";

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;


void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;
	return ((void *) old);
}

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	serial_init,		/* serial communications setup */
	NULL,
};

void board_init(void)
{
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;

	/* Pointer is writable since we allocated a register for it. */
	gd = (gd_t *)CFG_GBL_DATA_OFFSET;
	memset((void *)gd, 0, CFG_GBL_DATA_SIZE);

	gd->bd = (bd_t *)(gd+1);	/* At end of global data */
	gd->baudrate = CONFIG_BAUDRATE;

	bd = gd->bd;
	bd->bi_baudrate = CONFIG_BAUDRATE;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		WATCHDOG_RESET ();
		if ((*init_fnc_ptr) () != 0) {
			hang ();
		}
	}

	/* main_loop */
	for (;;) {
		WATCHDOG_RESET ();
		main_loop ();
	}
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
