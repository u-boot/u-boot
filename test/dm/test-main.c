/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <dm/test.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

struct unit_test_state global_dm_test_state;
static struct dm_test_state _global_priv_dm_test_state;

/* Get ready for testing */
static int dm_test_init(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;

	memset(dms, '\0', sizeof(*dms));
	gd->dm_root = NULL;
	memset(dm_testdrv_op_count, '\0', sizeof(dm_testdrv_op_count));

	ut_assertok(dm_init());
	dms->root = dm_root();

	return 0;
}

/* Ensure all the test devices are probed */
static int do_autoprobe(struct unit_test_state *uts)
{
	struct udevice *dev;
	int ret;

	/* Scanning the uclass is enough to probe all the devices */
	for (ret = uclass_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_next_device(&dev))
		;

	return ret;
}

static int dm_test_destroy(struct unit_test_state *uts)
{
	int id;

	for (id = 0; id < UCLASS_COUNT; id++) {
		struct uclass *uc;

		/*
		 * If the uclass doesn't exist we don't want to create it. So
		 * check that here before we call uclass_find_device()/
		 */
		uc = uclass_find(id);
		if (!uc)
			continue;
		ut_assertok(uclass_destroy(uc));
	}

	return 0;
}

static int dm_test_main(const char *test_name)
{
	struct unit_test *tests = ll_entry_start(struct unit_test, dm_test);
	const int n_ents = ll_entry_count(struct unit_test, dm_test);
	struct unit_test_state *uts = &global_dm_test_state;
	uts->priv = &_global_priv_dm_test_state;
	struct unit_test *test;

	/*
	 * If we have no device tree, or it only has a root node, then these
	 * tests clearly aren't going to work...
	 */
	if (!gd->fdt_blob || fdt_next_node(gd->fdt_blob, 0, NULL) < 0) {
		puts("Please run with test device tree:\n"
		     "    ./u-boot -d arch/sandbox/dts/test.dtb\n");
		ut_assert(gd->fdt_blob);
	}

	if (!test_name)
		printf("Running %d driver model tests\n", n_ents);

	for (test = tests; test < tests + n_ents; test++) {
		if (test_name && strcmp(test_name, test->name))
			continue;
		printf("Test: %s\n", test->name);
		ut_assertok(dm_test_init(uts));

		uts->start = mallinfo();
		if (test->flags & DM_TESTF_SCAN_PDATA)
			ut_assertok(dm_scan_platdata(false));
		if (test->flags & DM_TESTF_PROBE_TEST)
			ut_assertok(do_autoprobe(uts));
		if (test->flags & DM_TESTF_SCAN_FDT)
			ut_assertok(dm_scan_fdt(gd->fdt_blob, false));

		test->func(uts);

		ut_assertok(dm_test_destroy(uts));
	}

	printf("Failures: %d\n", uts->fail_count);

	gd->dm_root = NULL;
	ut_assertok(dm_init());
	dm_scan_platdata(false);
	dm_scan_fdt(gd->fdt_blob, false);

	return uts->fail_count ? CMD_RET_FAILURE : 0;
}

int do_ut_dm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *test_name = NULL;

	if (argc > 1)
		test_name = argv[1];

	return dm_test_main(test_name);
}
