/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <dm/test.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>
#include <dm/ut.h>

DECLARE_GLOBAL_DATA_PTR;

struct dm_test_state global_test_state;

/* Get ready for testing */
static int dm_test_init(struct dm_test_state *dms)
{
	memset(dms, '\0', sizeof(*dms));
	gd->dm_root = NULL;
	memset(dm_testdrv_op_count, '\0', sizeof(dm_testdrv_op_count));

	ut_assertok(dm_init());
	dms->root = dm_root();

	return 0;
}

/* Ensure all the test devices are probed */
static int do_autoprobe(struct dm_test_state *dms)
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

static int dm_test_destroy(struct dm_test_state *dms)
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

int dm_test_main(void)
{
	struct dm_test *tests = ll_entry_start(struct dm_test, dm_test);
	const int n_ents = ll_entry_count(struct dm_test, dm_test);
	struct dm_test_state *dms = &global_test_state;
	struct dm_test *test;

	/*
	 * If we have no device tree, or it only has a root node, then these
	 * tests clearly aren't going to work...
	 */
	if (!gd->fdt_blob || fdt_next_node(gd->fdt_blob, 0, NULL) < 0) {
		puts("Please run with test device tree:\n"
		     "     dtc -I dts -O dtb test/dm/test.dts  -o test/dm/test.dtb\n"
		     "    ./u-boot -d test/dm/test.dtb\n");
		ut_assert(gd->fdt_blob);
	}

	printf("Running %d driver model tests\n", n_ents);

	for (test = tests; test < tests + n_ents; test++) {
		printf("Test: %s\n", test->name);
		ut_assertok(dm_test_init(dms));

		if (test->flags & DM_TESTF_SCAN_PDATA)
			ut_assertok(dm_scan_platdata());
		if (test->flags & DM_TESTF_PROBE_TEST)
			ut_assertok(do_autoprobe(dms));
		if (test->flags & DM_TESTF_SCAN_FDT)
			ut_assertok(dm_scan_fdt(gd->fdt_blob));

		if (test->func(dms))
			break;

		ut_assertok(dm_test_destroy(dms));
	}

	printf("Failures: %d\n", dms->fail_count);

	return 0;
}
