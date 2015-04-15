/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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
