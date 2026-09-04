// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for 'pr' (Altera FPGA partial reconfiguration) command
 *
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#include <test/cmd.h>
#include <test/ut.h>

/* Check usage output from CMD_RET_USAGE */
static int check_pr_usage(struct unit_test_state *uts)
{
	ut_assert_nextline("pr - SoCFPGA partial reconfiguration control");
	if (IS_ENABLED(CONFIG_SYS_LONGHELP)) {
		ut_assert_nextline_empty();
		ut_assert_nextline("Usage:");
		ut_assert_nextline("pr start [region] - Start the partial reconfiguration by freeze the PR region");
		ut_assert_nextline("end [region] - End the partial reconfiguration by unfreeze the PR region");
		ut_assert_nextline_empty();
	}
	ut_assert_console_end();

	return 0;
}

/* Test 'pr' usage with missing or invalid arguments */
static int cmd_test_pr_usage(struct unit_test_state *uts)
{
	/* No arguments */
	ut_asserteq(1, run_command("pr", 0));
	ut_assertok(check_pr_usage(uts));

	/* Too many arguments */
	ut_asserteq(1, run_command("pr start 0 extra", 0));
	ut_assertok(check_pr_usage(uts));

	/* Invalid subcommand */
	ut_asserteq(1, run_command("pr bogus", 0));
	ut_assertok(check_pr_usage(uts));

	return 0;
}

CMD_TEST(cmd_test_pr_usage, UTF_CONSOLE);

/* Test 'pr start/end' when freeze bridge is not in the device tree */
static int cmd_test_pr_no_freeze_br(struct unit_test_state *uts)
{
	/* Default region 0 */
	ut_assert(run_command("pr start", 0));
	ut_assert_nextline("alias freeze_br0 not found in dts");
	ut_assert_console_end();

	ut_assert(run_command("pr end", 0));
	ut_assert_nextline("alias freeze_br0 not found in dts");
	ut_assert_console_end();

	/* Explicit region number */
	ut_assert(run_command("pr start 1", 0));
	ut_assert_nextline("alias freeze_br1 not found in dts");
	ut_assert_console_end();

	ut_assert(run_command("pr end 2", 0));
	ut_assert_nextline("alias freeze_br2 not found in dts");
	ut_assert_console_end();

	return 0;
}

CMD_TEST(cmd_test_pr_no_freeze_br, UTF_CONSOLE);
