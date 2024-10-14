// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for coreboot commands
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <command.h>
#include <test/cmd.h>
#include <test/test.h>
#include <test/ut.h>

/**
 * test_cmd_cbsysinfo() - test the cbsysinfo command produces expected output
 *
 * This includes ensuring that the coreboot build has the expected options
 * enabled
 */
static int test_cmd_cbsysinfo(struct unit_test_state *uts)
{
	ut_assertok(run_command("cbsysinfo", 0));
	ut_assert_nextlinen("Coreboot table at");

	/* Make sure CMOS options are enabled */
	ut_assert_skip_to_line(
		" 1c0    1    e   1  power_on_after_fail    0:Disable 1:Enable");
	ut_assert_skip_to_line("CMOS start  : 1c0");
	ut_assert_nextline("   CMOS end    : 1cf");
	ut_assert_nextline("   CMOS csum loc: 3f0");

	/* Make sure the linear frame buffer is enabled */
	ut_assert_skip_to_linen("Framebuffer");
	ut_assert_nextlinen("   Phys addr");

	ut_assert_skip_to_line("Chrome OS VPD: 00000000");
	ut_assert_nextlinen("RSDP");
	ut_assert_nextlinen("Unimpl.");
	ut_assert_console_end();

	return 0;
}
CMD_TEST(test_cmd_cbsysinfo, UTF_CONSOLE);
