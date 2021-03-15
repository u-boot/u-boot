// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <asm/state.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* This is valid when a test is running, NULL otherwise */
static struct unit_test_state *cur_test_state;

struct unit_test_state *test_get_state(void)
{
	return cur_test_state;
}

void test_set_state(struct unit_test_state *uts)
{
	cur_test_state = uts;
}

/**
 * dm_test_pre_run() - Get ready to run a driver model test
 *
 * This clears out the driver model data structures. For sandbox it resets the
 * state structure
 *
 * @uts: Test state
 */
static int dm_test_pre_run(struct unit_test_state *uts)
{
	bool of_live = uts->of_live;

	uts->root = NULL;
	uts->testdev = NULL;
	uts->force_fail_alloc = false;
	uts->skip_post_probe = false;
	gd->dm_root = NULL;
	if (!CONFIG_IS_ENABLED(OF_PLATDATA))
		memset(dm_testdrv_op_count, '\0', sizeof(dm_testdrv_op_count));
	state_reset_for_test(state_get_current());

	/* Determine whether to make the live tree available */
	gd_set_of_root(of_live ? uts->of_root : NULL);
	ut_assertok(dm_init(of_live));
	uts->root = dm_root();

	return 0;
}

static int dm_test_post_run(struct unit_test_state *uts)
{
	int id;

	/*
	 * With of-platdata-inst the uclasses are created at build time. If we
	 * destroy them we cannot get them back since uclass_add() is not
	 * supported. So skip this.
	 */
	if (!CONFIG_IS_ENABLED(OF_PLATDATA_INST)) {
		for (id = 0; id < UCLASS_COUNT; id++) {
			struct uclass *uc;

			/*
			 * If the uclass doesn't exist we don't want to create
			 * it. So check that here before we call
			 * uclass_find_device().
			 */
			uc = uclass_find(id);
			if (!uc)
				continue;
			ut_assertok(uclass_destroy(uc));
		}
	}

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

/*
 * ut_test_run_on_flattree() - Check if we should run a test with flat DT
 *
 * This skips long/slow tests where there is not much value in running a flat
 * DT test in addition to a live DT test.
 *
 * @return true to run the given test on the flat device tree
 */
static bool ut_test_run_on_flattree(struct unit_test *test)
{
	const char *fname = strrchr(test->file, '/') + 1;

	if (!(test->flags & UT_TESTF_DM))
		return false;

	return !strstr(fname, "video") || strstr(test->name, "video_base");
}

/**
 * test_matches() - Check if a test should be run
 *
 * This checks if the a test should be run. In the normal case of running all
 * tests, @select_name is NULL.
 *
 * @prefix: String prefix for the tests. Any tests that have this prefix will be
 *	printed without the prefix, so that it is easier to see the unique part
 *	of the test name. If NULL, any suite name (xxx_test) is considered to be
 *	a prefix.
 * @test_name: Name of current test
 * @select_name: Name of test to run (or NULL for all)
 * @return true to run this test, false to skip it
 */
static bool test_matches(const char *prefix, const char *test_name,
			 const char *select_name)
{
	if (!select_name)
		return true;

	if (!strcmp(test_name, select_name))
		return true;

	if (!prefix) {
		const char *p = strstr(test_name, "_test_");

		/* convert xxx_test_yyy to yyy, i.e. remove the suite name */
		if (p)
			test_name = p + 6;
	} else {
		/* All tests have this prefix */
		if (!strncmp(test_name, prefix, strlen(prefix)))
			test_name += strlen(prefix);
	}

	if (!strcmp(test_name, select_name))
		return true;

	return false;
}

/**
 * ut_list_has_dm_tests() - Check if a list of tests has driver model ones
 *
 * @tests: List of tests to run
 * @count: Number of tests to ru
 * @return true if any of the tests have the UT_TESTF_DM flag
 */
static bool ut_list_has_dm_tests(struct unit_test *tests, int count)
{
	struct unit_test *test;

	for (test = tests; test < tests + count; test++) {
		if (test->flags & UT_TESTF_DM)
			return true;
	}

	return false;
}

/**
 * dm_test_restore() Put things back to normal so sandbox works as expected
 *
 * @of_root: Value to set for of_root
 * @return 0 if OK, -ve on error
 */
static int dm_test_restore(struct device_node *of_root)
{
	int ret;

	gd_set_of_root(of_root);
	gd->dm_root = NULL;
	ret = dm_init(CONFIG_IS_ENABLED(OF_LIVE));
	if (ret)
		return ret;
	dm_scan_plat(false);
	if (!CONFIG_IS_ENABLED(OF_PLATDATA))
		dm_scan_fdt(false);

	return 0;
}

/**
 * test_pre_run() - Handle any preparation needed to run a test
 *
 * @uts: Test state
 * @test: Test to prepare for
 * @return 0 if OK, -EAGAIN to skip this test since some required feature is not
 *	available, other -ve on error (meaning that testing cannot likely
 *	continue)
 */
static int test_pre_run(struct unit_test_state *uts, struct unit_test *test)
{
	if (test->flags & UT_TESTF_DM)
		ut_assertok(dm_test_pre_run(uts));

	ut_set_skip_delays(uts, false);

	uts->start = mallinfo();

	if (test->flags & UT_TESTF_SCAN_PDATA)
		ut_assertok(dm_scan_plat(false));

	if (test->flags & UT_TESTF_PROBE_TEST)
		ut_assertok(do_autoprobe(uts));

	if (!CONFIG_IS_ENABLED(OF_PLATDATA) &&
	    (test->flags & UT_TESTF_SCAN_FDT))
		ut_assertok(dm_extended_scan(false));

	if (test->flags & UT_TESTF_CONSOLE_REC) {
		int ret = console_record_reset_enable();

		if (ret) {
			printf("Skipping: Console recording disabled\n");
			return -EAGAIN;
		}
	}
	ut_silence_console(uts);

	return 0;
}

/**
 * test_post_run() - Handle cleaning up after a test
 *
 * @uts: Test state
 * @test: Test to clean up after
 * @return 0 if OK, -ve on error (meaning that testing cannot likely continue)
 */
static int test_post_run(struct unit_test_state *uts, struct unit_test *test)
{
	ut_unsilence_console(uts);
	if (test->flags & UT_TESTF_DM)
		ut_assertok(dm_test_post_run(uts));

	return 0;
}

/**
 * ut_run_test() - Run a single test
 *
 * This runs the test, handling any preparation and clean-up needed. It prints
 * the name of each test before running it.
 *
 * @uts: Test state to update. The caller should ensure that this is zeroed for
 *	the first call to this function. On exit, @uts->fail_count is
 *	incremented by the number of failures (0, one hopes)
 * @test_name: Test to run
 * @name: Name of test, possibly skipping a prefix that should not be displayed
 * @return 0 if all tests passed, -EAGAIN if the test should be skipped, -1 if
 *	any failed
 */
static int ut_run_test(struct unit_test_state *uts, struct unit_test *test,
		       const char *test_name)
{
	const char *fname = strrchr(test->file, '/') + 1;
	const char *note = "";
	int ret;

	if ((test->flags & UT_TESTF_DM) && !uts->of_live)
		note = " (flat tree)";
	printf("Test: %s: %s%s\n", test_name, fname, note);

	/* Allow access to test state from drivers */
	test_set_state(uts);

	ret = test_pre_run(uts, test);
	if (ret == -EAGAIN)
		return -EAGAIN;
	if (ret)
		return ret;

	test->func(uts);

	ret = test_post_run(uts, test);
	if (ret)
		return ret;

	test_set_state( NULL);

	return 0;
}

/**
 * ut_run_test_live_flat() - Run a test with both live and flat tree
 *
 * This calls ut_run_test() with livetree enabled, which is the standard setup
 * for runnig tests. Then, for driver model test, it calls it again with
 * livetree disabled. This allows checking of flattree being used when OF_LIVE
 * is enabled, as is the case in U-Boot proper before relocation, as well as in
 * SPL.
 *
 * @uts: Test state to update. The caller should ensure that this is zeroed for
 *	the first call to this function. On exit, @uts->fail_count is
 *	incremented by the number of failures (0, one hopes)
 * @test: Test to run
 * @name: Name of test, possibly skipping a prefix that should not be displayed
 * @return 0 if all tests passed, -EAGAIN if the test should be skipped, -1 if
 *	any failed
 */
static int ut_run_test_live_flat(struct unit_test_state *uts,
				 struct unit_test *test, const char *name)
{
	int runs;

	/* Run with the live tree if possible */
	runs = 0;
	if (CONFIG_IS_ENABLED(OF_LIVE)) {
		if (!(test->flags & UT_TESTF_FLAT_TREE)) {
			uts->of_live = true;
			ut_assertok(ut_run_test(uts, test, test->name));
			runs++;
		}
	}

	/*
	 * Run with the flat tree if we couldn't run it with live tree,
	 * or it is a core test.
	 */
	if (!(test->flags & UT_TESTF_LIVE_TREE) &&
	    (!runs || ut_test_run_on_flattree(test))) {
		uts->of_live = false;
		ut_assertok(ut_run_test(uts, test, test->name));
		runs++;
	}

	return 0;
}

/**
 * ut_run_tests() - Run a set of tests
 *
 * This runs the tests, handling any preparation and clean-up needed. It prints
 * the name of each test before running it.
 *
 * @uts: Test state to update. The caller should ensure that this is zeroed for
 *	the first call to this function. On exit, @uts->fail_count is
 *	incremented by the number of failures (0, one hopes)
 * @prefix: String prefix for the tests. Any tests that have this prefix will be
 *	printed without the prefix, so that it is easier to see the unique part
 *	of the test name. If NULL, no prefix processing is done
 * @tests: List of tests to run
 * @count: Number of tests to run
 * @select_name: Name of a single test to run (from the list provided). If NULL
 *	then all tests are run
 * @return 0 if all tests passed, -ENOENT if test @select_name was not found,
 *	-EBADF if any failed
 */
static int ut_run_tests(struct unit_test_state *uts, const char *prefix,
			struct unit_test *tests, int count,
			const char *select_name)
{
	struct unit_test *test;
	int found = 0;

	for (test = tests; test < tests + count; test++) {
		const char *test_name = test->name;
		int ret;

		if (!test_matches(prefix, test_name, select_name))
			continue;
		ret = ut_run_test_live_flat(uts, test, select_name);
		found++;
		if (ret == -EAGAIN)
			continue;
		if (ret)
			return ret;
	}
	if (select_name && !found)
		return -ENOENT;

	return uts->fail_count ? -EBADF : 0;
}

int ut_run_list(const char *category, const char *prefix,
		struct unit_test *tests, int count, const char *select_name)
{
	struct unit_test_state uts = { .fail_count = 0 };
	bool has_dm_tests = false;
	int ret;

	if (!CONFIG_IS_ENABLED(OF_PLATDATA) &&
	    ut_list_has_dm_tests(tests, count)) {
		has_dm_tests = true;
		/*
		 * If we have no device tree, or it only has a root node, then
		 * these * tests clearly aren't going to work...
		 */
		if (!gd->fdt_blob || fdt_next_node(gd->fdt_blob, 0, NULL) < 0) {
			puts("Please run with test device tree:\n"
			     "    ./u-boot -d arch/sandbox/dts/test.dtb\n");
			return CMD_RET_FAILURE;
		}
	}

	if (!select_name)
		printf("Running %d %s tests\n", count, category);

	uts.of_root = gd_of_root();
	ret = ut_run_tests(&uts, prefix, tests, count, select_name);

	if (ret == -ENOENT)
		printf("Test '%s' not found\n", select_name);
	else
		printf("Failures: %d\n", uts.fail_count);

	/* Best efforts only...ignore errors */
	if (has_dm_tests)
		dm_test_restore(uts.of_root);

	return ret;
}
