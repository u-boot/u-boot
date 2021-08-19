// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/gpio.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that watchdog driver functions are called */
static int dm_test_wdt_base(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();
	struct udevice *dev;
	const u64 timeout = 42;

	ut_assertok(uclass_get_device_by_driver(UCLASS_WDT,
						DM_DRIVER_GET(wdt_sandbox), &dev));
	ut_assertnonnull(dev);
	ut_asserteq(0, state->wdt.counter);
	ut_asserteq(false, state->wdt.running);

	ut_assertok(wdt_start(dev, timeout, 0));
	ut_asserteq(timeout, state->wdt.counter);
	ut_asserteq(true, state->wdt.running);

	uint reset_count = state->wdt.reset_count;
	ut_assertok(wdt_reset(dev));
	ut_asserteq(reset_count + 1, state->wdt.reset_count);
	ut_asserteq(true, state->wdt.running);

	ut_assertok(wdt_stop(dev));
	ut_asserteq(false, state->wdt.running);

	return 0;
}
DM_TEST(dm_test_wdt_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_wdt_gpio(struct unit_test_state *uts)
{
	/*
	 * The sandbox wdt gpio is "connected" to gpio bank a, offset
	 * 7. Use the sandbox back door to verify that the gpio-wdt
	 * driver behaves as expected.
	 */
	struct udevice *wdt, *gpio;
	const u64 timeout = 42;
	const int offset = 7;
	int val;

	ut_assertok(uclass_get_device_by_driver(UCLASS_WDT,
						DM_DRIVER_GET(wdt_gpio), &wdt));
	ut_assertnonnull(wdt);

	ut_assertok(uclass_get_device_by_name(UCLASS_GPIO, "base-gpios", &gpio));
	ut_assertnonnull(gpio);
	ut_assertok(wdt_start(wdt, timeout, 0));

	val = sandbox_gpio_get_value(gpio, offset);
	ut_assertok(wdt_reset(wdt));
	ut_asserteq(!val, sandbox_gpio_get_value(gpio, offset));
	ut_assertok(wdt_reset(wdt));
	ut_asserteq(val, sandbox_gpio_get_value(gpio, offset));

	ut_asserteq(-ENOSYS, wdt_stop(wdt));

	return 0;
}
DM_TEST(dm_test_wdt_gpio, UT_TESTF_SCAN_FDT);
