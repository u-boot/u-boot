// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for addrmap command
 *
 * Copyright (C) 2021, Bin Meng <bmeng.cn@gmail.com>
 */

#include <console.h>
#include <test/ut.h>

/* Declare a new addrmap test */
#define ADDRMAP_TEST(_name, _flags)	UNIT_TEST(_name, _flags, addrmap)

/* Test 'addrmap' command output */
static int addrmap_test_basic(struct unit_test_state *uts)
{
	ut_assertok(run_command("addrmap", 0));
	ut_assert_nextline("           vaddr            paddr             size");
	ut_assert_nextline("================ ================ ================");
	/* There should be at least one entry */
	ut_assertok(!ut_check_console_end(uts));

	return 0;
}
ADDRMAP_TEST(addrmap_test_basic, UTF_CONSOLE);
