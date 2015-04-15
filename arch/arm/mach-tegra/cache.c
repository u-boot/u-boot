/*
 * Copyright (c) 2013-2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Tegra cache routines */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch/gp_padctrl.h>

void config_cache(void)
{
	u32 reg = 0;

	/* enable SMP mode and FW for CPU0, by writing to Auxiliary Ctl reg */
	asm volatile(
		"mrc p15, 0, r0, c1, c0, 1\n"
		"orr r0, r0, #0x41\n"
		"mcr p15, 0, r0, c1, c0, 1\n");

	/* Currently, only Tegra114+ needs this L2 cache change to boot Linux */
	if (tegra_get_chip() < CHIPID_TEGRA114)
		return;

	/*
	 * Systems with an architectural L2 cache must not use the PL310.
	 * Config L2CTLR here for a data RAM latency of 3 cycles.
	 */
	asm("mrc p15, 1, %0, c9, c0, 2" : : "r" (reg));
	reg &= ~7;
	reg |= 2;
	asm("mcr p15, 1, %0, c9, c0, 2" : : "r" (reg));
}
