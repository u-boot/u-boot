// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <dm.h>
#include <led.h>
#include <asm/gpio.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Base test of the led uclass */
static int dm_test_led_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Get the top-level device */
	ut_assertok(uclass_get_device(UCLASS_LED, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 2, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 3, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 4, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 5, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 6, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 7, &dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 8, &dev));
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_LED, 9, &dev));

	return 0;
}
DM_TEST(dm_test_led_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of the LED 'default-state' device tree property */
static int dm_test_led_default_state(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Check that we handle the default-state property correctly. */
	ut_assertok(led_get_by_label("sandbox:default_on", &dev));
	ut_asserteq(LEDST_ON, led_get_state(dev));

	/* Also tests default label behaviour */
	ut_assertok(led_get_by_label("default_off", &dev));
	ut_asserteq(LEDST_OFF, led_get_state(dev));

	return 0;
}
DM_TEST(dm_test_led_default_state, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of the led uclass using the led_gpio driver */
static int dm_test_led_gpio(struct unit_test_state *uts)
{
	const int offset = 1;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate an LED. LED 0 is connected to GPIO
	 * bank gpio_a, offset 1.
	 */
	ut_assertok(uclass_get_device(UCLASS_LED, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_assertok(led_set_state(dev, LEDST_ON));
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(LEDST_ON, led_get_state(dev));

	ut_assertok(led_set_state(dev, LEDST_OFF));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(LEDST_OFF, led_get_state(dev));

	return 0;
}
DM_TEST(dm_test_led_gpio, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test that we can toggle LEDs */
static int dm_test_led_toggle(struct unit_test_state *uts)
{
	const int offset = 1;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate an LED. LED 0 is connected to GPIO
	 * bank gpio_a, offset 1.
	 */
	ut_assertok(uclass_get_device(UCLASS_LED, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_assertok(led_set_state(dev, LEDST_TOGGLE));
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(LEDST_ON, led_get_state(dev));

	ut_assertok(led_set_state(dev, LEDST_TOGGLE));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(LEDST_OFF, led_get_state(dev));

	return 0;
}
DM_TEST(dm_test_led_toggle, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test obtaining an LED by label */
static int dm_test_led_label(struct unit_test_state *uts)
{
	struct udevice *dev, *cmp;

	ut_assertok(led_get_by_label("sandbox:red", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 0, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_assertok(led_get_by_label("sandbox:green", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_LED, 1, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_asserteq(-ENODEV, led_get_by_label("sandbox:blue", &dev));

	/* Test if function, color and function-enumerator naming works */
	ut_assertok(led_get_by_label("red:status-20", &dev));

	/* Test if function, color naming works */
	ut_assertok(led_get_by_label("green:status", &dev));

	/* Test if function, without color naming works */
	ut_assertok(led_get_by_label(":status", &dev));

	/* Test if color without function naming works */
	ut_assertok(led_get_by_label("green:", &dev));

	/* Test if function, color naming is ignored if label is found */
	ut_assertok(led_get_by_label("sandbox:function", &dev));

	return 0;
}
DM_TEST(dm_test_led_label, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test LED blinking */
#ifdef CONFIG_LED_BLINK
static int dm_test_led_blink(struct unit_test_state *uts)
{
	const int offset = 1;
	struct udevice *dev, *gpio;

	/*
	 * Check that we get an error when trying to blink an LED, since it is
	 * not supported by the GPIO LED driver.
	 */
	ut_assertok(uclass_get_device(UCLASS_LED, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(-ENOSYS, led_set_state(dev, LEDST_BLINK));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(LEDST_OFF, led_get_state(dev));
	ut_asserteq(-ENOSYS, led_set_period(dev, 100));

	return 0;
}
DM_TEST(dm_test_led_blink, UTF_SCAN_PDATA | UTF_SCAN_FDT);
#endif

/* Test LED boot */
#ifdef CONFIG_LED_BOOT
static int dm_test_led_boot(struct unit_test_state *uts)
{
	struct udevice *dev

	/* options/u-boot/boot-led is set to phandle to "sandbox:green" */
	ut_assertok(led_get_by_label("sandbox:green", &dev));
	ut_asserteq(LEDST_OFF, led_get_state(dev));
	ut_assertok(led_boot_on());
	ut_asserteq(LEDST_ON, led_get_state(dev));
	ut_assertok(led_boot_off());
	ut_asserteq(LEDST_OFF, led_get_state(dev));

	return 0;
}
DM_TEST(dm_test_led_boot, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test LED boot blink fallback */
#ifndef CONFIG_LED_BLINK
static int dm_test_led_boot_blink(struct unit_test_state *uts)
{
	struct udevice *dev

	/* options/u-boot/boot-led is set to phandle to "sandbox:green" */
	ut_assertok(led_get_by_label("sandbox:green", &dev));
	ut_asserteq(LEDST_OFF, led_get_state(dev));
	ut_assertok(led_boot_blink());
	ut_asserteq(LEDST_ON, led_get_state(dev));
	ut_assertok(led_boot_off());
	ut_asserteq(LEDST_OFF, led_get_state(dev));

	return 0;
}
DM_TEST(dm_test_led_boot_blink, UTF_SCAN_PDATA | UTF_SCAN_FDT);
#endif
#endif

/* Test LED activity */
#ifdef CONFIG_LED_ACTIVITY
static int dm_test_led_activity(struct unit_test_state *uts)
{
	struct udevice *dev

	/* options/u-boot/activity-led is set to phandle to "sandbox:red" */
	ut_assertok(led_get_by_label("sandbox:red", &dev));
	ut_asserteq(LEDST_OFF, led_get_state(dev));
	ut_assertok(led_activity_on());
	ut_asserteq(LEDST_ON, led_get_state(dev));
	ut_assertok(led_activity_off());
	ut_asserteq(LEDST_OFF, led_get_state(dev));

	return 0;
}
DM_TEST(dm_test_led_activity, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test LED activity blink fallback */
#ifndef CONFIG_LED_BLINK
static int dm_test_led_activityt_blink(struct unit_test_state *uts)
{
	struct udevice *dev

	/* options/u-boot/activity-led is set to phandle to "sandbox:red" */
	ut_assertok(led_get_by_label("sandbox:red", &dev));
	ut_asserteq(LEDST_OFF, led_get_state(dev));
	ut_assertok(led_activity_blink());
	ut_asserteq(LEDST_ON, led_get_state(dev));
	ut_assertok(led_activity_off());
	ut_asserteq(LEDST_OFF, led_get_state(dev));

	return 0;
}
DM_TEST(dm_test_led_activityt_blink, UTF_SCAN_PDATA | UTF_SCAN_FDT);
#endif
#endif
