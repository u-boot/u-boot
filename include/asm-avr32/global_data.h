/*
 * Copyright (C) 2004-2006 Atmel Corporation
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
#ifndef __ASM_GLOBAL_DATA_H__
#define __ASM_GLOBAL_DATA_H__

/*
 * The following data structure is placed in some memory wich is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 *
 * Keep it *SMALL* and remember to set CFG_GBL_DATA_SIZE > sizeof(gd_t)
 */

typedef	struct	global_data {
	bd_t		*bd;
	unsigned long	flags;
	unsigned long	baudrate;
	unsigned long	stack_end;	/* highest stack address */
	unsigned long	have_console;	/* serial_init() was called */
	unsigned long	reloc_off;	/* Relocation Offset */
	unsigned long	env_addr;	/* Address of env struct */
	unsigned long	env_valid;	/* Checksum of env valid? */
	unsigned long	cpu_hz;		/* cpu core clock frequency */
	void		**jt;		/* jump table */
} gd_t;

/*
 * Global Data Flags
 */
#define GD_FLG_RELOC	0x00001		/* Code was relocated to RAM	 */
#define GD_FLG_DEVINIT	0x00002		/* Devices have been initialized */
#define GD_FLG_SILENT	0x00004		/* Silent mode			 */
#define GD_FLG_POSTFAIL	0x00008		/* Critical POST test failed	 */

#define DECLARE_GLOBAL_DATA_PTR register gd_t *gd asm("r5")

#endif /* __ASM_GLOBAL_DATA_H__ */
