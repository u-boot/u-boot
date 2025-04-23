// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Tests for spawn and wait commands
 *
 * Copyright 2025, Linaro Ltd.
 */

#include <command.h>
#include <test/cmd.h>
#include <test/test.h>
#include <test/ut.h>

static int test_cmd_spawn(struct unit_test_state *uts)
{
	ut_assertok(run_command("wait; spawn sleep 2; setenv j ${job_id}; "
				"spawn setenv spawned true; "
				"setenv jj ${job_id}; wait; "
				"echo ${j} ${jj} ${spawned}", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_ptr(uts->actual_str,
			strstr(uts->actual_str, "1 2 true"));

	ut_assertok(run_command("spawn true; wait; setenv t $?; spawn false; "
				"wait; setenv f $?; wait; echo $t $f $?", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_ptr(uts->actual_str,
			strstr(uts->actual_str, "0 1 0"));
	ut_assert_console_end();

	return 0;
}
CMD_TEST(test_cmd_spawn, UTF_CONSOLE);
