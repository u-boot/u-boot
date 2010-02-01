/*
 * Cirrus Logic EP93xx CPU-specific support.
 *
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2004, 2005
 * Cory T. Tusar, Videon Central, Inc., <ctusar@videon-central.com>
 *
 * See file CREDITS for list of people who contributed to this project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <asm/arch/ep93xx.h>
#include <asm/io.h>

/* We reset the CPU by generating a 1-->0 transition on DeviceCfg bit 31. */
extern void reset_cpu(ulong addr)
{
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;
	uint32_t value;

	/* Unlock DeviceCfg and set SWRST */
	writel(0xAA, &syscon->sysswlock);
	value = readl(&syscon->devicecfg);
	value |= SYSCON_DEVICECFG_SWRST;
	writel(value, &syscon->devicecfg);

	/* Unlock DeviceCfg and clear SWRST */
	writel(0xAA, &syscon->sysswlock);
	value = readl(&syscon->devicecfg);
	value &= ~SYSCON_DEVICECFG_SWRST;
	writel(value, &syscon->devicecfg);

	/* Dying... */
	while (1)
		; /* noop */
}
