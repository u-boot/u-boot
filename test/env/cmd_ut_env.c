/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <command.h>
#include <test/env.h>
#include <test/suites.h>
#include <test/ut.h>

int do_ut_env(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test, env_test);
	const int n_ents = ll_entry_count(struct unit_test, env_test);
	struct unit_test_state uts = { .fail_count = 0 };
	struct unit_test *test;

	if (argc == 1)
		printf("Running %d environment tests\n", n_ents);

	for (test = tests; test < tests + n_ents; test++) {
		if (argc > 1 && strcmp(argv[1], test->name))
			continue;
		printf("Test: %s\n", test->name);

		uts.start = mallinfo();

		test->func(&uts);
	}

	printf("Failures: %d\n", uts.fail_count);

	return uts.fail_count ? CMD_RET_FAILURE : 0;
}
