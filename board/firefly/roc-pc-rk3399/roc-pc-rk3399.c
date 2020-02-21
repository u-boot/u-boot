// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <asm/arch-rockchip/periph.h>
#include <power/regulator.h>
#include <spl_gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/gpio.h>
#include <asm/arch-rockchip/grf_rk3399.h>

#ifndef CONFIG_SPL_BUILD
int board_early_init_f(void)
{
	struct udevice *regulator;
	int ret;

	ret = regulator_get_by_platname("vcc5v0_host", &regulator);
	if (ret) {
		debug("%s vcc5v0_host init fail! ret %d\n", __func__, ret);
		goto out;
	}

	ret = regulator_set_enable(regulator, true);
	if (ret)
		debug("%s vcc5v0-host-en set fail! ret %d\n", __func__, ret);
out:
	return 0;
}
#endif

#if defined(CONFIG_TPL_BUILD)

#define PMUGRF_BASE     0xff320000
#define GPIO0_BASE      0xff720000

int board_early_init_f(void)
{
	struct rockchip_gpio_regs * const gpio0 = (void *)GPIO0_BASE;
	struct rk3399_pmugrf_regs * const pmugrf = (void *)PMUGRF_BASE;

	/**
	 * 1. Glow yellow LED, termed as low power
	 * 2. Poll for on board power key press
	 * 3. Once 2 done, off yellow and glow red LED, termed as full power
	 * 4. Continue booting...
	 */
	spl_gpio_output(gpio0, GPIO(BANK_A, 2), 1);

	spl_gpio_set_pull(&pmugrf->gpio0_p, GPIO(BANK_A, 5), GPIO_PULL_NORMAL);
	while (readl(&gpio0->ext_port) & 0x20);

	spl_gpio_output(gpio0, GPIO(BANK_A, 2), 0);
	spl_gpio_output(gpio0, GPIO(BANK_B, 5), 1);

	return 0;
}
#endif
