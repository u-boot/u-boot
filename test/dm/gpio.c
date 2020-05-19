// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Google, Inc
 */

#include <common.h>
#include <fdtdec.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <asm/gpio.h>
#include <test/ut.h>

/* Test that sandbox GPIOs work correctly */
static int dm_test_gpio(struct unit_test_state *uts)
{
	unsigned int offset, gpio;
	struct dm_gpio_ops *ops;
	struct udevice *dev;
	const char *name;
	int offset_count;
	char buf[80];

	/*
	 * We expect to get 4 banks. One is anonymous (just numbered) and
	 * comes from platdata. The other are named a (20 gpios),
	 * b (10 gpios) and c (10 gpios) and come from the device tree. See
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

	/* Make it an open drain output, and reset it */
	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_dir_flags(dev, offset));
	ut_assertok(ops->set_dir_flags(dev, offset,
				       GPIOD_IS_OUT | GPIOD_OPEN_DRAIN));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN,
		    sandbox_gpio_get_dir_flags(dev, offset));
	ut_assertok(ops->set_dir_flags(dev, offset,
				       GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_dir_flags(dev, offset));

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

/* Test that GPIO open-drain/open-source emulation works correctly */
static int dm_test_gpio_opendrain_opensource(struct unit_test_state *uts)
{
	struct gpio_desc desc_list[8];
	struct udevice *dev, *gpio_c;
	char buf[80];

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(uclass_get_device(UCLASS_GPIO, 3, &gpio_c));
	ut_asserteq_str("pinmux-gpios", gpio_c->name);

	ut_asserteq(8, gpio_request_list_by_name(dev, "test3-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0))

	ut_asserteq(true, !!device_active(gpio_c));
	ut_asserteq_ptr(gpio_c, desc_list[0].dev);
	ut_asserteq_ptr(gpio_c, desc_list[1].dev);
	ut_asserteq_ptr(gpio_c, desc_list[2].dev);
	ut_asserteq_ptr(gpio_c, desc_list[3].dev);
	ut_asserteq_ptr(gpio_c, desc_list[4].dev);
	ut_asserteq_ptr(gpio_c, desc_list[5].dev);
	ut_asserteq_ptr(gpio_c, desc_list[6].dev);
	ut_asserteq_ptr(gpio_c, desc_list[7].dev);

	/* GPIO 0 is (GPIO_OUT|GPIO_OPEN_DRAIN) */
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN,
		    sandbox_gpio_get_dir_flags(gpio_c, 0));

	/* Set it as output high, should become an input */
	ut_assertok(dm_gpio_set_value(&desc_list[0], 1));
	ut_assertok(gpio_get_status(gpio_c, 0, buf, sizeof(buf)));
	ut_asserteq_str("c0: input: 0 [x] a-test.test3-gpios0", buf);

	/* Set it as output low, should become output low */
	ut_assertok(dm_gpio_set_value(&desc_list[0], 0));
	ut_assertok(gpio_get_status(gpio_c, 0, buf, sizeof(buf)));
	ut_asserteq_str("c0: output: 0 [x] a-test.test3-gpios0", buf);

	/* GPIO 1 is (GPIO_OUT|GPIO_OPEN_SOURCE) */
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_SOURCE,
		    sandbox_gpio_get_dir_flags(gpio_c, 1));

	/* Set it as output high, should become output high */
	ut_assertok(dm_gpio_set_value(&desc_list[1], 1));
	ut_assertok(gpio_get_status(gpio_c, 1, buf, sizeof(buf)));
	ut_asserteq_str("c1: output: 1 [x] a-test.test3-gpios1", buf);

	/* Set it as output low, should become an input */
	ut_assertok(dm_gpio_set_value(&desc_list[1], 0));
	ut_assertok(gpio_get_status(gpio_c, 1, buf, sizeof(buf)));
	ut_asserteq_str("c1: input: 1 [x] a-test.test3-gpios1", buf);

	/* GPIO 6 is (GPIO_ACTIVE_LOW|GPIO_OUT|GPIO_OPEN_DRAIN) */
	ut_asserteq(GPIOD_ACTIVE_LOW | GPIOD_IS_OUT | GPIOD_OPEN_DRAIN,
		    sandbox_gpio_get_dir_flags(gpio_c, 6));

	/* Set it as output high, should become output low */
	ut_assertok(dm_gpio_set_value(&desc_list[6], 1));
	ut_assertok(gpio_get_status(gpio_c, 6, buf, sizeof(buf)));
	ut_asserteq_str("c6: output: 0 [x] a-test.test3-gpios6", buf);

	/* Set it as output low, should become an input */
	ut_assertok(dm_gpio_set_value(&desc_list[6], 0));
	ut_assertok(gpio_get_status(gpio_c, 6, buf, sizeof(buf)));
	ut_asserteq_str("c6: input: 0 [x] a-test.test3-gpios6", buf);

	/* GPIO 7 is (GPIO_ACTIVE_LOW|GPIO_OUT|GPIO_OPEN_SOURCE) */
	ut_asserteq(GPIOD_ACTIVE_LOW | GPIOD_IS_OUT | GPIOD_OPEN_SOURCE,
		    sandbox_gpio_get_dir_flags(gpio_c, 7));

	/* Set it as output high, should become an input */
	ut_assertok(dm_gpio_set_value(&desc_list[7], 1));
	ut_assertok(gpio_get_status(gpio_c, 7, buf, sizeof(buf)));
	ut_asserteq_str("c7: input: 0 [x] a-test.test3-gpios7", buf);

	/* Set it as output low, should become output high */
	ut_assertok(dm_gpio_set_value(&desc_list[7], 0));
	ut_assertok(gpio_get_status(gpio_c, 7, buf, sizeof(buf)));
	ut_asserteq_str("c7: output: 1 [x] a-test.test3-gpios7", buf);

	ut_assertok(gpio_free_list(dev, desc_list, 8));

	return 0;
}
DM_TEST(dm_test_gpio_opendrain_opensource,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that sandbox anonymous GPIOs work correctly */
static int dm_test_gpio_anon(struct unit_test_state *uts)
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
static int dm_test_gpio_requestf(struct unit_test_state *uts)
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
static int dm_test_gpio_copy(struct unit_test_state *uts)
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
static int dm_test_gpio_leak(struct unit_test_state *uts)
{
	ut_assertok(dm_test_gpio(uts));
	ut_assertok(dm_test_gpio_anon(uts));
	ut_assertok(dm_test_gpio_requestf(uts));
	ut_assertok(dm_leak_check_end(uts));

	return 0;
}
DM_TEST(dm_test_gpio_leak, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can find GPIOs using phandles */
static int dm_test_gpio_phandles(struct unit_test_state *uts)
{
	struct gpio_desc desc, desc_list[8], desc_list2[8];
	struct udevice *dev, *gpio_a, *gpio_b;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(gpio_request_by_name(dev, "test-gpios", 1, &desc, 0));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio_a));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 2, &gpio_b));
	ut_asserteq_str("base-gpios", gpio_a->name);
	ut_asserteq(true, !!device_active(gpio_a));
	ut_asserteq_ptr(gpio_a, desc.dev);
	ut_asserteq(4, desc.offset);
	/* GPIOF_INPUT is the sandbox GPIO driver default */
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_a, 4, NULL));
	ut_assertok(dm_gpio_free(dev, &desc));

	ut_asserteq(-ENOENT, gpio_request_by_name(dev, "test-gpios", 3, &desc,
						  0));
	ut_asserteq_ptr(NULL, desc.dev);
	ut_asserteq(desc.offset, 0);
	ut_asserteq(-ENOENT, gpio_request_by_name(dev, "test-gpios", 5, &desc,
						  0));

	/* Last GPIO is ignord as it comes after <0> */
	ut_asserteq(3, gpio_request_list_by_name(dev, "test-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));
	ut_asserteq(-EBUSY, gpio_request_list_by_name(dev, "test-gpios",
						      desc_list2,
						      ARRAY_SIZE(desc_list2),
						      0));
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_a, 4, NULL));
	ut_assertok(gpio_free_list(dev, desc_list, 3));
	ut_asserteq(GPIOF_UNUSED, gpio_get_function(gpio_a, 4, NULL));
	ut_asserteq(3, gpio_request_list_by_name(dev,  "test-gpios", desc_list,
						 ARRAY_SIZE(desc_list),
						 GPIOD_IS_OUT |
						 GPIOD_IS_OUT_ACTIVE));
	ut_asserteq(GPIOF_OUTPUT, gpio_get_function(gpio_a, 4, NULL));
	ut_asserteq_ptr(gpio_a, desc_list[0].dev);
	ut_asserteq(1, desc_list[0].offset);
	ut_asserteq_ptr(gpio_a, desc_list[1].dev);
	ut_asserteq(4, desc_list[1].offset);
	ut_asserteq_ptr(gpio_b, desc_list[2].dev);
	ut_asserteq(5, desc_list[2].offset);
	ut_asserteq(1, dm_gpio_get_value(desc_list));
	ut_assertok(gpio_free_list(dev, desc_list, 3));

	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_dir_flags(gpio_a, 1));
	ut_asserteq(6, gpio_request_list_by_name(dev, "test2-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));

	/* This was set to output previously but flags resetted to 0 = INPUT */
	ut_asserteq(0, sandbox_gpio_get_dir_flags(gpio_a, 1));
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_a, 1, NULL));

	/* Active low should invert the input value */
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_b, 6, NULL));
	ut_asserteq(1, dm_gpio_get_value(&desc_list[2]));

	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_b, 7, NULL));
	ut_asserteq(GPIOF_OUTPUT, gpio_get_function(gpio_b, 8, NULL));
	ut_asserteq(0, dm_gpio_get_value(&desc_list[4]));
	ut_asserteq(GPIOF_OUTPUT, gpio_get_function(gpio_b, 9, NULL));
	ut_asserteq(1, dm_gpio_get_value(&desc_list[5]));

	return 0;
}
DM_TEST(dm_test_gpio_phandles, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Check the gpio pin configuration get from device tree information */
static int dm_test_gpio_get_dir_flags(struct unit_test_state *uts)
{
	struct gpio_desc desc_list[6];
	struct udevice *dev;
	ulong flags;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));

	ut_asserteq(6, gpio_request_list_by_name(dev, "test3-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));

	ut_assertok(dm_gpio_get_dir_flags(&desc_list[0], &flags));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN, flags);

	ut_assertok(dm_gpio_get_dir_flags(&desc_list[1], &flags));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_SOURCE, flags);

	ut_assertok(dm_gpio_get_dir_flags(&desc_list[2], &flags));
	ut_asserteq(GPIOD_IS_OUT, flags);

	ut_assertok(dm_gpio_get_dir_flags(&desc_list[3], &flags));
	ut_asserteq(GPIOD_IS_IN | GPIOD_PULL_UP, flags);

	ut_assertok(dm_gpio_get_dir_flags(&desc_list[4], &flags));
	ut_asserteq(GPIOD_IS_IN | GPIOD_PULL_DOWN, flags);

	ut_assertok(dm_gpio_get_dir_flags(&desc_list[5], &flags));
	ut_asserteq(GPIOD_IS_IN, flags);

	ut_assertok(gpio_free_list(dev, desc_list, 6));

	return 0;
}
DM_TEST(dm_test_gpio_get_dir_flags, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
