// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/clk.h>
#include <dm/test.h>
#include <dm/uclass.h>
#include <linux/err.h>
#include <test/ut.h>
#include <sandbox-clk.h>

/* Tests for Common Clock Framework driver */
static int dm_test_clk_ccf(struct unit_test_state *uts)
{
	struct clk *clk, *pclk;
	struct udevice *dev;
	long long rate;
	int ret;

	/* Get the device using the clk device */
	ut_assertok(uclass_get_device_by_name(UCLASS_CLK, "clk-ccf", &dev));

	/* Test for clk_get_by_id() */
	ret = clk_get_by_id(SANDBOX_CLK_ECSPI_ROOT, &clk);
	ut_assertok(ret);
	ut_asserteq_str("ecspi_root", clk->dev->name);

	/* Test for clk_get_parent_rate() */
	ret = clk_get_by_id(SANDBOX_CLK_ECSPI1, &clk);
	ut_assertok(ret);
	ut_asserteq_str("ecspi1", clk->dev->name);

	rate = clk_get_parent_rate(clk);
	ut_asserteq(rate, 20000000);

	/* Test the mux of CCF */
	ret = clk_get_by_id(SANDBOX_CLK_USDHC1_SEL, &clk);
	ut_assertok(ret);
	ut_asserteq_str("usdhc1_sel", clk->dev->name);

	rate = clk_get_parent_rate(clk);
	ut_asserteq(rate, 60000000);

	ret = clk_get_by_id(SANDBOX_CLK_USDHC2_SEL, &clk);
	ut_assertok(ret);
	ut_asserteq_str("usdhc2_sel", clk->dev->name);

	rate = clk_get_parent_rate(clk);
	ut_asserteq(rate, 80000000);

	pclk = clk_get_parent(clk);
	ut_asserteq_str("pll3_80m", pclk->dev->name);

	/* Test the composite of CCF */
	ret = clk_get_by_id(SANDBOX_CLK_I2C, &clk);
	ut_assertok(ret);
	ut_asserteq_str("i2c", clk->dev->name);

	rate = clk_get_rate(clk);
	ut_asserteq(rate, 60000000);

	return 1;
}

DM_TEST(dm_test_clk_ccf, DM_TESTF_SCAN_FDT);
