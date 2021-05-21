// SPDX-License-Identifier: GPL-2.0+
/*
 * Executes tests for pinmux command
 *
 * Copyright (C) 2021, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_cmd_pinmux_status_pinname(struct unit_test_state *uts)
{
	/* Test that 'pinmux status <pinname>' displays the selected pin. */
	console_record_reset();
	run_command("pinmux status a5", 0);
	ut_assert_nextline("a5        : gpio input .                            ");
	ut_assert_console_end();

	console_record_reset();
	run_command("pinmux status P7", 0);
	ut_assert_nextline("P7        : GPIO2 bias-pull-down input-enable.      ");
	ut_assert_console_end();

	console_record_reset();
	run_command("pinmux status P9", 0);
	ut_assert_nextline("single-pinctrl pinctrl-single-no-width: missing register width");
	ut_assert_nextline("P9 not found");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_cmd_pinmux_status_pinname, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
