/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <led.h>
#include <asm/gpio.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Base test of the led uclass */
static int dm_test_led_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Get the top-level device */
	ut_assertok(uclass_get_device(UCLASS_LED, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 2, &dev));
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_LED, 3, &dev));

	return 0;
}
DM_TEST(dm_test_led_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test of the led uclass using the led_gpio driver */
static int dm_test_led_gpio(struct unit_test_state *uts)
{
	const int offset = 1;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate an LED. LED 1 is connected to GPIO
	 * bank gpio_a, offset 1.
	 */
	ut_assertok(uclass_get_device(UCLASS_LED, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	led_set_on(dev, 1);
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	led_set_on(dev, 0);
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));

	return 0;
}
DM_TEST(dm_test_led_gpio, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test obtaining an LED by label */
static int dm_test_led_label(struct unit_test_state *uts)
{
	struct udevice *dev, *cmp;

	ut_assertok(led_get_by_label("sandbox:red", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 1, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_assertok(led_get_by_label("sandbox:green", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 2, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_asserteq(-ENODEV, led_get_by_label("sandbox:blue", &dev));

	return 0;
}
DM_TEST(dm_test_led_label, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
