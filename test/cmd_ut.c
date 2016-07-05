/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <command.h>
#include <test/suites.h>

static int do_ut_all(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

static cmd_tbl_t cmd_ut_sub[] = {
	U_BOOT_CMD_MKENT(all, CONFIG_SYS_MAXARGS, 1, do_ut_all, "", ""),
#if defined(CONFIG_UT_DM)
	U_BOOT_CMD_MKENT(dm, CONFIG_SYS_MAXARGS, 1, do_ut_dm, "", ""),
#endif
#if defined(CONFIG_UT_ENV)
	U_BOOT_CMD_MKENT(env, CONFIG_SYS_MAXARGS, 1, do_ut_env, "", ""),
#endif
#ifdef CONFIG_UT_OVERLAY
	U_BOOT_CMD_MKENT(overlay, CONFIG_SYS_MAXARGS, 1, do_ut_overlay, "", ""),
#endif
#ifdef CONFIG_UT_TIME
	U_BOOT_CMD_MKENT(time, CONFIG_SYS_MAXARGS, 1, do_ut_time, "", ""),
#endif
};

static int do_ut_all(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int retval;
	int any_fail = 0;

	for (i = 1; i < ARRAY_SIZE(cmd_ut_sub); i++) {
		printf("----Running %s tests----\n", cmd_ut_sub[i].name);
		retval = cmd_ut_sub[i].cmd(cmdtp, flag, 1, &cmd_ut_sub[i].name);
		if (!any_fail)
			any_fail = retval;
	}

	return any_fail;
}

static int do_ut(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "ut" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_ut_sub, ARRAY_SIZE(cmd_ut_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char ut_help_text[] =
	"all - execute all enabled tests\n"
#ifdef CONFIG_UT_DM
	"ut dm [test-name]\n"
#endif
#ifdef CONFIG_UT_ENV
	"ut env [test-name]\n"
#endif
#ifdef CONFIG_UT_OVERLAY
	"ut overlay [test-name]\n"
#endif
#ifdef CONFIG_UT_TIME
	"ut time - Very basic test of time functions\n"
#endif
	;
#endif

U_BOOT_CMD(
	ut, CONFIG_SYS_MAXARGS, 1, do_ut,
	"unit tests", ut_help_text
);
