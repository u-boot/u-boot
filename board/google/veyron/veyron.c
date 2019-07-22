// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <asm/arch-rockchip/clock.h>

/*
 * We should increase the DDR voltage to 1.2V using the PWM regulator.
 * There is a U-Boot driver for this but it may need to add support for the
 * 'voltage-table' property.
 */

int board_early_init_f(void)
{
	struct udevice *dev;
	int ret;

	/*
	 * This init is done in SPL, but when chain-loading U-Boot SPL will
	 * have been skipped. Allow the clock driver to check if it needs
	 * setting up.
	 */
	ret = rockchip_get_clk(&dev);
	if (ret) {
		debug("CLK init failed: %d\n", ret);
		return ret;
	}

	return 0;
}
