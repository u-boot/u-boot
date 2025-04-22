// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, The Chromium Authors
 */

#define DEBUG

#include <command.h>
#include <env.h>
#include <log.h>
#include <string.h>
#include <linux/errno.h>
#include <test/cmd.h>
#include <test/ut.h>

static const char test_cmd[] = "setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3\0"
		"setenv list ${list}4";

static int command_test(struct unit_test_state *uts)
{
	char long_str[CONFIG_SYS_CBSIZE + 42];

	printf("%s: Testing commands\n", __func__);
	run_command("env default -f -a", 0);

	/* commands separated by \n */
	run_command_list("setenv list 1\n setenv list ${list}1", -1, 0);
	ut_assert(!strcmp("11", env_get("list")));

	/* command followed by \n and nothing else */
	run_command_list("setenv list 1${list}\n", -1, 0);
	ut_assert(!strcmp("111", env_get("list")));

	/* a command string with \0 in it. Stuff after \0 should be ignored */
	run_command("setenv list", 0);
	run_command_list(test_cmd, sizeof(test_cmd), 0);
	ut_assert(!strcmp("123", env_get("list")));

	/*
	 * a command list where we limit execution to only the first command
	 * using the length parameter.
	 */
	run_command_list("setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3", strlen("setenv list 1"), 0);
	ut_assert(!strcmp("1", env_get("list")));

	ut_assertok(run_command("echo", 0));
	ut_assertok(run_command_list("echo", -1, 0));

	if (IS_ENABLED(CONFIG_HUSH_PARSER)) {
		ut_asserteq(1, run_command("false", 0));
		ut_asserteq(1, run_command_list("false", -1, 0));
		run_command("setenv foo 'setenv black 1\nsetenv adder 2'", 0);
		run_command("run foo", 0);
		ut_assertnonnull(env_get("black"));
		ut_asserteq(0, strcmp("1", env_get("black")));
		ut_assertnonnull(env_get("adder"));
		ut_asserteq(0, strcmp("2", env_get("adder")));
		ut_assertok(run_command("", 0));
		ut_assertok(run_command(" ", 0));
	}

	ut_asserteq(1, run_command("'", 0));

	/* Variadic function test-cases */
	if (IS_ENABLED(CONFIG_HUSH_PARSER)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
		ut_assertok(run_commandf(""));
#pragma GCC diagnostic pop
		ut_assertok(run_commandf(" "));
	}
	ut_asserteq(1, run_commandf("'"));

	ut_assertok(run_commandf("env %s %s", "delete -f", "list"));
	/*
	 * Expected: "## Error: "list" not defined"
	 * (disabled to avoid pytest bailing out)
	 *
	 * ut_asserteq(1, run_commandf("printenv list"));
	 */

	memset(long_str, 'x', sizeof(long_str));
	ut_asserteq(-ENOSPC, run_commandf("Truncation case: %s", long_str));

	if (IS_ENABLED(CONFIG_HUSH_PARSER)) {
		ut_assertok(run_commandf("env %s %s %s %s", "delete -f",
					 "adder", "black", "foo"));
		ut_assertok(run_commandf(
			"setenv foo 'setenv %s 1\nsetenv %s 2'",
			"black", "adder"));
		ut_assertok(run_command("run foo", 0));
		ut_assertnonnull(env_get("black"));
		ut_asserteq(0, strcmp("1", env_get("black")));
		ut_assertnonnull(env_get("adder"));
		ut_asserteq(0, strcmp("2", env_get("adder")));
	}

	/* Clean up before exit */
	ut_assertok(run_command("env default -f -a", 0));

	/* put back the FDT environment */
	ut_assertok(env_set("from_fdt", "yes"));

	printf("%s: Everything went swimmingly\n", __func__);
	return 0;
}
CMD_TEST(command_test, 0);
