/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/test.h>
#include <dm/test.h>
#include <linux/err.h>
#include <test/ut.h>

/* Test that we can find and adjust clocks */
static int dm_test_clk_base(struct unit_test_state *uts)
{
	struct udevice *clk;
	ulong rate;

	ut_assertok(uclass_get_device(UCLASS_CLK, 0, &clk));
	rate = clk_get_rate(clk);
	ut_asserteq(SANDBOX_CLK_RATE, rate);
	ut_asserteq(-EINVAL, clk_set_rate(clk, 0));
	ut_assertok(clk_set_rate(clk, rate * 2));
	ut_asserteq(SANDBOX_CLK_RATE * 2, clk_get_rate(clk));

	return 0;
}
DM_TEST(dm_test_clk_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that peripheral clocks work as expected */
static int dm_test_clk_periph(struct unit_test_state *uts)
{
	struct udevice *clk;
	ulong rate;

	ut_assertok(uclass_get_device(UCLASS_CLK, 0, &clk));
	rate = clk_set_periph_rate(clk, PERIPH_ID_COUNT, 123);
	ut_asserteq(-EINVAL, rate);
	ut_asserteq(1, IS_ERR_VALUE(rate));

	rate = clk_set_periph_rate(clk, PERIPH_ID_SPI, 123);
	ut_asserteq(0, rate);
	ut_asserteq(123, clk_get_periph_rate(clk, PERIPH_ID_SPI));

	rate = clk_set_periph_rate(clk, PERIPH_ID_SPI, 1234);
	ut_asserteq(123, rate);

	rate = clk_set_periph_rate(clk, PERIPH_ID_I2C, 567);

	rate = clk_set_periph_rate(clk, PERIPH_ID_SPI, 1234);
	ut_asserteq(1234, rate);

	ut_asserteq(567, clk_get_periph_rate(clk, PERIPH_ID_I2C));

	return 0;
}
DM_TEST(dm_test_clk_periph, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
