// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

static int do_ut_all(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[]);

int cmd_ut_category(const char *name, const char *prefix,
		    struct unit_test *tests, int n_ents,
		    int argc, char *const argv[])
{
	int ret;

	ret = ut_run_list(name, prefix, tests, n_ents,
			  argc > 1 ? argv[1] : NULL);

	return ret ? CMD_RET_FAILURE : 0;
}

static struct cmd_tbl cmd_ut_sub[] = {
	U_BOOT_CMD_MKENT(all, CONFIG_SYS_MAXARGS, 1, do_ut_all, "", ""),
#if defined(CONFIG_UT_DM)
	U_BOOT_CMD_MKENT(dm, CONFIG_SYS_MAXARGS, 1, do_ut_dm, "", ""),
#endif
#if defined(CONFIG_UT_ENV)
	U_BOOT_CMD_MKENT(env, CONFIG_SYS_MAXARGS, 1, do_ut_env, "", ""),
#endif
#ifdef CONFIG_UT_OPTEE
	U_BOOT_CMD_MKENT(optee, CONFIG_SYS_MAXARGS, 1, do_ut_optee, "", ""),
#endif
#ifdef CONFIG_UT_OVERLAY
	U_BOOT_CMD_MKENT(overlay, CONFIG_SYS_MAXARGS, 1, do_ut_overlay, "", ""),
#endif
#ifdef CONFIG_UT_LIB
	U_BOOT_CMD_MKENT(lib, CONFIG_SYS_MAXARGS, 1, do_ut_lib, "", ""),
#endif
#ifdef CONFIG_UT_LOG
	U_BOOT_CMD_MKENT(log, CONFIG_SYS_MAXARGS, 1, do_ut_log, "", ""),
#endif
	U_BOOT_CMD_MKENT(mem, CONFIG_SYS_MAXARGS, 1, do_ut_mem, "", ""),
#ifdef CONFIG_CMD_SETEXPR
	U_BOOT_CMD_MKENT(setexpr, CONFIG_SYS_MAXARGS, 1, do_ut_setexpr, "",
			 ""),
#endif
#ifdef CONFIG_UT_TIME
	U_BOOT_CMD_MKENT(time, CONFIG_SYS_MAXARGS, 1, do_ut_time, "", ""),
#endif
#if CONFIG_IS_ENABLED(UT_UNICODE) && !defined(API_BUILD)
	U_BOOT_CMD_MKENT(unicode, CONFIG_SYS_MAXARGS, 1, do_ut_unicode, "", ""),
#endif
#ifdef CONFIG_SANDBOX
	U_BOOT_CMD_MKENT(compression, CONFIG_SYS_MAXARGS, 1, do_ut_compression,
			 "", ""),
	U_BOOT_CMD_MKENT(bloblist, CONFIG_SYS_MAXARGS, 1, do_ut_bloblist,
			 "", ""),
	U_BOOT_CMD_MKENT(bootm, CONFIG_SYS_MAXARGS, 1, do_ut_bootm, "", ""),
#endif
	U_BOOT_CMD_MKENT(str, CONFIG_SYS_MAXARGS, 1, do_ut_str, "", ""),
#ifdef CONFIG_CMD_ADDRMAP
	U_BOOT_CMD_MKENT(addrmap, CONFIG_SYS_MAXARGS, 1, do_ut_addrmap, "", ""),
#endif
};

static int do_ut_all(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
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

static int do_ut(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

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
#ifdef CONFIG_SANDBOX
	"ut bloblist - Test bloblist implementation\n"
	"ut compression - Test compressors and bootm decompression\n"
#endif
#ifdef CONFIG_UT_DM
	"ut dm [test-name]\n"
#endif
#ifdef CONFIG_UT_ENV
	"ut env [test-name]\n"
#endif
#ifdef CONFIG_UT_LIB
	"ut lib [test-name] - test library functions\n"
#endif
#ifdef CONFIG_UT_LOG
	"ut log [test-name] - test logging functions\n"
#endif
	"ut mem [test-name] - test memory-related commands\n"
#ifdef CONFIG_UT_OPTEE
	"ut optee [test-name]\n"
#endif
#ifdef CONFIG_UT_OVERLAY
	"ut overlay [test-name]\n"
#endif
	"ut setexpr [test-name] - test setexpr command\n"
#ifdef CONFIG_SANDBOX
	"ut str - Basic test of string functions\n"
#endif
#ifdef CONFIG_UT_TIME
	"ut time - Very basic test of time functions\n"
#endif
#if defined(CONFIG_UT_UNICODE) && \
	!defined(CONFIG_SPL_BUILD) && !defined(API_BUILD)
	"ut unicode [test-name] - test Unicode functions\n"
#endif
#ifdef CONFIG_CMD_ADDRMAP
	"ut addrmap - Very basic test of addrmap command\n"
#endif
	;
#endif /* CONFIG_SYS_LONGHELP */

U_BOOT_CMD(
	ut, CONFIG_SYS_MAXARGS, 1, do_ut,
	"unit tests", ut_help_text
);
