// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Based on led.c
 */

#include <common.h>
#include <dm.h>
#include <button.h>
#include <asm/gpio.h>
#include <dm/test.h>
#include <test/ut.h>

/* Base test of the button uclass */
static int dm_test_button_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Get the top-level device */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 2, &dev));
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_BUTTON, 3, &dev));

	return 0;
}
DM_TEST(dm_test_button_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test of the button uclass using the button_gpio driver */
static int dm_test_button_gpio(struct unit_test_state *uts)
{
	const int offset = 3;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate an BUTTON. BUTTON 1 is connected to GPIO
	 * bank gpio_a, offset 3.
	 */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));

	ut_asserteq(0, sandbox_gpio_set_value(gpio, offset, 0));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(BUTTON_OFF, button_get_state(dev));

	ut_asserteq(0, sandbox_gpio_set_value(gpio, offset, 1));
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(BUTTON_ON, button_get_state(dev));

	return 0;
}
DM_TEST(dm_test_button_gpio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test obtaining an BUTTON by label */
static int dm_test_button_label(struct unit_test_state *uts)
{
	struct udevice *dev, *cmp;

	ut_assertok(button_get_by_label("button1", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_assertok(button_get_by_label("button2", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 2, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_asserteq(-ENODEV, button_get_by_label("nobutton", &dev));

	return 0;
}
DM_TEST(dm_test_button_label, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
