// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for msr command
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <test/cmd.h>
#include <test/ut.h>

static int cmd_test_msr(struct unit_test_state *uts)
{
	ut_assertok(run_commandf("msr read 200"));
	ut_assert_nextline("00000000 ffe00006");
	ut_assert_console_end();

	/* change the first variable msr and see it reflected in the mtrr cmd */
	ut_assertok(run_commandf("mtrr"));
	ut_assert_nextline("CPU 65537:");
	ut_assert_nextlinen("Reg");
	ut_assert_nextlinen("0   Y     Back         00000000ffe00000");
	ut_assertok(console_record_reset_enable());

	/* change the type from 6 to 5 */
	ut_assertok(run_commandf("msr write 200 0 ffe00005"));
	ut_assert_console_end();

	/* Now it shows 'Protect' */
	ut_assertok(run_commandf("mtrr"));
	ut_assert_nextline("CPU 65537:");
	ut_assert_nextlinen("Reg");
	ut_assert_nextlinen("0   Y     Protect      00000000ffe00000");
	ut_assertok(console_record_reset_enable());

	return 0;
}
CMD_TEST(cmd_test_msr, UTF_CONSOLE);
