// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026 Nokia
 */

#include <command.h>
#include <console.h>
#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>
#include <wdt.h>

/* Test the wdt gettimeout command supported path */
static int dm_test_wdt_cmd_gettimeout(struct unit_test_state *uts)
{
	struct udevice *dev;
	const u64 timeout = 5000;

	ut_assertok(uclass_get_device_by_driver(UCLASS_WDT,
						DM_DRIVER_GET(wdt_sandbox), &dev));
	ut_assertnonnull(dev);

	ut_assertok(run_commandf("wdt dev %s", dev->name));
	ut_assertok(run_commandf("wdt start %llu", timeout));

	console_record_reset();
	ut_assertok(run_commandf("wdt gettimeout"));
	ut_assert_nextline("Timeout: %llums", timeout);
	ut_assert_console_end();

	ut_assertok(run_commandf("wdt stop"));

	console_record_reset();
	ut_asserteq(CMD_RET_FAILURE, run_commandf("wdt gettimeout"));
	ut_assert_nextline("Watchdog not running.");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_wdt_cmd_gettimeout, UTF_SCAN_FDT | UTF_CONSOLE);

/* Test the wdt gettimeout command unsupported path */
static int dm_test_wdt_cmd_gettimeout_unsupported(struct unit_test_state *uts)
{
	struct udevice *wdt;

	ut_assertok(uclass_get_device_by_name(UCLASS_WDT,
					      "wdt-gpio-toggle", &wdt));
	ut_assertnonnull(wdt);

	ut_assertok(run_commandf("wdt dev %s", wdt->name));
	console_record_reset();
	ut_asserteq(CMD_RET_FAILURE, run_commandf("wdt gettimeout"));
	ut_assert_nextline("Getting watchdog timeout not supported.");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_wdt_cmd_gettimeout_unsupported, UTF_SCAN_FDT | UTF_CONSOLE);
