/*
 * (C) Copyright 2000-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Command line user interface to firmware (=U-Boot) environment.
 *
 * Implements:
 *	fw_printenv [ -a key ] [[ -n name ] | [ name ... ]]
 *              - prints the value of a single environment variable
 *                "name", the ``name=value'' pairs of one or more
 *                environment variables "name", or the whole
 *                environment if no names are specified.
 *	fw_setenv [ -a key ] name [ value ... ]
 *		- If a name without any values is given, the variable
 *		  with this name is deleted from the environment;
 *		  otherwise, all "value" arguments are concatenated,
 *		  separated by single blank characters, and the
 *		  resulting string is assigned to the environment
 *		  variable "name"
 *
 * If '-a key' is specified, the env block is encrypted with AES 128 CBC.
 * The 'key' argument is in the format of 32 hexadecimal numbers (16 bytes
 * of AES key), eg. '-a aabbccddeeff00112233445566778899'.
 */

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>
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
		"usage:\tfw_printenv [-a key] [-n] [variable name]\n"
		"\tfw_setenv [-a key] [variable name] [variable value]\n"
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

int main(int argc, char *argv[])
{
	char *p;
	char *cmdname = *argv;
	char *script_file = NULL;
	int c;
	const char *lockname = "/var/lock/" CMD_PRINTENV ".lock";
	int lockfd = -1;
	int retval = EXIT_SUCCESS;

	lockfd = open(lockname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (-1 == lockfd) {
		fprintf(stderr, "Error opening lock file %s\n", lockname);
		return EXIT_FAILURE;
	}

	if (-1 == flock(lockfd, LOCK_EX)) {
		fprintf(stderr, "Error locking file %s\n", lockname);
		close(lockfd);
		return EXIT_FAILURE;
	}

	if ((p = strrchr (cmdname, '/')) != NULL) {
		cmdname = p + 1;
	}

	while ((c = getopt_long (argc, argv, "a:ns:h",
		long_options, NULL)) != EOF) {
		switch (c) {
		case 'a':
			/* AES key, handled later */
			break;
		case 'n':
			/* handled in fw_printenv */
			break;
		case 's':
			script_file = optarg;
			break;
		case 'h':
			usage();
			goto exit;
		default: /* '?' */
			fprintf(stderr, "Try `%s --help' for more information."
				"\n", cmdname);
			retval = EXIT_FAILURE;
			goto exit;
		}
	}

	if (strcmp(cmdname, CMD_PRINTENV) == 0) {
		if (fw_printenv(argc, argv) != 0)
			retval = EXIT_FAILURE;
	} else if (strcmp(cmdname, CMD_SETENV) == 0) {
		if (!script_file) {
			if (fw_setenv(argc, argv) != 0)
				retval = EXIT_FAILURE;
		} else {
			if (fw_parse_script(script_file) != 0)
				retval = EXIT_FAILURE;
		}
	} else {
		fprintf(stderr,
			"Identity crisis - may be called as `" CMD_PRINTENV
			"' or as `" CMD_SETENV "' but not as `%s'\n",
			cmdname);
		retval = EXIT_FAILURE;
	}

exit:
	flock(lockfd, LOCK_UN);
	close(lockfd);
	return retval;
}
