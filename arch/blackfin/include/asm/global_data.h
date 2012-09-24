/*
 * U-boot - global_data.h Declarations for global data of u-boot
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2010
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

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/u-boot.h>

/*
 * The following data structure is placed in some memory wich is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 */
typedef struct global_data {
	bd_t *bd;
	unsigned long flags;
	unsigned long board_type;
	unsigned long baudrate;
	unsigned long have_console;	/* serial_init() was called */
#ifdef CONFIG_PRE_CONSOLE_BUFFER
	unsigned long	precon_buf_idx;	/* Pre-Console buffer index */
#endif
	phys_size_t ram_size;		/* RAM size */
	unsigned long env_addr;	/* Address  of Environment struct */
	unsigned long env_valid;	/* Checksum of Environment valid? */
#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
	unsigned long post_log_word;	/* Record POST activities */
	unsigned long post_log_res; 	/* success of POST test */
	unsigned long post_init_f_time;	/* When post_init_f started */
#endif

	void	**jt;			/* jump table */
	char	env_buf[32];		/* buffer for getenv() before reloc. */
} gd_t;

#include <asm-generic/global_data_flags.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("P3")

#endif
