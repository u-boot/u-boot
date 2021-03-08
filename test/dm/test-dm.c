// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/state.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

int dm_test_run(const char *test_name)
{
	struct unit_test *tests = ll_entry_start(struct unit_test, dm_test);
	const int n_ents = ll_entry_count(struct unit_test, dm_test);
	int ret;

	ret = ut_run_list("driver model", "dm_test_", tests, n_ents, test_name);

	return ret ? CMD_RET_FAILURE : 0;
}

int do_ut_dm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const char *test_name = NULL;

	if (argc > 1)
		test_name = argv[1];

	return dm_test_run(test_name);
}
