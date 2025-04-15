// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <asm/io.h>
#include <linux/bitops.h>

#define TIMER_LOAD_COUNT_L	0x00
#define TIMER_LOAD_COUNT_H	0x04
#define TIMER_CONTROL_REG	0x10
#define TIMER_EN	0x1
#define	TIMER_FMODE	BIT(0)
#define	TIMER_RMODE	BIT(1)

__weak void rockchip_stimer_init(void)
{
#if defined(CONFIG_ROCKCHIP_STIMER_BASE)
	/* If Timer already enabled, don't re-init it */
	u32 reg = readl(CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);

	if (reg & TIMER_EN)
		return;

#ifndef CONFIG_ARM64
	asm volatile("mcr p15, 0, %0, c14, c0, 0"
		     : : "r"(CONFIG_COUNTER_FREQUENCY));
#endif

	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 4);
	writel(TIMER_EN | TIMER_FMODE, CONFIG_ROCKCHIP_STIMER_BASE +
	       TIMER_CONTROL_REG);
#endif
}
