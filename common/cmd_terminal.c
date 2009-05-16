/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
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
#include <stdio_dev.h>
#include <serial.h>

int do_terminal(cmd_tbl_t * cmd, int flag, int argc, char *argv[])
{
	int last_tilde = 0;
	struct stdio_dev *dev = NULL;

	if (argc < 1)
		return -1;

	/* Scan for selected output/input device */
	dev = stdio_get_by_name(argv[1]);
	if (!dev)
		return -1;

	serial_reinit_all();
	printf("Entering terminal mode for port %s\n", dev->name);
	puts("Use '~.' to leave the terminal and get back to u-boot\n");

	while (1) {
		int c;

		/* read from console and display on serial port */
		if (stdio_devices[0]->tstc()) {
			c = stdio_devices[0]->getc();
			if (last_tilde == 1) {
				if (c == '.') {
					putc(c);
					putc('\n');
					break;
				} else {
					last_tilde = 0;
					/* write the delayed tilde */
					dev->putc('~');
					/* fall-through to print current
					 * character */
				}
			}
			if (c == '~') {
				last_tilde = 1;
				puts("[u-boot]");
				putc(c);
			}
			dev->putc(c);
		}

		/* read from serial port and display on console */
		if (dev->tstc()) {
			c = dev->getc();
			putc(c);
		}
	}
	return 0;
}


/***************************************************/

U_BOOT_CMD(
	terminal,	3,	1,	do_terminal,
	"start terminal emulator",
	""
);
