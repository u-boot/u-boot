// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test for probing i3c controller with invalid name */
static int dm_test_i3c_cmd_probe_invalid_master(struct unit_test_state *uts)
{
	ut_asserteq(1, run_command("i3c any", 0));
	ut_assert_nextline("i3c0 (i3c_sandbox)");
	ut_assert_nextline("i3c1 (i3c_sandbox)");
	ut_assert_nextline("i3c: Host controller not initialized: any");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_i3c_cmd_probe_invalid_master, UTF_SCAN_PDATA | UTF_SCAN_FDT | UTF_DM);

/* Basic test of the i3c controller for valid name as per test DT */
static int dm_test_i3c_cmd_probe_valid_master(struct unit_test_state *uts)
{
	ut_asserteq(0, run_command("i3c i3c0", 0));
	ut_assert_nextline("i3c: Current controller: i3c0");
	ut_assert_console_end();

	ut_asserteq(0, run_command("i3c current", 0));
	ut_assert_nextline("i3c: Current controller: i3c0");
	ut_assert_console_end();

	ut_asserteq(0, run_command("i3c i3c1", 0));
	ut_assert_nextline("i3c: Current controller: i3c1");
	ut_assert_console_end();

	ut_asserteq(0, run_command("i3c current", 0));
	ut_assert_nextline("i3c: Current controller: i3c1");
	ut_assert_console_end();

	ut_asserteq(0, run_command("i3c list", 0));
	ut_assert_nextline("i3c0 (i3c_sandbox)");
	ut_assert_nextline("i3c1 (i3c_sandbox)");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_i3c_cmd_probe_valid_master, UTF_SCAN_PDATA | UTF_SCAN_FDT | UTF_DM);