/*
 * (C) Copyright 2003
 * Murray Jensen, CSIRO-MIT, <Murray.Jensen@csiro.au>
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

/* imports from common/main.c */
extern char console_buffer[CFG_CBSIZE];

int
hymod_get_serno (const char *prompt)
{
	for (;;) {
		int n, serno;
		char *p;

#ifdef CONFIG_BOOT_RETRY_TIME
		reset_cmd_timeout ();
#endif

		n = readline (prompt);

		if (n < 0)
			return (n);

		if (n == 0)
			continue;

		serno = (int) simple_strtol (console_buffer, &p, 10);

		if (p > console_buffer && *p == '\0' && serno > 0)
			return (serno);

		printf ("Invalid number (%s) - please re-enter\n",
			console_buffer);
	}
}

int
hymod_get_ethaddr (void)
{
	for (;;) {
		int n;

#ifdef CONFIG_BOOT_RETRY_TIME
		reset_cmd_timeout ();
#endif

		n = readline ("Enter board ethernet address: ");

		if (n < 0)
			return (n);

		if (n == 0)
			continue;

		if (n == 17) {
			int i;
			char *p, *q;
			uchar ea[6];

			/* see if it looks like an ethernet address */

			p = console_buffer;

			for (i = 0; i < 6; i++) {
				char term = (i == 5 ? '\0' : ':');

				ea[i] = simple_strtol (p, &q, 16);

				if ((q - p) != 2 || *q++ != term)
					break;

				p = q;
			}

			if (i == 6) {
				/* it looks ok - set it */
				printf ("Setting ethernet addr to %s\n",
					console_buffer);

				setenv ("ethaddr", console_buffer);

				puts ("Remember to do a 'saveenv' to "
					"make it permanent\n");

				return (0);
			}
		}

		printf ("Invalid ethernet addr (%s) - please re-enter\n",
			console_buffer);
	}
}
