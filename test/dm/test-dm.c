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

struct unit_test_state global_dm_test_state;

int dm_test_run(const char *test_name)
{
	struct unit_test *tests = ll_entry_start(struct unit_test, dm_test);
	const int n_ents = ll_entry_count(struct unit_test, dm_test);
	struct unit_test_state uts_s = { .fail_count = 0 }, *uts = &uts_s;
	struct device_node *of_root;

	uts->fail_count = 0;

	if (!CONFIG_IS_ENABLED(OF_PLATDATA)) {
		/*
		 * If we have no device tree, or it only has a root node, then
		 * these * tests clearly aren't going to work...
		 */
		if (!gd->fdt_blob || fdt_next_node(gd->fdt_blob, 0, NULL) < 0) {
			puts("Please run with test device tree:\n"
			     "    ./u-boot -d arch/sandbox/dts/test.dtb\n");
			ut_assert(gd->fdt_blob);
		}
	}

	of_root = gd_of_root();
	ut_run_list("driver model", "dm_test_", tests, n_ents, test_name);

	/* Put everything back to normal so that sandbox works as expected */
	gd_set_of_root(of_root);
	gd->dm_root = NULL;
	ut_assertok(dm_init(CONFIG_IS_ENABLED(OF_LIVE)));
	dm_scan_plat(false);
	if (!CONFIG_IS_ENABLED(OF_PLATDATA))
		dm_scan_fdt(false);

	return uts->fail_count ? CMD_RET_FAILURE : 0;
}

int do_ut_dm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const char *test_name = NULL;

	if (argc > 1)
		test_name = argv[1];

	return dm_test_run(test_name);
}
