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

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <devices.h>

#if defined(CONFIG_CMD_CONSOLE)

extern void _do_coninfo (void);
int do_coninfo (cmd_tbl_t * cmd, int flag, int argc, char *argv[])
{
	int i, l;

	/* Scan for valid output and input devices */

	puts ("List of available devices:\n");

	for (i = 1; i <= ListNumItems (devlist); i++) {
		device_t *dev = ListGetPtrToItem (devlist, i);

		printf ("%-8s %08x %c%c%c ",
			dev->name,
			dev->flags,
			(dev->flags & DEV_FLAGS_SYSTEM) ? 'S' : '.',
			(dev->flags & DEV_FLAGS_INPUT) ? 'I' : '.',
			(dev->flags & DEV_FLAGS_OUTPUT) ? 'O' : '.');

		for (l = 0; l < MAX_FILES; l++) {
			if (stdio_devices[l] == dev) {
				printf ("%s ", stdio_names[l]);
			}
		}
		putc ('\n');
	}
	return 0;
}


/***************************************************/

U_BOOT_CMD(
	coninfo,	3,	1,	do_coninfo,
	"coninfo - print console devices and information\n",
	""
);

#endif /* CFG_CMD_CONSOLE */
