/*
 * U-boot - stamp.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __STAMP_H__
#define __STAMP_H__

extern void init_Flags(void);

extern volatile unsigned long *ambctl0;
extern volatile unsigned long *ambctl1;
extern volatile unsigned long *amgctl;

extern unsigned long pll_div_fact;
extern void serial_setbrg(void);

/* Definitions used in  Compact Flash Boot support */
#define FIO_EDGE_CF_BITS	0x0000
#define FIO_POLAR_CF_BITS	0x0000
#define	FIO_EDGE_BITS		0x1E0
#define	FIO_POLAR_BITS		0x160

/* Compact flash status bits in status register */
#define CF_STAT_BITS		0x00000060

/* CF Flags used to switch between expansion and external
 * memory banks
 */
#define CF_PF0			0x0001
#define CF_PF1			0x0002
#define CF_PF1_PF0		0x0003

#endif
