// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <dm/root.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

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

int test_pre_run(struct unit_test_state *uts, struct unit_test *test)
{
	/* DM tests have already done this */
	if (!(test->flags & UT_TESTF_DM))
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

	return 0;
}

int test_post_run(struct unit_test_state *uts, struct unit_test *test)
{
	gd->flags &= ~GD_FLG_RECORD;

	return 0;
}

int ut_run_tests(struct unit_test_state *uts, const char *prefix,
		 struct unit_test *tests, int count, const char *select_name)
{
	struct unit_test *test;
	int prefix_len = prefix ? strlen(prefix) : 0;
	int found = 0;

	for (test = tests; test < tests + count; test++) {
		const char *test_name = test->name;
		int ret;

		/* Remove the prefix */
		if (prefix && !strncmp(test_name, prefix, prefix_len))
			test_name += prefix_len;

		if (select_name && strcmp(select_name, test_name))
			continue;
		printf("Test: %s\n", test_name);
		found++;

		ret = test_pre_run(uts, test);
		if (ret == -EAGAIN)
			continue;
		if (ret)
			return ret;

		test->func(uts);

		ret = test_post_run(uts, test);
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
	int ret;

	if (!select_name)
		printf("Running %d %s tests\n", count, category);

	ret = ut_run_tests(&uts, prefix, tests, count, select_name);

	if (ret == -ENOENT)
		printf("Test '%s' not found\n", select_name);
	else
		printf("Failures: %d\n", uts.fail_count);

	return ret;
}
