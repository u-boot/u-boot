// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for cpuid command
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <test/cmd.h>
#include <test/ut.h>

static int cmd_test_cpuid(struct unit_test_state *uts)
{
	ut_assertok(run_commandf("cpuid 1"));
	ut_assert_nextline("eax 00060fb1");
	ut_assert_nextline("ebx 00000800");
	ut_assert_nextline("ecx 80002001");
	ut_assert_nextline("edx 078bfbfd");

	return 0;
}
CMD_TEST(cmd_test_cpuid, UTF_CONSOLE);
