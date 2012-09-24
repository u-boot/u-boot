/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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

/**************************************************************
 * CAUTION:
 *   - do not implement for NDS32 Arch yet.
 *   - so far no one uses the macros defined in this head file.
 **************************************************************/

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H
/*
 * The following data structure is placed in some memory wich is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 */

typedef	struct global_data {
	bd_t		*bd;
	unsigned long	flags;
	unsigned long	baudrate;
	unsigned long	have_console;	/* serial_init() was called */

	unsigned long	reloc_off;	/* Relocation Offset */
	unsigned long	env_addr;	/* Address  of Environment struct */
	unsigned long	env_valid;	/* Checksum of Environment valid? */
	unsigned long	fb_base;	/* base address of frame buffer */

	unsigned long	relocaddr;	/* Start address of U-Boot in RAM */
	phys_size_t	ram_size;	/* RAM size */
	unsigned long	mon_len;	/* monitor len */
	unsigned long	irq_sp;		/* irq stack pointer */
	unsigned long	start_addr_sp;	/* start_addr_stackpointer */
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	unsigned long	tlb_addr;
#endif

	void		**jt;		/* jump table */
	char		env_buf[32];	/* buffer for getenv() before reloc. */
} gd_t;

#include <asm-generic/global_data_flags.h>

#ifdef CONFIG_GLOBAL_DATA_NOT_REG10
extern volatile gd_t g_gd;
#define DECLARE_GLOBAL_DATA_PTR		static volatile gd_t *gd = &g_gd
#else
#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("$r10")
#endif

#endif /* __ASM_GBL_DATA_H */
