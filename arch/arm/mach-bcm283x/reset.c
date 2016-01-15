/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/wdog.h>

#define RESET_TIMEOUT 10

void reset_cpu(ulong addr)
{
	struct bcm2835_wdog_regs *regs =
		(struct bcm2835_wdog_regs *)BCM2835_WDOG_PHYSADDR;
	uint32_t rstc;

	rstc = readl(&regs->rstc);
	rstc &= ~BCM2835_WDOG_RSTC_WRCFG_MASK;
	rstc |= BCM2835_WDOG_RSTC_WRCFG_FULL_RESET;

	writel(BCM2835_WDOG_PASSWORD | RESET_TIMEOUT, &regs->wdog);
	writel(BCM2835_WDOG_PASSWORD | rstc, &regs->rstc);
}
