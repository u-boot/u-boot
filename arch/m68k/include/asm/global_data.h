/*
 * (C) Copyright 2002 - 2010
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H
/*
 * The following data structure is placed in some memory wich is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 */

typedef	struct	global_data {
	bd_t		*bd;
	unsigned long	flags;
	unsigned int	baudrate;
	unsigned long	cpu_clk;	/* CPU clock in Hz!		*/
	unsigned long	bus_clk;
#ifdef CONFIG_PCI
	unsigned long	pci_clk;
#endif
#ifdef CONFIG_EXTRA_CLOCK
	unsigned long	inp_clk;
	unsigned long	vco_clk;
	unsigned long	flb_clk;
#endif
#ifdef CONFIG_FSL_I2C
	unsigned long	i2c1_clk;
	unsigned long	i2c2_clk;
#endif
	phys_size_t	ram_size;	/* RAM size */
	unsigned long	reloc_off;	/* Relocation Offset */
	unsigned long	reset_status;	/* reset status register at boot	*/
	unsigned long	env_addr;	/* Address  of Environment struct	*/
	unsigned long	env_valid;	/* Checksum of Environment valid?	*/
	unsigned long	have_console;	/* serial_init() was called		*/
#ifdef CONFIG_PRE_CONSOLE_BUFFER
	unsigned long	precon_buf_idx;	/* Pre-Console buffer index */
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	unsigned long	fb_base;	/* Base addr of framebuffer memory */
#endif
#ifdef CONFIG_BOARD_TYPES
	unsigned long	board_type;
#endif
	void		**jt;		/* Standalone app jump table */
	char		env_buf[32];	/* buffer for getenv() before reloc. */
} gd_t;

#include <asm-generic/global_data_flags.h>

#if 0
extern gd_t *global_data;
#define DECLARE_GLOBAL_DATA_PTR     gd_t *gd = global_data
#else
#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("d7")
#endif

#endif /* __ASM_GBL_DATA_H */
