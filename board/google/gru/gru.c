// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google
 */

#include <common.h>
#include <dm.h>
#include <init.h>

#ifdef CONFIG_SPL_BUILD
/* provided to defeat compiler optimisation in board_init_f() */
void gru_dummy_function(int i)
{
}

int board_early_init_f(void)
{
# ifdef CONFIG_TARGET_CHROMEBOOK_BOB
	int sum, i;

	/*
	 * Add a delay and ensure that the compiler does not optimise this out.
	 * This is needed since the power rails tail a while to turn on, and
	 * we get garbage serial output otherwise.
	 */
	sum = 0;
	for (i = 0; i < 150000; i++)
		sum += i;
	gru_dummy_function(sum);
#endif /* CONFIG_TARGET_CHROMEBOOK_BOB */

	return 0;
}
#endif

#ifndef CONFIG_SPL_BUILD
int board_early_init_r(void)
{
	struct udevice *clk;
	int ret;

	/*
	 * This init is done in SPL, but when chain-loading U-Boot SPL will
	 * have been skipped. Allow the clock driver to check if it needs
	 * setting up.
	 */
	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_DRIVER_GET(clk_rk3399), &clk);
	if (ret) {
		debug("%s: CLK init failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}
#endif
