// SPDX-License-Identifier: GPL-2.0
/*
 * Tests for config command
 */

#include <console.h>
#include <test/cmd.h>
#include <test/ut.h>

static int cmd_test_config(struct unit_test_state *uts)
{
	ut_assertok(run_command("config", 0));
	ut_assert_skip_to_line("# Automatically generated file; DO NOT EDIT.");
	ut_assert_skip_to_linen("# Compiler:");
	ut_assert_skip_to_line("CONFIG_CMD_CONFIG=y");

	console_record_reset_enable();

	ut_assertok(run_command("config cmd_config=y", 0));
	ut_assert_nextline("CONFIG_CMD_CONFIG=y");
	ut_assert_console_end();

	ut_assertok(run_command("config 'this string never appears in .config'", 0));
	ut_assert_console_end();

	return 0;
}
CMD_TEST(cmd_test_config, UTF_CONSOLE);
