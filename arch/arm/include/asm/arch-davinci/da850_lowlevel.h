/*
 * SoC-specific lowlevel code for DA850
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
#ifndef __DA850_LOWLEVEL_H
#define __DA850_LOWLEVEL_H

#include <asm/arch/pinmux_defs.h>

/* pinmux_resource[] vector is defined in the board specific file */
extern const struct pinmux_resource pinmuxes[];
extern const int pinmuxes_size;

extern const struct lpsc_resource lpsc[];
extern const int lpsc_size;

/* NOR Boot Configuration Word Field Descriptions */
#define DA850_NORBOOT_COPY_XK(X)	((X - 1) << 8)
#define DA850_NORBOOT_METHOD_DIRECT	(1 << 4)
#define DA850_NORBOOT_16BIT		(1 << 0)

#define dv_maskbits(addr, val) \
	writel((readl(addr) & val), addr)

void da850_waitloop(unsigned long loopcnt);
int da850_pll_init(struct davinci_pllc_regs *reg, unsigned long pllmult);
void da850_lpc_transition(unsigned char pscnum, unsigned char module,
		unsigned char domain, unsigned char state);
int da850_ddr_setup(void);
void da850_psc_init(void);
void da850_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value);

#endif /* #ifndef __DA850_LOWLEVEL_H */
