/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
