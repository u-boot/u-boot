/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * Portions of this file are derived from the Linux kernel source
 *  Copyright (C) 1991, 1992  Linus Torvalds
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __X86_CONTROL_REGS_H
#define __X86_CONTROL_REGS_H

/*
 * The memory clobber prevents the GCC from reordering the read/write order
 * of CR0
*/
static inline unsigned long read_cr0(void)
{
	unsigned long val;

	asm volatile ("movl %%cr0, %0" : "=r" (val) : : "memory");
	return val;
}

static inline void write_cr0(unsigned long val)
{
	asm volatile ("movl %0, %%cr0" : : "r" (val) : "memory");
}

static inline unsigned long read_cr2(void)
{
	unsigned long val;

	asm volatile("mov %%cr2,%0\n\t" : "=r" (val) : : "memory");
	return val;
}

static inline unsigned long read_cr3(void)
{
	unsigned long val;

	asm volatile("mov %%cr3,%0\n\t" : "=r" (val) : : "memory");
	return val;
}

static inline unsigned long read_cr4(void)
{
	unsigned long val;

	asm volatile("mov %%cr4,%0\n\t" : "=r" (val) : : "memory");
	return val;
}

static inline unsigned long get_debugreg(int regno)
{
	unsigned long val = 0;  /* Damn you, gcc! */

	switch (regno) {
	case 0:
		asm("mov %%db0, %0" : "=r" (val));
		break;
	case 1:
		asm("mov %%db1, %0" : "=r" (val));
		break;
	case 2:
		asm("mov %%db2, %0" : "=r" (val));
		break;
	case 3:
		asm("mov %%db3, %0" : "=r" (val));
		break;
	case 6:
		asm("mov %%db6, %0" : "=r" (val));
		break;
	case 7:
		asm("mov %%db7, %0" : "=r" (val));
		break;
	default:
		val = 0;
	}
	return val;
}

#endif
