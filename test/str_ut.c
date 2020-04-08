// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Google LLC
 */

#include <common.h>
#include <vsprintf.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

/* This is large enough for any of the test strings */
#define TEST_STR_SIZE	200

static const char str1[] = "I'm sorry I'm late.";
static const char str2[] = "1099abNo, don't bother apologising.";
static const char str3[] = "0xbI'm sorry you're alive.";

/* Declare a new str test */
#define STR_TEST(_name, _flags)		UNIT_TEST(_name, _flags, str_test)

static int run_strtoul(struct unit_test_state *uts, const char *str, int base,
		       ulong expect_val, int expect_endp_offset)
{
	char *endp;
	ulong val;

	val = simple_strtoul(str, &endp, base);
	ut_asserteq(expect_val, val);
	ut_asserteq(expect_endp_offset, endp - str);

	return 0;
}

static int str_simple_strtoul(struct unit_test_state *uts)
{
	/* Base 10 and base 16 */
	ut_assertok(run_strtoul(uts, str2, 10, 1099, 4));
	ut_assertok(run_strtoul(uts, str2, 16, 0x1099ab, 6));

	/* Invalid string */
	ut_assertok(run_strtoul(uts, str1, 10, 0, 0));

	/* Base 0 */
	ut_assertok(run_strtoul(uts, str1, 0, 0, 0));
	ut_assertok(run_strtoul(uts, str2, 0, 1099, 4));
	ut_assertok(run_strtoul(uts, str3, 0, 0xb, 3));

	/* Base 2 */
	ut_assertok(run_strtoul(uts, str1, 2, 0, 0));
	ut_assertok(run_strtoul(uts, str2, 2, 2, 2));

	/* Check endp being NULL */
	ut_asserteq(1099, simple_strtoul(str2, NULL, 0));

	return 0;
}
STR_TEST(str_simple_strtoul, 0);

int do_ut_str(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test,
						 str_test);
	const int n_ents = ll_entry_count(struct unit_test, str_test);

	return cmd_ut_category("str", "str_", tests, n_ents, argc, argv);
}
