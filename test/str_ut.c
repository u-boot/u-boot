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
static const char str4[] = "1234567890123 I lost closer friends";
static const char str5[] = "0x9876543210the last time I was deloused";
static const char str6[] = "0778octal is seldom used";
static const char str7[] = "707it is a piece of computing history";

/* Declare a new str test */
#define STR_TEST(_name, _flags)		UNIT_TEST(_name, _flags, str_test)

static int str_upper(struct unit_test_state *uts)
{
	char out[TEST_STR_SIZE];

	/* Make sure it adds a terminator */
	out[strlen(str1)] = 'a';
	str_to_upper(str1, out, SIZE_MAX);
	ut_asserteq_str("I'M SORRY I'M LATE.", out);

	/* In-place operation */
	strcpy(out, str2);
	str_to_upper(out, out, SIZE_MAX);
	ut_asserteq_str("1099ABNO, DON'T BOTHER APOLOGISING.", out);

	/* Limited length */
	str_to_upper(str1, out, 7);
	ut_asserteq_str("I'M SORO, DON'T BOTHER APOLOGISING.", out);

	/* In-place with limited length */
	strcpy(out, str2);
	str_to_upper(out, out, 7);
	ut_asserteq_str("1099ABNo, don't bother apologising.", out);

	/* Copy an empty string to a buffer with space*/
	out[1] = 0x7f;
	str_to_upper("", out, SIZE_MAX);
	ut_asserteq('\0', *out);
	ut_asserteq(0x7f, out[1]);

	/* Copy an empty string to a buffer with no space*/
	out[0] = 0x7f;
	str_to_upper("", out, 0);
	ut_asserteq(0x7f, out[0]);

	return 0;
}
STR_TEST(str_upper, 0);

static int run_strtoul(struct unit_test_state *uts, const char *str, int base,
		       ulong expect_val, int expect_endp_offset, bool upper)
{
	char out[TEST_STR_SIZE];
	char *endp;
	ulong val;

	strcpy(out, str);
	if (upper)
		str_to_upper(out, out, -1);

	val = simple_strtoul(out, &endp, base);
	ut_asserteq(expect_val, val);
	ut_asserteq(expect_endp_offset, endp - out);

	return 0;
}

static int str_simple_strtoul(struct unit_test_state *uts)
{
	int upper;

	/* Check that it is case-insentive */
	for (upper = 0; upper < 2; upper++) {
		/* Base 10 and base 16 */
		ut_assertok(run_strtoul(uts, str2, 10, 1099, 4, upper));
		ut_assertok(run_strtoul(uts, str2, 16, 0x1099ab, 6, upper));
		ut_assertok(run_strtoul(uts, str3, 16, 0xb, 3, upper));
		ut_assertok(run_strtoul(uts, str3, 10, 0xb, 3, upper));

		/* Octal */
		ut_assertok(run_strtoul(uts, str6, 0, 63, 3, upper));
		ut_assertok(run_strtoul(uts, str7, 8, 0x1c7, 3, upper));

		/* Invalid string */
		ut_assertok(run_strtoul(uts, str1, 10, 0, 0, upper));

		/* Base 0 */
		ut_assertok(run_strtoul(uts, str1, 0, 0, 0, upper));
		ut_assertok(run_strtoul(uts, str2, 0, 1099, 4, upper));
		ut_assertok(run_strtoul(uts, str3, 0, 0xb, 3, upper));

		/* Base 2 */
		ut_assertok(run_strtoul(uts, str1, 2, 0, 0, upper));
		ut_assertok(run_strtoul(uts, str2, 2, 2, 2, upper));
	}

	/* Check endp being NULL */
	ut_asserteq(1099, simple_strtoul(str2, NULL, 0));

	return 0;
}
STR_TEST(str_simple_strtoul, 0);

static int run_strtoull(struct unit_test_state *uts, const char *str, int base,
			unsigned long long expect_val, int expect_endp_offset,
			bool upper)
{
	char out[TEST_STR_SIZE];
	char *endp;
	unsigned long long val;

	strcpy(out, str);
	if (upper)
		str_to_upper(out, out, -1);

	val = simple_strtoull(out, &endp, base);
	ut_asserteq(expect_val, val);
	ut_asserteq(expect_endp_offset, endp - out);

	return 0;
}

static int str_simple_strtoull(struct unit_test_state *uts)
{
	int upper;

	/* Check that it is case-insentive */
	for (upper = 0; upper < 2; upper++) {
		/* Base 10 and base 16 */
		ut_assertok(run_strtoull(uts, str2, 10, 1099, 4, upper));
		ut_assertok(run_strtoull(uts, str2, 16, 0x1099ab, 6, upper));
		ut_assertok(run_strtoull(uts, str3, 16, 0xb, 3, upper));
		ut_assertok(run_strtoull(uts, str3, 10, 0xb, 3, upper));

		/* Octal */
		ut_assertok(run_strtoull(uts, str6, 0, 63, 3, upper));
		ut_assertok(run_strtoull(uts, str7, 8, 0x1c7, 3, upper));

		/* Large values */
		ut_assertok(run_strtoull(uts, str4, 10, 1234567890123, 13,
					 upper));
		ut_assertok(run_strtoull(uts, str4, 16, 0x1234567890123, 13,
					 upper));
		ut_assertok(run_strtoull(uts, str5, 0, 0x9876543210, 12,
					 upper));

		/* Invalid string */
		ut_assertok(run_strtoull(uts, str1, 10, 0, 0, upper));

		/* Base 0 */
		ut_assertok(run_strtoull(uts, str1, 0, 0, 0, upper));
		ut_assertok(run_strtoull(uts, str2, 0, 1099, 4, upper));
		ut_assertok(run_strtoull(uts, str3, 0, 0xb, 3, upper));

		/* Base 2 */
		ut_assertok(run_strtoull(uts, str1, 2, 0, 0, upper));
		ut_assertok(run_strtoull(uts, str2, 2, 2, 2, upper));
	}

	/* Check endp being NULL */
	ut_asserteq(1099, simple_strtoull(str2, NULL, 0));

	return 0;
}
STR_TEST(str_simple_strtoull, 0);

static int str_hextoul(struct unit_test_state *uts)
{
	char *endp;

	/* Just a simple test, since we know this uses simple_strtoul() */
	ut_asserteq(0x1099ab, hextoul(str2, &endp));
	ut_asserteq(6, endp - str2);

	return 0;
}
STR_TEST(str_hextoul, 0);

static int str_dectoul(struct unit_test_state *uts)
{
	char *endp;

	/* Just a simple test, since we know this uses simple_strtoul() */
	ut_asserteq(1099, dectoul(str2, &endp));
	ut_asserteq(4, endp - str2);

	return 0;
}
STR_TEST(str_dectoul, 0);

int do_ut_str(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(str_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(str_test);

	return cmd_ut_category("str", "str_", tests, n_ents, argc, argv);
}
