/*
 * (C) Copyright 2003
 * Murray Jensen, CSIRO-MIT, <Murray.Jensen@csiro.au>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

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

			/* see if it looks like an ethernet address */

			p = console_buffer;

			for (i = 0; i < 6; i++) {
				char term = (i == 5 ? '\0' : ':');

				(void)simple_strtol (p, &q, 16);

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
