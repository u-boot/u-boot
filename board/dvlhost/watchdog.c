/*
 * (C) Copyright 2009
 * Michael Schwingen, michael@schwingen.org
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include "dvlhost_hw.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#include <asm/arch/ixp425.h>

void hw_watchdog_reset(void)
{
	unsigned int x;
	x = readl(IXP425_GPIO_GPOUTR);
	x ^= (1 << (CONFIG_SYS_GPIO_WDGTRIGGER));
	writel(x, IXP425_GPIO_GPOUTR);
}

#endif /* CONFIG_HW_WATCHDOG */
