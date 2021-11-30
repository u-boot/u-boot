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

/**
 * dm_test_run() - Run driver model tests
 *
 * Run all the available driver model tests, or a selection
 *
 * @test_name: Name of single test to run (e.g. "dm_test_fdt_pre_reloc" or just
 *	"fdt_pre_reloc"), or NULL to run all
 * @return 0 if all tests passed, 1 if not
 */
static int dm_test_run(const char *test_name)
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(dm_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(dm_test);
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
