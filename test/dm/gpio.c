/*
 * Copyright (C) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <dm.h>
#include <dm/root.h>
#include <dm/ut.h>
#include <dm/test.h>
#include <dm/util.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test that sandbox GPIOs work correctly */
static int dm_test_gpio(struct dm_test_state *dms)
{
	unsigned int offset, gpio;
	struct dm_gpio_ops *ops;
	struct udevice *dev;
	const char *name;
	int offset_count;
	char buf[80];

	/*
	 * We expect to get 3 banks. One is anonymous (just numbered) and
	 * comes from platdata. The other two are named a (20 gpios)
	 * and b (10 gpios) and come from the device tree. See
	 * test/dm/test.dts.
	 */
	ut_assertok(gpio_lookup_name("b4", &dev, &offset, &gpio));
	ut_asserteq_str(dev->name, "extra-gpios");
	ut_asserteq(4, offset);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT + 20 + 4, gpio);

	name = gpio_get_bank_info(dev, &offset_count);
	ut_asserteq_str("b", name);
	ut_asserteq(10, offset_count);

	/* Get the operations for this device */
	ops = gpio_get_ops(dev);
	ut_assert(ops->get_function);

	/* Cannot get a value until it is reserved */
	ut_asserteq(-EBUSY, gpio_get_value(gpio + 1));
	/*
	 * Now some tests that use the 'sandbox' back door. All GPIOs
	 * should default to input, include b4 that we are using here.
	 */
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 0 [ ]", buf);

	/* Change it to an output */
	sandbox_gpio_set_direction(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 0 [ ]", buf);

	sandbox_gpio_set_value(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 1 [ ]", buf);

	ut_assertok(gpio_request(gpio, "testing"));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 1 [x] testing", buf);

	/* Change the value a bit */
	ut_asserteq(1, ops->get_value(dev, offset));
	ut_assertok(ops->set_value(dev, offset, 0));
	ut_asserteq(0, ops->get_value(dev, offset));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 0 [x] testing", buf);
	ut_assertok(ops->set_value(dev, offset, 1));
	ut_asserteq(1, ops->get_value(dev, offset));

	/* Make it an input */
	ut_assertok(ops->direction_input(dev, offset));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 1 [x] testing", buf);
	sandbox_gpio_set_value(dev, offset, 0);
	ut_asserteq(0, sandbox_gpio_get_value(dev, offset));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 0 [x] testing", buf);

	ut_assertok(gpio_free(gpio));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 0 [ ]", buf);

	/* Check the 'a' bank also */
	ut_assertok(gpio_lookup_name("a15", &dev, &offset, &gpio));
	ut_asserteq_str(dev->name, "base-gpios");
	ut_asserteq(15, offset);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT + 15, gpio);

	name = gpio_get_bank_info(dev, &offset_count);
	ut_asserteq_str("a", name);
	ut_asserteq(20, offset_count);

	return 0;
}
DM_TEST(dm_test_gpio, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that sandbox anonymous GPIOs work correctly */
static int dm_test_gpio_anon(struct dm_test_state *dms)
{
	unsigned int offset, gpio;
	struct udevice *dev;
	const char *name;
	int offset_count;

	/* And the anonymous bank */
	ut_assertok(gpio_lookup_name("14", &dev, &offset, &gpio));
	ut_asserteq_str(dev->name, "gpio_sandbox");
	ut_asserteq(14, offset);
	ut_asserteq(14, gpio);

	name = gpio_get_bank_info(dev, &offset_count);
	ut_asserteq_ptr(NULL, name);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT, offset_count);

	return 0;
}
DM_TEST(dm_test_gpio_anon, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that gpio_requestf() works as expected */
static int dm_test_gpio_requestf(struct dm_test_state *dms)
{
	unsigned int offset, gpio;
	struct udevice *dev;
	char buf[80];

	ut_assertok(gpio_lookup_name("b5", &dev, &offset, &gpio));
	ut_assertok(gpio_requestf(gpio, "testing %d %s", 1, "hi"));
	sandbox_gpio_set_direction(dev, offset, 1);
	sandbox_gpio_set_value(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b5: output: 1 [x] testing 1 hi", buf);

	return 0;
}
DM_TEST(dm_test_gpio_requestf, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that gpio_request() copies its string */
static int dm_test_gpio_copy(struct dm_test_state *dms)
{
	unsigned int offset, gpio;
	struct udevice *dev;
	char buf[80], name[10];

	ut_assertok(gpio_lookup_name("b6", &dev, &offset, &gpio));
	strcpy(name, "odd_name");
	ut_assertok(gpio_request(gpio, name));
	sandbox_gpio_set_direction(dev, offset, 1);
	sandbox_gpio_set_value(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b6: output: 1 [x] odd_name", buf);
	strcpy(name, "nothing");
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b6: output: 1 [x] odd_name", buf);

	return 0;
}
DM_TEST(dm_test_gpio_copy, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we don't leak memory with GPIOs */
static int dm_test_gpio_leak(struct dm_test_state *dms)
{
	ut_assertok(dm_test_gpio(dms));
	ut_assertok(dm_test_gpio_anon(dms));
	ut_assertok(dm_test_gpio_requestf(dms));
	ut_assertok(dm_leak_check_end(dms));

	return 0;
}

DM_TEST(dm_test_gpio_leak, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
