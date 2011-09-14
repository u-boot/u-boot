/*
 * SoC-specific lowlevel code for AM1808 and similar chips
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
#ifndef __AM1808_LOWLEVEL_H
#define __AM1808_LOWLEVEL_H

/* NOR Boot Configuration Word Field Descriptions */
#define AM1808_NORBOOT_COPY_XK(X)	((X - 1) << 8)
#define AM1808_NORBOOT_METHOD_DIRECT	(1 << 4)
#define AM1808_NORBOOT_16BIT		(1 << 0)

#define dv_maskbits(addr, val) \
	writel((readl(addr) & val), addr)

void am1808_waitloop(unsigned long loopcnt);
int am1808_pll_init(struct davinci_pllc_regs *reg, unsigned long pllmult);
void am1808_lpc_transition(unsigned char pscnum, unsigned char module,
		unsigned char domain, unsigned char state);
int am1808_ddr_setup(unsigned int freq);
void am1808_psc_init(void);
void am1808_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value);

#endif /* #ifndef __AM1808_LOWLEVEL_H */
