// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 */

#include <common.h>
#include <cli.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>

static int cli_ch_test(struct unit_test_state *uts)
{
	struct cli_ch_state s_cch, *cch = &s_cch;

	cli_ch_init(cch);

	/* should be nothing to return at first */
	ut_asserteq(0, cli_ch_process(cch, 0));

	/* check normal entry */
	ut_asserteq('a', cli_ch_process(cch, 'a'));
	ut_asserteq('b', cli_ch_process(cch, 'b'));
	ut_asserteq('c', cli_ch_process(cch, 'c'));
	ut_asserteq(0, cli_ch_process(cch, 0));

	/* send an invalid escape sequence */
	ut_asserteq(0, cli_ch_process(cch, '\e'));
	ut_asserteq(0, cli_ch_process(cch, '['));

	/*
	 * with the next char it sees that the sequence is invalid, so starts
	 * emitting it
	 */
	ut_asserteq('\e', cli_ch_process(cch, 'X'));

	/* now we set 0 bytes to empty the buffer */
	ut_asserteq('[', cli_ch_process(cch, 0));
	ut_asserteq('X', cli_ch_process(cch, 0));
	ut_asserteq(0, cli_ch_process(cch, 0));

	/* things are normal again */
	ut_asserteq('a', cli_ch_process(cch, 'a'));
	ut_asserteq(0, cli_ch_process(cch, 0));

	return 0;
}
COMMON_TEST(cli_ch_test, 0);
