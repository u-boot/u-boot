/*
 * (C) Copyright 2002
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
 * Diagnostics support
 */
#include <common.h>
#include <command.h>
#include <post.h>

int do_diag (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned int i;

	if (argc == 1 || strcmp (argv[1], "run") != 0) {
		/* List test info */
		if (argc == 1) {
			puts ("Available hardware tests:\n");
			post_info (NULL);
			puts ("Use 'diag [<test1> [<test2> ...]]'"
					" to get more info.\n");
			puts ("Use 'diag run [<test1> [<test2> ...]]'"
					" to run tests.\n");
		} else {
			for (i = 1; i < argc; i++) {
			    if (post_info (argv[i]) != 0)
				printf ("%s - no such test\n", argv[i]);
			}
		}
	} else {
		/* Run tests */
		if (argc == 2) {
			post_run (NULL, POST_RAM | POST_MANUAL);
		} else {
			for (i = 2; i < argc; i++) {
			    if (post_run (argv[i], POST_RAM | POST_MANUAL) != 0)
				printf ("%s - unable to execute the test\n",
					argv[i]);
			}
		}
	}

	return 0;
}
/***************************************************/

U_BOOT_CMD(
	diag,	CONFIG_SYS_MAXARGS,	0,	do_diag,
	"perform board diagnostics",
	     "    - print list of available tests\n"
	"diag [test1 [test2]]\n"
	"         - print information about specified tests\n"
	"diag run - run all available tests\n"
	"diag run [test1 [test2]]\n"
	"         - run specified tests"
);
