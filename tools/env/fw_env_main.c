/*
 * (C) Copyright 2000-2008
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
 *	fw_printenv [[ -n name ] | [ name ... ]]
 *              - prints the value of a single environment variable
 *                "name", the ``name=value'' pairs of one or more
 *                environment variables "name", or the whole
 *                environment if no names are specified.
 *	fw_setenv name [ value ... ]
 *		- If a name without any values is given, the variable
 *		  with this name is deleted from the environment;
 *		  otherwise, all "value" arguments are concatenated,
 *		  separated by single blank characters, and the
 *		  resulting string is assigned to the environment
 *		  variable "name"
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "fw_env.h"

#define	CMD_PRINTENV	"fw_printenv"
#define CMD_SETENV	"fw_setenv"

static struct option long_options[] = {
	{"script", required_argument, NULL, 's'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0}
};

void usage(void)
{

	fprintf(stderr, "fw_printenv/fw_setenv, "
		"a command line interface to U-Boot environment\n\n"
		"usage:\tfw_printenv [-n] [variable name]\n"
		"\tfw_setenv [variable name] [variable value]\n"
		"\tfw_setenv -s [ file ]\n"
		"\tfw_setenv -s - < [ file ]\n\n"
		"The file passed as argument contains only pairs "
		"name / value\n"
		"Example:\n"
		"# Any line starting with # is treated as comment\n"
		"\n"
		"\t      netdev         eth0\n"
		"\t      kernel_addr    400000\n"
		"\t      var1\n"
		"\t      var2          The quick brown fox jumps over the "
		"lazy dog\n"
		"\n"
		"A variable without value will be dropped. It is possible\n"
		"to put any number of spaces between the fields, but any\n"
		"space inside the value is treated as part of the value "
		"itself.\n\n"
	);
}

int
main(int argc, char *argv[])
{
	char *p;
	char *cmdname = *argv;
	char *script_file = NULL;
	int c;

	if ((p = strrchr (cmdname, '/')) != NULL) {
		cmdname = p + 1;
	}

	while ((c = getopt_long (argc, argv, "ns:h",
		long_options, NULL)) != EOF) {
		switch (c) {
		case 'n':
			/* handled in fw_printenv */
			break;
		case 's':
			script_file = optarg;
			break;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default: /* '?' */
			fprintf(stderr, "Try `%s --help' for more information."
				"\n", cmdname);
			return EXIT_FAILURE;
		}
	}


	if (strcmp(cmdname, CMD_PRINTENV) == 0) {

		if (fw_printenv (argc, argv) != 0)
			return EXIT_FAILURE;

		return EXIT_SUCCESS;

	} else if (strcmp(cmdname, CMD_SETENV) == 0) {
		if (!script_file) {
			if (fw_setenv(argc, argv) != 0)
				return EXIT_FAILURE;
		} else {
			if (fw_parse_script(script_file) != 0)
				return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;

	}

	fprintf (stderr,
		"Identity crisis - may be called as `" CMD_PRINTENV
		"' or as `" CMD_SETENV "' but not as `%s'\n",
		cmdname);
	return EXIT_FAILURE;
}
