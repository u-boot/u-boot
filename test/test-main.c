// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_TEST

#include <blk.h>
#include <console.h>
#include <cyclic.h>
#include <dm.h>
#include <event.h>
#include <net.h>
#include <of_live.h>
#include <os.h>
#include <spl.h>
#include <usb.h>
#include <dm/ofnode.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>
#include <u-boot/crc.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * enum fdtchk_t - what to do with the device tree (gd->fdt_blob)
 *
 * This affects what happens with the device tree before and after a test
 *
 * @FDTCHK_NONE: Do nothing
 * @FDTCHK_CHECKSUM: Take a checksum of the FDT before the test runs and
 *	compare it afterwards to detect any changes
 * @FDTCHK_COPY: Make a copy of the FDT and restore it afterwards
 */
enum fdtchk_t {
	FDTCHK_NONE,
	FDTCHK_CHECKSUM,
	FDTCHK_COPY,
};

/**
 * fdt_action() - get the required action for the FDT
 *
 * @return the action that should be taken for this build
 */
static enum fdtchk_t fdt_action(void)
{
	/* For sandbox SPL builds, do nothing */
	if (IS_ENABLED(CONFIG_SANDBOX) && IS_ENABLED(CONFIG_XPL_BUILD))
		return FDTCHK_NONE;

	/* Do a copy for sandbox (but only the U-Boot build, not SPL) */
	if (IS_ENABLED(CONFIG_SANDBOX))
		return FDTCHK_COPY;

	/* For all other boards, do a checksum */
	return FDTCHK_CHECKSUM;
}

/* This is valid when a test is running, NULL otherwise */
static struct unit_test_state *cur_test_state;

struct unit_test_state *ut_get_state(void)
{
	return cur_test_state;
}

void ut_set_state(struct unit_test_state *uts)
{
	cur_test_state = uts;
}

void ut_init_state(struct unit_test_state *uts)
{
	memset(uts, '\0', sizeof(*uts));
}

void ut_uninit_state(struct unit_test_state *uts)
{
	if (IS_ENABLED(CONFIG_SANDBOX)) {
		os_free(uts->fdt_copy);
		os_free(uts->other_fdt);
	}
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

	if (of_live && (gd->flags & GD_FLG_FDT_CHANGED)) {
		printf("Cannot run live tree test as device tree changed\n");
		return -EFAULT;
	}
	uts->root = NULL;
	uts->testdev = NULL;
	uts->force_fail_alloc = false;
	uts->skip_post_probe = false;
	if (fdt_action() == FDTCHK_CHECKSUM)
		uts->fdt_chksum = crc8(0, gd->fdt_blob,
				       fdt_totalsize(gd->fdt_blob));
	gd->dm_root = NULL;
	malloc_disable_testing();
	if (CONFIG_IS_ENABLED(UT_DM) && !CONFIG_IS_ENABLED(OF_PLATDATA))
		memset(dm_testdrv_op_count, '\0', sizeof(dm_testdrv_op_count));
	arch_reset_for_test();

	/* Determine whether to make the live tree available */
	gd_set_of_root(of_live ? uts->of_root : NULL);
	oftree_reset();
	ut_assertok(dm_init(of_live));
	uts->root = dm_root();

	return 0;
}

static int dm_test_post_run(struct unit_test_state *uts)
{
	int id;

	if (gd->fdt_blob) {
		switch (fdt_action()) {
		case FDTCHK_COPY:
			memcpy((void *)gd->fdt_blob, uts->fdt_copy, uts->fdt_size);
			break;
		case FDTCHK_CHECKSUM: {
			uint chksum;

			chksum = crc8(0, gd->fdt_blob, fdt_totalsize(gd->fdt_blob));
			if (chksum != uts->fdt_chksum) {
				/*
				 * We cannot run any more tests that need the
				 * live tree, since its strings point into the
				 * flat tree, which has changed. This likely
				 * means that at least some of the pointers from
				 * the live tree point to different things
				 */
				printf("Device tree changed: cannot run live tree tests\n");
				gd->flags |= GD_FLG_FDT_CHANGED;
			}
			break;
		}
		case FDTCHK_NONE:
			break;
		}
	}

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
	return uclass_probe_all(UCLASS_TEST);
}

/*
 * ut_test_run_on_flattree() - Check if we should run a test with flat DT
 *
 * This skips long/slow tests where there is not much value in running a flat
 * DT test in addition to a live DT test.
 *
 * Return: true to run the given test on the flat device tree
 */
static bool ut_test_run_on_flattree(struct unit_test *test)
{
	const char *fname = strrchr(test->file, '/') + 1;

	if (!(test->flags & UTF_DM))
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
 * Return: true to run this test, false to skip it
 */
static bool test_matches(const char *prefix, const char *test_name,
			 const char *select_name)
{
	size_t len;

	if (!select_name)
		return true;

	/* Allow glob expansion in the test name */
	len = select_name[strlen(select_name) - 1] == '*' ? strlen(select_name) : 0;
	if (len-- == 1)
		return true;

	if (!strncmp(test_name, select_name, len))
		return true;

	if (prefix) {
		/* All tests have this prefix */
		if (!strncmp(test_name, prefix, strlen(prefix)))
			test_name += strlen(prefix);
	} else {
		const char *p = strstr(test_name, "_test_");

		/* convert xxx_test_yyy to yyy, i.e. remove the suite name */
		if (p)
			test_name = p + strlen("_test_");
	}

	if (!strncmp(test_name, select_name, len))
		return true;

	return false;
}

/**
 * ut_list_has_dm_tests() - Check if a list of tests has driver model ones
 *
 * @tests: List of tests to run
 * @count: Number of tests to run
 * @prefix: String prefix for the tests. Any tests that have this prefix will be
 *	printed without the prefix, so that it is easier to see the unique part
 *	of the test name. If NULL, no prefix processing is done
 * @select_name: Name of a single test being run (from the list provided). If
 *	NULL all tests are being run
 * Return: true if any of the tests have the UTF_DM flag
 */
static bool ut_list_has_dm_tests(struct unit_test *tests, int count,
				 const char *prefix, const char *select_name)
{
	struct unit_test *test;

	for (test = tests; test < tests + count; test++) {
		if (test_matches(prefix, test->name, select_name) &&
		    (test->flags & UTF_DM))
			return true;
	}

	return false;
}

/**
 * dm_test_restore() Put things back to normal so sandbox works as expected
 *
 * @of_root: Value to set for of_root
 * Return: 0 if OK, -ve on error
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
		dm_extended_scan(false);

	return 0;
}

/**
 * test_pre_run() - Handle any preparation needed to run a test
 *
 * @uts: Test state
 * @test: Test to prepare for
 * Return: 0 if OK, -EAGAIN to skip this test since some required feature is not
 *	available, other -ve on error (meaning that testing cannot likely
 *	continue)
 */
static int test_pre_run(struct unit_test_state *uts, struct unit_test *test)
{
	ut_assertok(event_init());

	/*
	 * Remove any USB keyboard, so that we can add and remove USB devices
	 * in tests.
	 *
	 * For UTF_DM tests, the old driver model state is saved and
	 * restored across each test. Within in each test there is therefore a
	 * new driver model state, which means that any USB keyboard device in
	 * stdio points to the old state.
	 *
	 * This is fine in most cases. But if a non-UTF_DM test starts up
	 * USB (thus creating a stdio record pointing to the USB keyboard
	 * device) then when the test finishes, the new driver model state is
	 * freed, meaning that there is now a stale pointer in stdio.
	 *
	 * This means that any future UTF_DM test which uses stdin will
	 * cause the console system to call tstc() on the stale device pointer,
	 * causing a crash.
	 *
	 * We don't want to fix this by enabling UTF_DM for all tests as
	 * this causes other problems. For example, bootflow_efi relies on
	 * U-Boot going through a proper init - without that we don't have the
	 * TCG measurement working and get an error
	 * 'tcg2 measurement fails(0x8000000000000007)'. Once we tidy up how EFI
	 * runs tests (e.g. get rid of all the restarting of U-Boot) we could
	 * potentially make the bootstd tests set UTF_DM, but other tests
	 * might do the same thing.
	 *
	 * We could add a test flag to declare that USB is being used, but that
	 * seems unnecessary, at least for now. We could detect USB being used
	 * in a test, but there is no obvious drawback to clearing out stale
	 * pointers always.
	 *
	 * So just remove any USB keyboards from the console tables. This allows
	 * UTF_DM and non-UTF_DM tests to coexist happily.
	 */
	usb_kbd_remove_for_test();

	if (test->flags & UTF_DM)
		ut_assertok(dm_test_pre_run(uts));

	ut_set_skip_delays(uts, false);

	uts->start = mallinfo();

	if (test->flags & UTF_SCAN_PDATA)
		ut_assertok(dm_scan_plat(false));

	if (test->flags & UTF_PROBE_TEST)
		ut_assertok(do_autoprobe(uts));

	if (CONFIG_IS_ENABLED(OF_REAL) &&
	    (test->flags & UTF_SCAN_FDT)) {
		/*
		 * only set this if we know the ethernet uclass will be created
		 */
		eth_set_enable_bootdevs(test->flags & UTF_ETH_BOOTDEV);
		test_sf_set_enable_bootdevs(test->flags & UTF_SF_BOOTDEV);
		ut_assertok(dm_extended_scan(false));
	}

	/*
	 * Do this after FDT scan since dm_scan_other() in bootstd-uclass.c
	 * checks for the existence of bootstd
	 */
	if (test->flags & UTF_SCAN_PDATA)
		ut_assertok(dm_scan_other(false));

	if (IS_ENABLED(CONFIG_SANDBOX) && (test->flags & UTF_OTHER_FDT)) {
		/* make sure the other FDT is available */
		ut_assertok(test_load_other_fdt(uts));

		/*
		 * create a new live tree with it for every test, in case a
		 * test modifies the tree
		 */
		if (of_live_active()) {
			ut_assertok(unflatten_device_tree(uts->other_fdt,
							  &uts->of_other));
		}
	}

	if (test->flags & UTF_CONSOLE) {
		int ret = console_record_reset_enable();

		if (ret) {
			printf("Skipping: Console recording disabled\n");
			return -EAGAIN;
		}
	}
	if (test->flags & UFT_BLOBLIST) {
		log_debug("save bloblist %p\n", gd_bloblist());
		uts->old_bloblist = gd_bloblist();
		gd_set_bloblist(NULL);
	}

	ut_silence_console(uts);

	return 0;
}

/**
 * test_post_run() - Handle cleaning up after a test
 *
 * @uts: Test state
 * @test: Test to clean up after
 * Return: 0 if OK, -ve on error (meaning that testing cannot likely continue)
 */
static int test_post_run(struct unit_test_state *uts, struct unit_test *test)
{
	ut_unsilence_console(uts);
	if (test->flags & UTF_DM)
		ut_assertok(dm_test_post_run(uts));
	ut_assertok(cyclic_unregister_all());
	ut_assertok(event_uninit());

	free(uts->of_other);
	uts->of_other = NULL;

	if (test->flags & UFT_BLOBLIST) {
		gd_set_bloblist(uts->old_bloblist);
		log_debug("restore bloblist %p\n", gd_bloblist());
	}

	blkcache_free();

	return 0;
}

/**
 * skip_test() - Handle skipping a test
 *
 * @uts: Test state to update
 * @return -EAGAIN (always)
 */
static int skip_test(struct unit_test_state *uts)
{
	uts->cur.skip_count++;

	return -EAGAIN;
}

/**
 * ut_run_test() - Run a single test
 *
 * This runs the test, handling any preparation and clean-up needed. It prints
 * the name of each test before running it.
 *
 * @uts: Test state to update. The caller should ensure that this is zeroed for
 *	the first call to this function. On exit, @uts->cur.fail_count is
 *	incremented by the number of failures (0, one hopes)
 * @test_name: Test to run
 * @name: Name of test, possibly skipping a prefix that should not be displayed
 * Return: 0 if all tests passed, -EAGAIN if the test should be skipped, -1 if
 *	any failed
 */
static int ut_run_test(struct unit_test_state *uts, struct unit_test *test,
		       const char *test_name)
{
	const char *fname = strrchr(test->file, '/') + 1;
	const char *note = "";
	int ret;

	if ((test->flags & UTF_DM) && !uts->of_live)
		note = " (flat tree)";
	printf("Test: %s: %s%s\n", test_name, fname, note);

	/* Allow access to test state from drivers */
	ut_set_state(uts);

	ret = test_pre_run(uts, test);
	if (ret == -EAGAIN)
		return skip_test(uts);
	if (ret)
		return ret;

	ret = test->func(uts);
	if (ret == -EAGAIN)
		skip_test(uts);

	ret = test_post_run(uts, test);
	if (ret)
		return ret;

	ut_set_state(NULL);

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
 *	the first call to this function. On exit, @uts->cur.fail_count is
 *	incremented by the number of failures (0, one hopes)
 * @test: Test to run
 * @leaf: Part of the name to show, or NULL to use test->name
 * Return: 0 if all tests passed, -EAGAIN if the test should be skipped, -1 if
 *	any failed
 */
static int ut_run_test_live_flat(struct unit_test_state *uts,
				 struct unit_test *test, const char *leaf)
{
	int runs, ret;

	if ((test->flags & UTF_OTHER_FDT) && !IS_ENABLED(CONFIG_SANDBOX))
		return skip_test(uts);

	/* Run with the live tree if possible */
	runs = 0;
	if (CONFIG_IS_ENABLED(OF_LIVE)) {
		if (!(test->flags & UTF_FLAT_TREE)) {
			uts->of_live = true;
			ret = ut_run_test(uts, test, leaf ?: test->name);
			if (ret != -EAGAIN) {
				ut_assertok(ret);
				runs++;
			}
		}
	}

	/*
	 * Run with the flat tree if:
	 * - it is not marked for live tree only
	 * - it doesn't require the 'other' FDT when OFNODE_MULTI_TREE_MAX is
	 *   not enabled (since flat tree can only support a single FDT in that
	 *   case
	 * - we couldn't run it with live tree,
	 * - it is a core test (dm tests except video)
	 * - the FDT is still valid and has not been updated by an earlier test
	 *   (for sandbox we handle this by copying the tree, but not for other
	 *    boards)
	 */
	if ((!CONFIG_IS_ENABLED(OF_LIVE) ||
	     (test->flags & UTF_SCAN_FDT)) &&
	    !(test->flags & UTF_LIVE_TREE) &&
	    (CONFIG_IS_ENABLED(OFNODE_MULTI_TREE) ||
	     !(test->flags & UTF_OTHER_FDT)) &&
	    (!runs || ut_test_run_on_flattree(test)) &&
	    !(gd->flags & GD_FLG_FDT_CHANGED)) {
		uts->of_live = false;
		ret = ut_run_test(uts, test, leaf ?: test->name);
		if (ret != -EAGAIN) {
			ut_assertok(ret);
			runs++;
		}
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
 *	the first call to this function. On exit, @uts->cur.fail_count is
 *	incremented by the number of failures (0, one hopes)
 * @prefix: String prefix for the tests. Any tests that have this prefix will be
 *	printed without the prefix, so that it is easier to see the unique part
 *	of the test name. If NULL, no prefix processing is done
 * @tests: List of tests to run
 * @count: Number of tests to run
 * @select_name: Name of a single test to run (from the list provided). If NULL
 *	then all tests are run
 * @test_insert: String describing a test to run after n other tests run, in the
 * format n:name where n is the number of tests to run before this one and
 * name is the name of the test to run
 * Return: 0 if all tests passed, -ENOENT if test @select_name was not found,
 *	-EBADF if any failed
 */
static int ut_run_tests(struct unit_test_state *uts, const char *prefix,
			struct unit_test *tests, int count,
			const char *select_name, const char *test_insert)
{
	int prefix_len = prefix ? strlen(prefix) : 0;
	struct unit_test *test, *one;
	int found = 0;
	int pos = 0;
	int upto;

	one = NULL;
	if (test_insert) {
		char *p;

		pos = dectoul(test_insert, NULL);
		p = strchr(test_insert, ':');
		if (p)
			p++;

		for (test = tests; test < tests + count; test++) {
			if (!strcmp(p, test->name))
				one = test;
		}
	}

	for (upto = 0, test = tests; test < tests + count; test++, upto++) {
		const char *test_name = test->name;
		int ret, i, old_fail_count;

		if (!(test->flags & (UTF_INIT | UTF_UNINIT)) &&
		    !test_matches(prefix, test_name, select_name))
			continue;

		if (test->flags & UTF_MANUAL) {
			int len;

			/*
			 * manual tests must have a name ending "_norun" as this
			 * is how pytest knows to skip them. See
			 * generate_ut_subtest() for this check.
			 */
			len = strlen(test_name);
			if (len < 6 || strcmp(test_name + len - 6, "_norun")) {
				printf("Test '%s' is manual so must have a name ending in _norun\n",
				       test_name);
				uts->cur.fail_count++;
				return -EBADF;
			}
			if (!uts->force_run) {
				printf("Test: %s: skipped as it is manual (use -f to run it)\n",
				       test_name);
				continue;
			}
		}
		old_fail_count = uts->cur.fail_count;

		uts->cur.test_count++;
		if (one && upto == pos) {
			ret = ut_run_test_live_flat(uts, one, NULL);
			if (uts->cur.fail_count != old_fail_count) {
				printf("Test '%s' failed %d times (position %d)\n",
				       one->name,
				       uts->cur.fail_count - old_fail_count,
				       pos);
			}
			return -EBADF;
		}

		if (prefix_len && !strncmp(test_name, prefix, prefix_len))
			test_name = test_name + prefix_len;

		for (i = 0; i < uts->runs_per_test; i++)
			ret = ut_run_test_live_flat(uts, test, test_name);
		if (uts->cur.fail_count != old_fail_count) {
			printf("Test '%s' failed %d times\n", test_name,
			       uts->cur.fail_count - old_fail_count);
		}
		found++;
		if (ret == -EAGAIN)
			continue;
		if (ret)
			return ret;
	}
	if (select_name && !found)
		return -ENOENT;

	return uts->cur.fail_count ? -EBADF : 0;
}

void ut_report(struct ut_stats *stats, int run_count)
{
	if (run_count > 1)
		printf("Suites run: %d, total tests", run_count);
	else
		printf("Tests");
	printf(" run: %d, ", stats->test_count);
	if (stats && stats->test_count) {
		ulong dur = stats->duration_ms;

		printf("%ld ms, average: %ld ms, ", dur,
		       dur ? dur / stats->test_count : 0);
	}
	if (stats->skip_count)
		printf("skipped: %d, ", stats->skip_count);
	printf("failures: %d\n", stats->fail_count);
}

int ut_run_list(struct unit_test_state *uts, const char *category,
		const char *prefix, struct unit_test *tests, int count,
		const char *select_name, int runs_per_test, bool force_run,
		const char *test_insert)
{
	;
	bool has_dm_tests = false;
	ulong start_offset = 0;
	ulong test_offset = 0;
	int ret;

	memset(&uts->cur, '\0', sizeof(struct ut_stats));
	if (CONFIG_IS_ENABLED(UNIT_TEST_DURATION)) {
		uts->cur.start = get_timer(0);
		start_offset = timer_test_get_offset();
	}

	if (!CONFIG_IS_ENABLED(OF_PLATDATA) &&
	    ut_list_has_dm_tests(tests, count, prefix, select_name)) {
		has_dm_tests = true;
		/*
		 * If we have no device tree, or it only has a root node, then
		 * these tests clearly aren't going to work...
		 */
		if (!gd->fdt_blob || fdt_next_node(gd->fdt_blob, 0, NULL) < 0) {
			puts("Please run with test device tree:\n"
			     "    ./u-boot -d arch/sandbox/dts/test.dtb\n");
			return CMD_RET_FAILURE;
		}
	}

	if (!select_name)
		printf("Running %d %s tests\n", count, category);

	uts->of_root = gd_of_root();
	uts->runs_per_test = runs_per_test;
	if (fdt_action() == FDTCHK_COPY && gd->fdt_blob) {
		uts->fdt_size = fdt_totalsize(gd->fdt_blob);
		uts->fdt_copy = os_malloc(uts->fdt_size);
		if (!uts->fdt_copy) {
			printf("Out of memory for device tree copy\n");
			return -ENOMEM;
		}
		memcpy(uts->fdt_copy, gd->fdt_blob, uts->fdt_size);
	}
	uts->force_run = force_run;
	ret = ut_run_tests(uts, prefix, tests, count, select_name,
			   test_insert);

	/* Best efforts only...ignore errors */
	if (has_dm_tests)
		dm_test_restore(uts->of_root);

	if (ret == -ENOENT)
		printf("Test '%s' not found\n", select_name);
	if (CONFIG_IS_ENABLED(UNIT_TEST_DURATION)) {
		test_offset = timer_test_get_offset() - start_offset;

		uts->cur.duration_ms = get_timer(uts->cur.start) - test_offset;
	}
	ut_report(&uts->cur, 1);

	uts->total.skip_count += uts->cur.skip_count;
	uts->total.fail_count += uts->cur.fail_count;
	uts->total.test_count += uts->cur.test_count;
	uts->total.duration_ms += uts->cur.duration_ms;
	uts->run_count++;

	return ret;
}
