/*
 * Copyright (c) 2012, The Chromium Authors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define DEBUG

#include <common.h>

static const char test_cmd[] = "setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3\0"
		"setenv list ${list}4";

static int do_ut_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("%s: Testing commands\n", __func__);
	run_command("env default -f", 0);

	/* run a single command */
	run_command("setenv single 1", 0);
	assert(!strcmp("1", getenv("single")));

	/* make sure that compound statements work */
#ifdef CONFIG_SYS_HUSH_PARSER
	run_command("if test -n ${single} ; then setenv check 1; fi", 0);
	assert(!strcmp("1", getenv("check")));
	run_command("setenv check", 0);
#endif

	/* commands separated by ; */
	run_command_list("setenv list 1; setenv list ${list}1", -1, 0);
	assert(!strcmp("11", getenv("list")));

	/* commands separated by \n */
	run_command_list("setenv list 1\n setenv list ${list}1", -1, 0);
	assert(!strcmp("11", getenv("list")));

	/* command followed by \n and nothing else */
	run_command_list("setenv list 1${list}\n", -1, 0);
	assert(!strcmp("111", getenv("list")));

	/* three commands in a row */
	run_command_list("setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3", -1, 0);
	assert(!strcmp("123", getenv("list")));

	/* a command string with \0 in it. Stuff after \0 should be ignored */
	run_command("setenv list", 0);
	run_command_list(test_cmd, sizeof(test_cmd), 0);
	assert(!strcmp("123", getenv("list")));

	/*
	 * a command list where we limit execution to only the first command
	 * using the length parameter.
	 */
	run_command_list("setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3", strlen("setenv list 1"), 0);
	assert(!strcmp("1", getenv("list")));

	printf("%s: Everything went swimmingly\n", __func__);
	return 0;
}

U_BOOT_CMD(
	ut_cmd,	5,	1,	do_ut_cmd,
	"Very basic test of command parsers",
	""
);
