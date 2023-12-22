// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021
 * Francis Laniel, Amarula Solutions, francis.laniel@amarulasolutions.com
 */

#include <common.h>
#include <command.h>
#include <env_attr.h>
#include <test/hush.h>
#include <test/ut.h>

static int hush_test_semicolon(struct unit_test_state *uts)
{
	/* A; B = B truth table. */
	ut_asserteq(1, run_command("false; false", 0));
	ut_assertok(run_command("false; true", 0));
	ut_assertok(run_command("true; true", 0));
	ut_asserteq(1, run_command("true; false", 0));

	return 0;
}
HUSH_TEST(hush_test_semicolon, 0);

static int hush_test_and(struct unit_test_state *uts)
{
	/* A && B truth table. */
	ut_asserteq(1, run_command("false && false", 0));
	ut_asserteq(1, run_command("false && true", 0));
	ut_assertok(run_command("true && true", 0));
	ut_asserteq(1, run_command("true && false", 0));

	return 0;
}
HUSH_TEST(hush_test_and, 0);

static int hush_test_or(struct unit_test_state *uts)
{
	/* A || B truth table. */
	ut_asserteq(1, run_command("false || false", 0));
	ut_assertok(run_command("false || true", 0));
	ut_assertok(run_command("true || true", 0));
	ut_assertok(run_command("true || false", 0));

	return 0;
}
HUSH_TEST(hush_test_or, 0);

static int hush_test_and_or(struct unit_test_state *uts)
{
	/* A && B || C truth table. */
	ut_asserteq(1, run_command("false && false || false", 0));
	ut_asserteq(1, run_command("false && false || true", 0));
	ut_asserteq(1, run_command("false && true || true", 0));
	ut_asserteq(1, run_command("false && true || false", 0));
	ut_assertok(run_command("true && true || false", 0));
	ut_asserteq(1, run_command("true && false || false", 0));
	ut_assertok(run_command("true && false || true", 0));
	ut_assertok(run_command("true && true || true", 0));

	return 0;
}
HUSH_TEST(hush_test_and_or, 0);

static int hush_test_or_and(struct unit_test_state *uts)
{
	/* A || B && C truth table. */
	ut_asserteq(1, run_command("false || false && false", 0));
	ut_asserteq(1, run_command("false || false && true", 0));
	ut_assertok(run_command("false || true && true", 0));
	ut_asserteq(1, run_command("false || true && false", 0));
	ut_assertok(run_command("true || true && false", 0));
	ut_assertok(run_command("true || false && false", 0));
	ut_assertok(run_command("true || false && true", 0));
	ut_assertok(run_command("true || true && true", 0));

	return 0;
}
HUSH_TEST(hush_test_or_and, 0);
