// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the bootd command
 *
 * Copyright 2026 Mehmet Fide <mehmet.fide@screeningeagle.com>
 */

#include <command.h>
#include <console.h>
#include <env.h>
#include <malloc.h>
#include <test/cmd.h>
#include <test/test.h>
#include <test/ut.h>

static int cmd_bootd_test(struct unit_test_state *uts)
{
	char *const argv[] = { "bootd", NULL };
	const char *old = env_get("bootcmd");
	char *saved = NULL;
	int repeatable = 0;

	if (old) {
		saved = strdup(old);
		ut_assertnonnull(saved);
	}

	/* bootd runs the command held in bootcmd */
	ut_assertok(env_set("bootcmd", "echo hello bootd"));
	ut_assertok(run_command("bootd", 0));
	ut_assert_nextline("hello bootd");
	ut_assert_console_end();

	/* the "boot" alias does the same */
	ut_assertok(run_command("boot", 0));
	ut_assert_nextline("hello bootd");
	ut_assert_console_end();

	/* the return value is the one of the command in bootcmd */
	if (IS_ENABLED(CONFIG_HUSH_PARSER)) {
		ut_assertok(env_set("bootcmd", "false"));
		ut_asserteq(1, run_command("bootd", 0));
		ut_assert_console_end();
	}

	/* a bootd reached from bootd is refused rather than recursing */
	ut_assertok(env_set("bootcmd", "echo bootcmd must not run"));
	ut_asserteq(CMD_RET_FAILURE,
		    cmd_process(CMD_FLAG_BOOTD, 1, argv, &repeatable, NULL));
	ut_assert_nextline("'bootd' recursion detected");
	ut_assert_console_end();

	ut_assertok(env_set("bootcmd", saved));
	free(saved);

	return 0;
}
CMD_TEST(cmd_bootd_test, UTF_CONSOLE);
