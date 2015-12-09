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

#define CMD_PRINTENV	"fw_printenv"
#define CMD_SETENV	"fw_setenv"
static int do_printenv;

static struct option long_options[] = {
	{"aes", required_argument, NULL, 'a'},
	{"config", required_argument, NULL, 'c'},
	{"help", no_argument, NULL, 'h'},
	{"script", required_argument, NULL, 's'},
	{"noheader", required_argument, NULL, 'n'},
	{NULL, 0, NULL, 0}
};

struct common_args common_args;
struct printenv_args printenv_args;
struct setenv_args setenv_args;

void usage_printenv(void)
{

	fprintf(stderr,
		"Usage: fw_printenv [OPTIONS]... [VARIABLE]...\n"
		"Print variables from U-Boot environment\n"
		"\n"
		" -h, --help           print this help.\n"
#ifdef CONFIG_ENV_AES
		" -a, --aes            aes key to access environment\n"
#endif
#ifdef CONFIG_FILE
		" -c, --config         configuration file, default:" CONFIG_FILE "\n"
#endif
		" -n, --noheader       do not repeat variable name in output\n"
		"\n");
}

void usage_setenv(void)
{
	fprintf(stderr,
		"Usage: fw_setenv [OPTIONS]... [VARIABLE]...\n"
		"Modify variables in U-Boot environment\n"
		"\n"
		" -h, --help           print this help.\n"
#ifdef CONFIG_ENV_AES
		" -a, --aes            aes key to access environment\n"
#endif
#ifdef CONFIG_FILE
		" -c, --config         configuration file, default:" CONFIG_FILE "\n"
#endif
		" -s, --script         batch mode to minimize writes\n"
		"\n"
		"Examples:\n"
		"  fw_setenv foo bar   set variable foo equal bar\n"
		"  fw_setenv foo       clear variable foo\n"
		"  fw_setenv --script file run batch script\n"
		"\n"
		"Script Syntax:\n"
		"  key [space] value\n"
		"  lines starting with '#' are treated as commment\n"
		"\n"
		"  A variable without value will be deleted. Any number of spaces are\n"
		"  allowed between key and value. Space inside of the value is treated\n"
		"  as part of the value itself.\n"
		"\n"
		"Script Example:\n"
		"  netdev         eth0\n"
		"  kernel_addr    400000\n"
		"  foo            empty empty empty    empty empty empty\n"
		"  bar\n"
		"\n");
}

static void parse_common_args(int argc, char *argv[])
{
	int c;

#ifdef CONFIG_FILE
	common_args.config_file = CONFIG_FILE;
#endif

	while ((c = getopt_long(argc, argv, ":a:c:h", long_options, NULL)) !=
	       EOF) {
		switch (c) {
		case 'a':
			if (parse_aes_key(optarg, common_args.aes_key)) {
				fprintf(stderr, "AES key parse error\n");
				exit(EXIT_FAILURE);
			}
			common_args.aes_flag = 1;
			break;
#ifdef CONFIG_FILE
		case 'c':
			common_args.config_file = optarg;
			break;
#endif
		case 'h':
			do_printenv ? usage_printenv() : usage_setenv();
			exit(EXIT_SUCCESS);
			break;
		default:
			/* ignore unknown options */
			break;
		}
	}

	/* Reset getopt for the next pass. */
	opterr = 1;
	optind = 1;
}

int parse_printenv_args(int argc, char *argv[])
{
	int c;

	parse_common_args(argc, argv);

	while ((c = getopt_long(argc, argv, "a:c:ns:h", long_options, NULL)) !=
	       EOF) {
		switch (c) {
		case 'n':
			printenv_args.name_suppress = 1;
			break;
		case 'a':
		case 'c':
		case 'h':
			/* ignore common options */
			break;
		default: /* '?' */
			usage_printenv();
			exit(EXIT_FAILURE);
			break;
		}
	}
	return 0;
}

int parse_setenv_args(int argc, char *argv[])
{
	int c;

	parse_common_args(argc, argv);

	while ((c = getopt_long(argc, argv, "a:c:ns:h", long_options, NULL)) !=
	       EOF) {
		switch (c) {
		case 's':
			setenv_args.script_file = optarg;
			break;
		case 'a':
		case 'c':
		case 'h':
			/* ignore common options */
			break;
		default: /* '?' */
			usage_setenv();
			exit(EXIT_FAILURE);
			break;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	const char *lockname = "/var/lock/" CMD_PRINTENV ".lock";
	int lockfd = -1;
	int retval = EXIT_SUCCESS;
	char *_cmdname;

	_cmdname = *argv;
	if (strrchr(_cmdname, '/') != NULL)
		_cmdname = strrchr(_cmdname, '/') + 1;

	if (strcmp(_cmdname, CMD_PRINTENV) == 0) {
		do_printenv = 1;
	} else if (strcmp(_cmdname, CMD_SETENV) == 0) {
		do_printenv = 0;
	} else {
		fprintf(stderr,
			"Identity crisis - may be called as `%s' or as `%s' but not as `%s'\n",
			CMD_PRINTENV, CMD_SETENV, _cmdname);
		exit(EXIT_FAILURE);
	}

	if (do_printenv) {
		if (parse_printenv_args(argc, argv))
			exit(EXIT_FAILURE);
	} else {
		if (parse_setenv_args(argc, argv))
			exit(EXIT_FAILURE);
	}

	/* shift parsed flags, jump to non-option arguments */
	argc -= optind;
	argv += optind;

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

	if (do_printenv) {
		if (fw_printenv(argc, argv) != 0)
			retval = EXIT_FAILURE;
	} else {
		if (!setenv_args.script_file) {
			if (fw_setenv(argc, argv) != 0)
				retval = EXIT_FAILURE;
		} else {
			if (fw_parse_script(setenv_args.script_file) != 0)
				retval = EXIT_FAILURE;
		}
	}

	flock(lockfd, LOCK_UN);
	close(lockfd);
	return retval;
}
