// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <init.h>
#include <spl.h>
#include <misc.h>
#include <log.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/spl.h>

#define GEM_PHY_RESET	SIFIVE_GENERIC_GPIO_NR(0, 12)

int init_clk_and_ddr(void)
{
	int ret;

	ret = soc_spl_init();
	if (ret) {
		debug("FU540 SPL init failed: %d\n", ret);
		return ret;
	}

	/*
	 * GEMGXL init VSC8541 PHY reset sequence;
	 * leave pull-down active for 2ms
	 */
	udelay(2000);
	ret = gpio_request(GEM_PHY_RESET, "gem_phy_reset");
	if (ret) {
		debug("gem_phy_reset gpio request failed: %d\n", ret);
		return ret;
	}

	/* Set GPIO 12 (PHY NRESET) */
	ret = gpio_direction_output(GEM_PHY_RESET, 1);
	if (ret) {
		debug("gem_phy_reset gpio direction set failed: %d\n", ret);
		return ret;
	}

	udelay(1);

	/* Reset PHY again to enter unmanaged mode */
	gpio_set_value(GEM_PHY_RESET, 0);
	udelay(1);
	gpio_set_value(GEM_PHY_RESET, 1);
	mdelay(15);

	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed: %d\n", ret);

	arch_cpu_init_dm();

	preloader_console_init();

	ret = init_clk_and_ddr();
	if (ret)
		panic("init_clk_and_ddr() failed: %d\n", ret);
}
