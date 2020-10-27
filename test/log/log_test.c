// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support test program
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <test/log.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* emit some sample log records in different ways, for testing */
static int do_log_run(int cat, const char *file)
{
	int i;

	gd->log_fmt = LOGF_TEST;
	debug("debug\n");
	for (i = LOGL_FIRST; i < LOGL_COUNT; i++) {
		log(cat, i, "log %d\n", i);
		_log(log_uc_cat(cat), i, file, 100 + i, "func", "_log %d\n",
		     i);
	}

	gd->log_fmt = log_get_default_format();
	return 0;
}

#define log_run_cat(cat) do_log_run(cat, "file")
#define log_run_file(file) do_log_run(UCLASS_SPI, file)
#define log_run() do_log_run(UCLASS_SPI, "file")

#define EXPECT_LOG BIT(0)
#define EXPECT_DIRECT BIT(1)
#define EXPECT_EXTRA BIT(2)

static int do_check_log_entries(struct unit_test_state *uts, int flags, int min,
				int max)
{
	int i;

	for (i = min; i <= max; i++) {
		if (flags & EXPECT_LOG)
			ut_assert_nextline("do_log_run() log %d", i);
		if (flags & EXPECT_DIRECT)
			ut_assert_nextline("func() _log %d", i);
	}
	if (flags & EXPECT_EXTRA)
		for (; i <= LOGL_MAX ; i++)
			ut_assert_nextline("func() _log %d", i);

	ut_assert_console_end();
	return 0;
}

#define check_log_entries_flags_levels(flags, min, max) do {\
	int ret = do_check_log_entries(uts, flags, min, max); \
	if (ret) \
		return ret; \
} while (0)

#define check_log_entries_flags(flags) \
	check_log_entries_flags_levels(flags, LOGL_FIRST, _LOG_MAX_LEVEL)
#define check_log_entries() check_log_entries_flags(EXPECT_LOG | EXPECT_DIRECT)
#define check_log_entries_extra() \
	check_log_entries_flags(EXPECT_LOG | EXPECT_DIRECT | EXPECT_EXTRA)
#define check_log_entries_none() check_log_entries_flags(0)

/* Check a category filter using the first category */
int log_test_00(struct unit_test_state *uts)
{
	enum log_category_t cat_list[] = {
		log_uc_cat(UCLASS_MMC), log_uc_cat(UCLASS_SPI),
		LOGC_NONE, LOGC_END
	};
	int filt;

	filt = log_add_filter("console", cat_list, LOGL_MAX, NULL);
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_cat(UCLASS_MMC);
	check_log_entries_extra();

	ut_assertok(console_record_reset_enable());
	log_run_cat(UCLASS_SPI);
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_00, UT_TESTF_CONSOLE_REC);

/* Check a category filter that should block log entries */
int log_test_02(struct unit_test_state *uts)
{
	enum log_category_t cat_list[] = {
		log_uc_cat(UCLASS_MMC),  LOGC_NONE, LOGC_END
	};
	int filt;

	filt = log_add_filter("console", cat_list, LOGL_MAX, NULL);
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_cat(UCLASS_SPI);
	check_log_entries_none();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_02, UT_TESTF_CONSOLE_REC);

/* Check passing and failing file filters */
int log_test_03(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_MAX, "file");
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file");
	check_log_entries_flags(EXPECT_DIRECT | EXPECT_EXTRA);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_none();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_03, UT_TESTF_CONSOLE_REC);

/* Check a passing file filter (second in list) */
int log_test_05(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_MAX, "file,file2");
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_flags(EXPECT_DIRECT | EXPECT_EXTRA);

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_05, UT_TESTF_CONSOLE_REC);

/* Check a passing file filter (middle of list) */
int log_test_06(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_MAX,
			      "file,file2,log/log_test.c");
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_06, UT_TESTF_CONSOLE_REC);

/* Check a log level filter */
int log_test_07(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_WARNING, NULL);
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run();
	check_log_entries_flags_levels(EXPECT_LOG | EXPECT_DIRECT, LOGL_FIRST,
				       LOGL_WARNING);

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_07, UT_TESTF_CONSOLE_REC);

/* Check two filters, one of which passes everything */
int log_test_08(struct unit_test_state *uts)
{
	int filt1, filt2;

	filt1 = log_add_filter("console", NULL, LOGL_WARNING, NULL);
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter("console", NULL, LOGL_MAX, NULL);
	ut_assert(filt2 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run();
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	return 0;
}
LOG_TEST_FLAGS(log_test_08, UT_TESTF_CONSOLE_REC);

/* Check three filters, which together pass everything */
int log_test_09(struct unit_test_state *uts)
{
	int filt1, filt2, filt3;

	filt1 = log_add_filter("console", NULL, LOGL_MAX, "file)");
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter("console", NULL, LOGL_MAX, "file2");
	ut_assert(filt2 >= 0);
	filt3 = log_add_filter("console", NULL, LOGL_MAX, "log/log_test.c");
	ut_assert(filt3 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	ut_assertok(log_remove_filter("console", filt3));
	return 0;
}
LOG_TEST_FLAGS(log_test_09, UT_TESTF_CONSOLE_REC);

int do_log_test_10(struct unit_test_state *uts)
{
	int i;

	ut_assertok(console_record_reset_enable());
	log_err("level %d\n", LOGL_EMERG);
	log_err("level %d\n", LOGL_ALERT);
	log_err("level %d\n", LOGL_CRIT);
	log_err("level %d\n", LOGL_ERR);
	log_warning("level %d\n", LOGL_WARNING);
	log_notice("level %d\n", LOGL_NOTICE);
	log_info("level %d\n", LOGL_INFO);
	log_debug("level %d\n", LOGL_DEBUG);
	log_content("level %d\n", LOGL_DEBUG_CONTENT);
	log_io("level %d\n", LOGL_DEBUG_IO);

	for (i = LOGL_EMERG; i <= _LOG_MAX_LEVEL; i++)
		ut_assert_nextline("%s() level %d", __func__, i);
	ut_assert_console_end();
	return 0;
}

int log_test_10(struct unit_test_state *uts)
{
	int ret;

	gd->log_fmt = LOGF_TEST;
	ret = do_log_test_10(uts);
	gd->log_fmt = log_get_default_format();
	return ret;
}
LOG_TEST_FLAGS(log_test_10, UT_TESTF_CONSOLE_REC);

int do_log_test_11(struct unit_test_state *uts)
{
	ut_assertok(console_record_reset_enable());
	log_err("default\n");
	ut_assert_nextline("%s() default", __func__);

	ut_assertok(log_device_set_enable(LOG_GET_DRIVER(console), false));
	log_err("disabled\n");

	ut_assertok(log_device_set_enable(LOG_GET_DRIVER(console), true));
	log_err("enabled\n");
	ut_assert_nextline("%s() enabled", __func__);
	ut_assert_console_end();
	return 0;
}

int log_test_11(struct unit_test_state *uts)
{
	int ret;

	gd->log_fmt = LOGF_TEST;
	ret = do_log_test_10(uts);
	gd->log_fmt = log_get_default_format();
	return ret;
}
LOG_TEST_FLAGS(log_test_11, UT_TESTF_CONSOLE_REC);
