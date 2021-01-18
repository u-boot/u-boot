// SPDX-License-Identifier: GPL-2.0+
/*
 * Executes tests for memory-related commands
 *
 * Copyright 2020 Google LLC
 */

#include <common.h>
#include <command.h>
#include <test/suites.h>
#include <test/test.h>

int do_ut_mem(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test, mem_test);
	const int n_ents = ll_entry_count(struct unit_test, mem_test);

	return cmd_ut_category("cmd_mem", "mem_test_", tests, n_ents, argc,
			       argv);
}
