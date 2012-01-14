/*
 * SoC-specific lowlevel code for tms320dm365 and similar chips
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __DM365_LOWLEVEL_H
#define __DM365_LOWLEVEL_H

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

void dm365_waitloop(unsigned long loopcnt);
int dm365_pll1_init(unsigned long pllmult, unsigned long prediv);
int dm365_pll2_init(unsigned long pllm, unsigned long prediv);
int dm365_ddr_setup(void);
void dm365_psc_init(void);
void dm365_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value);
void dm36x_lowlevel_init(ulong bootflag);

#endif /* #ifndef __DM365_LOWLEVEL_H */
