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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Command line user interface to firmware (=U-Boot) environment.
 *
 * Implements:
 *	fw_printenv [ name ... ]
 *		- prints the values of the environment variables
 *		  "name", or the whole environment if no names are
 *		  specified
 *	fw_setenv name [ value ... ]
 *		- If a name without any values is given, the variable
 *		  with this name is deleted from the environment;
 *		  otherwise, all "value" arguments are concatenated,
 *		  separated by sinlge blank characters, and the
 *		  resulting string is assigned to the environment
 *		  variable "name"
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fw_env.h"

#define	CMD_PRINTENV	"fw_printenv"
#define CMD_SETENV	"fw_setenv"

int
main(int argc, char *argv[])
{
	char *p;
	char *cmdname = *argv;

	if ((p = strrchr (cmdname, '/')) != NULL) {
		cmdname = p + 1;
	}

	if (strcmp(cmdname, CMD_PRINTENV) == 0) {

			fw_printenv (argc, argv);

			return (EXIT_SUCCESS);

	} else if (strcmp(cmdname, CMD_SETENV) == 0) {

			if (fw_setenv (argc, argv) != 0)
				return (EXIT_FAILURE);

			return (EXIT_SUCCESS);
	}

	fprintf (stderr,
		"Identity crisis - may be called as `" CMD_PRINTENV
		"' or as `" CMD_SETENV "' but not as `%s'\n",
		cmdname);
	return (EXIT_FAILURE);
}
