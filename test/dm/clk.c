// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <clk.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/clk.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <linux/err.h>
#include <test/test.h>
#include <test/ut.h>

/* Base test of the clk uclass */
static int dm_test_clk_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct clk clk_method1;
	struct clk clk_method2;

	/* Get the device using the clk device */
	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "clk-test", &dev));

	/* Get the same clk port in 2 different ways and compare */
	ut_assertok(clk_get_by_index(dev, 0, &clk_method1));
	ut_assertok(clk_get_by_name(dev, NULL, &clk_method2));
	ut_asserteq(clk_is_match(&clk_method1, &clk_method2), true);
	ut_asserteq(clk_method1.id, clk_method2.id);

	ut_assertok(clk_get_by_index(dev, 1, &clk_method1));
	ut_assertok(clk_get_by_index_nodev(dev_ofnode(dev), 1, &clk_method2));
	ut_asserteq(clk_is_match(&clk_method1, &clk_method2), true);
	ut_asserteq(clk_method1.id, clk_method2.id);

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "clk-test2", &dev));
	ut_assertok(clk_set_defaults(dev, CLK_DEFAULTS_PRE));

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "clk-test3", &dev));
	ut_assertok(clk_set_defaults(dev, CLK_DEFAULTS_PRE));

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "clk-test4", &dev));
	ut_assertok(clk_set_defaults(dev, CLK_DEFAULTS_PRE));

	return 0;
}
DM_TEST(dm_test_clk_base, UTF_SCAN_FDT);

static int dm_test_clk(struct unit_test_state *uts)
{
	struct udevice *dev_fixed, *dev_fixed_factor, *dev_clk, *dev_test;
	ulong rate;

	ut_assertok(uclass_get_device_by_name(UCLASS_CLK, "clk-fixed",
					      &dev_fixed));

	ut_assertok(uclass_get_device_by_name(UCLASS_CLK, "clk-fixed-factor",
					      &dev_fixed_factor));

	ut_assertok(uclass_get_device_by_name(UCLASS_CLK, "clk-sbox",
					      &dev_clk));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));
	ut_asserteq(0, sandbox_clk_query_rate(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_rate(dev_clk, SANDBOX_CLK_ID_I2C));

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "clk-test",
					      &dev_test));
	ut_assertok(sandbox_clk_test_get(dev_test));
	ut_assertok(sandbox_clk_test_devm_get(dev_test));
	ut_assertok(sandbox_clk_test_valid(dev_test));

	ut_asserteq(0, sandbox_clk_test_get_rate(dev_test,
						 SANDBOX_CLK_TEST_ID_DEVM_NULL));
	ut_asserteq(0, sandbox_clk_test_set_rate(dev_test,
						 SANDBOX_CLK_TEST_ID_DEVM_NULL,
						 0));
	ut_asserteq(0, sandbox_clk_test_enable(dev_test,
					       SANDBOX_CLK_TEST_ID_DEVM_NULL));
	ut_asserteq(0, sandbox_clk_test_disable(dev_test,
						SANDBOX_CLK_TEST_ID_DEVM_NULL));

	ut_asserteq(1234,
		    sandbox_clk_test_get_rate(dev_test,
					      SANDBOX_CLK_TEST_ID_FIXED));
	ut_asserteq(0, sandbox_clk_test_get_rate(dev_test,
						 SANDBOX_CLK_TEST_ID_SPI));
	ut_asserteq(0, sandbox_clk_test_get_rate(dev_test,
						 SANDBOX_CLK_TEST_ID_I2C));
	ut_asserteq(321, sandbox_clk_test_get_rate(dev_test,
						   SANDBOX_CLK_TEST_ID_DEVM1));
	ut_asserteq(0, sandbox_clk_test_get_rate(dev_test,
						 SANDBOX_CLK_TEST_ID_DEVM2));

	rate = sandbox_clk_test_set_rate(dev_test, SANDBOX_CLK_TEST_ID_FIXED,
					 12345);
	ut_assert(IS_ERR_VALUE(rate));
	rate = sandbox_clk_test_get_rate(dev_test, SANDBOX_CLK_TEST_ID_FIXED);
	ut_asserteq(1234, rate);

	ut_asserteq(0, sandbox_clk_test_set_rate(dev_test,
						 SANDBOX_CLK_TEST_ID_SPI,
						 1000));
	ut_asserteq(0, sandbox_clk_test_set_rate(dev_test,
						 SANDBOX_CLK_TEST_ID_I2C,
						 2000));

	ut_asserteq(1000, sandbox_clk_test_get_rate(dev_test,
						    SANDBOX_CLK_TEST_ID_SPI));
	ut_asserteq(2000, sandbox_clk_test_get_rate(dev_test,
						    SANDBOX_CLK_TEST_ID_I2C));

	ut_asserteq(1000, sandbox_clk_test_set_rate(dev_test,
						    SANDBOX_CLK_TEST_ID_SPI,
						    10000));
	ut_asserteq(2000, sandbox_clk_test_set_rate(dev_test,
						    SANDBOX_CLK_TEST_ID_I2C,
						    20000));

	rate = sandbox_clk_test_set_rate(dev_test, SANDBOX_CLK_TEST_ID_SPI, 0);
	ut_assert(IS_ERR_VALUE(rate));
	rate = sandbox_clk_test_set_rate(dev_test, SANDBOX_CLK_TEST_ID_I2C, 0);
	ut_assert(IS_ERR_VALUE(rate));

	ut_asserteq(10000, sandbox_clk_test_get_rate(dev_test,
						     SANDBOX_CLK_TEST_ID_SPI));
	ut_asserteq(20000, sandbox_clk_test_get_rate(dev_test,
						     SANDBOX_CLK_TEST_ID_I2C));

	ut_asserteq(5000, sandbox_clk_test_round_rate(dev_test,
						      SANDBOX_CLK_TEST_ID_SPI,
						      5000));
	ut_asserteq(7000, sandbox_clk_test_round_rate(dev_test,
						      SANDBOX_CLK_TEST_ID_I2C,
						      7000));

	ut_asserteq(10000, sandbox_clk_test_get_rate(dev_test,
						     SANDBOX_CLK_TEST_ID_SPI));
	ut_asserteq(20000, sandbox_clk_test_get_rate(dev_test,
						     SANDBOX_CLK_TEST_ID_I2C));

	rate = sandbox_clk_test_round_rate(dev_test, SANDBOX_CLK_TEST_ID_SPI, 0);
	ut_assert(IS_ERR_VALUE(rate));
	rate = sandbox_clk_test_round_rate(dev_test, SANDBOX_CLK_TEST_ID_I2C, 0);
	ut_assert(IS_ERR_VALUE(rate));

	ut_asserteq(10000, sandbox_clk_test_get_rate(dev_test,
						     SANDBOX_CLK_TEST_ID_SPI));
	ut_asserteq(20000, sandbox_clk_test_get_rate(dev_test,
						     SANDBOX_CLK_TEST_ID_I2C));

	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));
	ut_asserteq(10000, sandbox_clk_query_rate(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(20000, sandbox_clk_query_rate(dev_clk, SANDBOX_CLK_ID_I2C));

	ut_assertok(sandbox_clk_test_enable(dev_test, SANDBOX_CLK_TEST_ID_SPI));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	ut_assertok(sandbox_clk_test_enable(dev_test, SANDBOX_CLK_TEST_ID_I2C));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	ut_assertok(sandbox_clk_test_disable(dev_test,
					     SANDBOX_CLK_TEST_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	ut_assertok(sandbox_clk_test_disable(dev_test,
					     SANDBOX_CLK_TEST_ID_I2C));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	ut_asserteq(1, sandbox_clk_query_requested(dev_clk,
						   SANDBOX_CLK_ID_SPI));
	ut_asserteq(1, sandbox_clk_query_requested(dev_clk,
						   SANDBOX_CLK_ID_I2C));
	ut_asserteq(1, sandbox_clk_query_requested(dev_clk,
						   SANDBOX_CLK_ID_UART2));

	ut_asserteq(1, sandbox_clk_query_requested(dev_clk,
						   SANDBOX_CLK_ID_UART1));
	ut_assertok(device_remove(dev_test, DM_REMOVE_NORMAL));
	return 0;
}
DM_TEST(dm_test_clk, UTF_SCAN_FDT);

static int dm_test_clk_bulk(struct unit_test_state *uts)
{
	struct udevice *dev_clk, *dev_test;

	ut_assertok(uclass_get_device_by_name(UCLASS_CLK, "clk-sbox",
					      &dev_clk));
	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "clk-test",
					      &dev_test));
	ut_assertok(sandbox_clk_test_get_bulk(dev_test));

	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	/* Fixed clock does not support enable, thus should not fail */
	ut_assertok(sandbox_clk_test_enable_bulk(dev_test));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	/* Fixed clock does not support disable, thus should not fail */
	ut_assertok(sandbox_clk_test_disable_bulk(dev_test));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	/* Fixed clock does not support enable, thus should not fail */
	ut_assertok(sandbox_clk_test_enable_bulk(dev_test));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(1, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));

	/* Fixed clock does not support disable, thus should not fail */
	ut_assertok(sandbox_clk_test_release_bulk(dev_test));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_SPI));
	ut_asserteq(0, sandbox_clk_query_enable(dev_clk, SANDBOX_CLK_ID_I2C));
	ut_assertok(device_remove(dev_test, DM_REMOVE_NORMAL));

	return 0;
}
DM_TEST(dm_test_clk_bulk, UTF_SCAN_FDT);
