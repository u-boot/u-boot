/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _DV_AINTC_DEFS_H_
#define _DV_AINTC_DEFS_H_

struct dv_aintc_regs {
	unsigned int	fiq0;		/* 0x00 */
	unsigned int	fiq1;		/* 0x04 */
	unsigned int	irq0;		/* 0x08 */
	unsigned int	irq1;		/* 0x0c */
	unsigned int	fiqentry;	/* 0x10 */
	unsigned int	irqentry;	/* 0x14 */
	unsigned int	eint0;		/* 0x18 */
	unsigned int	eint1;		/* 0x1c */
	unsigned int	intctl;		/* 0x20 */
	unsigned int	eabase;		/* 0x24 */
	unsigned char	rsvd0[8];	/* 0x28 */
	unsigned int	intpri0;	/* 0x30 */
	unsigned int	intpri1;	/* 0x34 */
	unsigned int	intpri2;	/* 0x38 */
	unsigned int	intpri3;	/* 0x3c */
	unsigned int	intpri4;	/* 0x40 */
	unsigned int	intpri5;	/* 0x44 */
	unsigned int	intpri6;	/* 0x48 */
	unsigned int	intpri7;	/* 0x4c */
};

#define dv_aintc_regs ((struct dv_aintc_regs *)DAVINCI_ARM_INTC_BASE)

#define DV_AINTC_INTCTL_IDMODE	(1 << 2)

#endif /* _DV_AINTC_DEFS_H_ */
