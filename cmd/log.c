/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <log.h>

static int do_log_level(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	if (argc > 1)
		gd->default_log_level = simple_strtol(argv[1], NULL, 10);
	else
		printf("Default log level: %d\n", gd->default_log_level);

	return 0;
}

static cmd_tbl_t log_sub[] = {
	U_BOOT_CMD_MKENT(level, CONFIG_SYS_MAXARGS, 1, do_log_level, "", ""),
#ifdef CONFIG_LOG_TEST
	U_BOOT_CMD_MKENT(test, 2, 1, do_log_test, "", ""),
#endif
};

static int do_log(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "log" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], log_sub, ARRAY_SIZE(log_sub));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char log_help_text[] =
	"level - get/set log level\n"
#ifdef CONFIG_LOG_TEST
	"log test - run log tests\n"
#endif
	;
#endif

U_BOOT_CMD(
	log, CONFIG_SYS_MAXARGS, 1, do_log,
	"log system", log_help_text
);
