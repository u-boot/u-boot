/*
 * U-boot - setup.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * This file is based on
 * asm/setup.h -- Definition of the Linux/Blackfin setup information
 * Copyright Lineo, Inc 2001 Tony Kou
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

#ifndef _BLACKFIN_SETUP_H
#define _BLACKFIN_SETUP_H

#include <linux/config.h>

/*
 * Linux/Blackfin Architectures
 */

#define MACH_BFIN	1

#ifdef __KERNEL__

#ifndef __ASSEMBLY__
extern unsigned long blackfin_machtype;
#endif

#if defined(CONFIG_BFIN)
#define MACH_IS_BFIN (blackfin_machtype == MACH_BFIN)
#endif

#ifndef MACH_TYPE
#define MACH_TYPE (blackfin_machtype)
#endif

#endif

/*
 * CPU, FPU and MMU types
 *
 * Note: we don't need now:
 *
 */

#ifndef __ASSEMBLY__
extern unsigned long blackfin_cputype;
#ifdef CONFIG_VME
extern unsigned long vme_brdtype;
#endif

/*
 *  Miscellaneous
 */

#define NUM_MEMINFO	4
#define CL_SIZE		256

extern int blackfin_num_memory;	/* # of memory blocks found (and used) */
extern int blackfin_realnum_memory;	/* real # of memory blocks found */
extern struct mem_info blackfin_memory[NUM_MEMINFO];	/* memory description */

struct mem_info {
	unsigned long addr;	/* physical address of memory chunk */
	unsigned long size;	/* length of memory chunk (in bytes) */
};
#endif

#endif
