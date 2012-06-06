/*
 * (C) Copyright 2005
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
#include <command.h>
#include <led-display.h>

#undef DEBUG_DISP

int do_display (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	/* Clear display */
	display_set(DISPLAY_CLEAR | DISPLAY_HOME);

	if (argc < 2)
		return (0);

	for (i = 1; i < argc; i++) {
		char *p = argv[i];

		if (i > 1) { /* Insert a space between strings */
			display_putc(' ');
		}

		while ((*p)) {
#ifdef DEBUG_DISP
			putc(*p);
#endif
			display_putc(*p++);
		}
	}

#ifdef DEBUG_DISP
	putc('\n');
#endif

	return (0);
}

/***************************************************/

U_BOOT_CMD(
	display,	CONFIG_SYS_MAXARGS,	1,	do_display,
	"display string on dot matrix display",
	"[<string>]\n"
	"    - with <string> argument: display <string> on dot matrix display\n"
	"    - without arguments: clear dot matrix display"
);
