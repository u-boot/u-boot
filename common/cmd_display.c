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

#undef DEBUG_DISP

#define DISP_SIZE	8
#define CWORD_CLEAR	0x80
#define CLEAR_DELAY	(110 * 2)

int do_display (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int pos;

	/* Clear display */
	*((volatile char*)(CONFIG_SYS_DISP_CWORD)) = CWORD_CLEAR;
	udelay(1000 * CLEAR_DELAY);

	if (argc < 2)
		return (0);

	for (pos = 0, i = 1; i < argc && pos < DISP_SIZE; i++) {
		char *p = argv[i], c;

		if (i > 1) {
			*((volatile uchar *) (CONFIG_SYS_DISP_CHR_RAM + pos++)) = ' ';
#ifdef DEBUG_DISP
			putc(' ');
#endif
		}

		while ((c = *p++) != '\0' && pos < DISP_SIZE) {
			*((volatile uchar *) (CONFIG_SYS_DISP_CHR_RAM + pos++)) = c;
#ifdef DEBUG_DISP
			putc(c);
#endif
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
