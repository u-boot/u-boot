// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for 'meminfo' command
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <dm/test.h>
#include <test/cmd.h>
#include <test/ut.h>

/* Test 'meminfo' command */
static int cmd_test_meminfo(struct unit_test_state *uts)
{
	ut_assertok(run_command("meminfo", 0));
	ut_assert_nextline("DRAM:  256 MiB");
	ut_assert_nextline_empty();

	ut_assert_nextline("Region           Base     Size      End      Gap");
	ut_assert_nextlinen("-");

	/* For now we don't worry about checking the values */
	ut_assert_nextlinen("video");
	ut_assert_nextlinen("code");
	ut_assert_nextlinen("malloc");
	ut_assert_nextlinen("board_info");
	ut_assert_nextlinen("global_data");
	ut_assert_nextlinen("devicetree");
	ut_assert_nextlinen("bootstage");
	ut_assert_nextlinen("bloblist");
	ut_assert_nextlinen("stack");

	/* we expect at least one lmb line, but don't know how many */
	ut_assert_nextlinen("lmb");
	ut_assert_skip_to_linen("free");

	ut_assert_console_end();

	return 0;
}
CMD_TEST(cmd_test_meminfo, UTF_CONSOLE);
