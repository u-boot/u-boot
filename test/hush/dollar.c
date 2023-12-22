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

static int hush_test_simple_dollar(struct unit_test_state *uts)
{
	console_record_reset_enable();
	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline_empty();
	ut_assert_console_end();

	ut_assertok(run_command("echo ${dollar_foo}", 0));
	ut_assert_nextline_empty();
	ut_assert_console_end();

	ut_assertok(run_command("dollar_foo=bar", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("echo ${dollar_foo}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_foo=\\$bar", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("$bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_foo='$bar'", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("$bar");
	ut_assert_console_end();

	ut_asserteq(1, run_command("dollar_foo=bar quux", 0));
	/* Next line contains error message */
	ut_assert_skipline();
	ut_assert_console_end();

	ut_asserteq(1, run_command("dollar_foo='bar quux", 0));
	/* Next line contains error message */
	ut_assert_skipline();
	ut_assert_console_end();

	ut_asserteq(1, run_command("dollar_foo=bar quux\"", 0));
	/* Two next lines contain error message */
	ut_assert_skipline();
	ut_assert_skipline();
	ut_assert_console_end();

	ut_assertok(run_command("dollar_foo='bar \"quux'", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	/*
	 * This one is buggy.
	 * ut_assert_nextline("bar \"quux");
	 * ut_assert_console_end();
	 *
	 * So, let's reset output:
	 */
	console_record_reset_enable();

	ut_asserteq(1, run_command("dollar_foo=\"bar 'quux\"", 0));
	/* Next line contains error message */
	ut_assert_skipline();
	ut_assert_console_end();

	ut_assertok(run_command("dollar_foo='bar quux'", 0));
	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("bar quux");
	ut_assert_console_end();

	puts("Beware: this test set local variable dollar_foo and it cannot be unset!");

	return 0;
}
HUSH_TEST(hush_test_simple_dollar, 0);

static int hush_test_env_dollar(struct unit_test_state *uts)
{
	env_set("env_foo", "bar");
	console_record_reset_enable();

	ut_assertok(run_command("echo $env_foo", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("echo ${env_foo}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	/* Environment variables have priority over local variable */
	ut_assertok(run_command("env_foo=quux", 0));
	ut_assertok(run_command("echo ${env_foo}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	/* Clean up setting the variable */
	env_set("env_foo", NULL);

	puts("Beware: this test set local variable env_foo and it cannot be unset!");

	return 0;
}
HUSH_TEST(hush_test_env_dollar, 0);

static int hush_test_command_dollar(struct unit_test_state *uts)
{
	console_record_reset_enable();

	ut_assertok(run_command("dollar_bar=\"echo bar\"", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("${dollar_bar}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_bar=\"echo\nbar\"", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_bar='echo bar\n'", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_bar='echo bar\\n'", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("barn");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_bar='echo $bar'", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("$bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_quux=quux", 0));
	ut_assertok(run_command("dollar_bar=\"echo $dollar_quux\"", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("quux");
	ut_assert_console_end();

	puts("Beware: this test sets local variable dollar_bar and dollar_quux and they cannot be unset!");

	return 0;
}
HUSH_TEST(hush_test_command_dollar, 0);
