/*
 * (C) Copyright 2002
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
 *
 * Keep it *SMALL* and remember to set CFG_GBL_DATA_SIZE > sizeof(gd_t)
 */

typedef	struct	global_data {
	bd_t		*bd;
	unsigned long	flags;
	unsigned long	baudrate;
	unsigned long	cpu_clk;	/* CPU clock in Hz!		*/
	unsigned long	bus_clk;
#if defined(CONFIG_8260)
	/* There are many clocks on the MPC8260 - see page 9-5 */
	unsigned long	vco_out;
	unsigned long	cpm_clk;
	unsigned long	scc_clk;
	unsigned long	brg_clk;
#endif
	unsigned long	ram_size;	/* RAM size */
	unsigned long	reloc_off;	/* Relocation Offset */
	unsigned long	reset_status;	/* reset status register at boot	*/
	unsigned long	env_addr;	/* Address  of Environment struct	*/
	unsigned long	env_valid;	/* Checksum of Environment valid?	*/
	unsigned long	have_console;	/* serial_init() was called		*/
#if defined(CFG_ALLOC_DPRAM) || defined(CONFIG_8260)
	unsigned int	dp_alloc_base;
	unsigned int	dp_alloc_top;
#endif
#if defined(CONFIG_EVB64260)
	unsigned int	mirror_hack[16];
#endif
#if defined(CONFIG_SANDPOINT) || defined(CONFIG_MUSENKI)
	void *		console_addr;
#endif
#ifdef CONFIG_AMIGAONEG3SE
	unsigned long	relocaddr;	/* Start address of U-Boot in RAM */
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	unsigned long	fb_base;	/* Base address of framebuffer memory	*/
#endif
#ifdef CONFIG_POST
	unsigned long	post_log_word;  /* Record POST activities */
#endif
#ifdef CONFIG_BOARD_TYPES
	unsigned long	board_type;
#endif
} gd_t;

/*
 * Global Data Flags
 */
#define	GD_FLG_RELOC	0x00001		/* Code was relocated to RAM		*/
#define	GD_FLG_DEVINIT	0x00002		/* Devices have been initialized	*/

#if 1
#define DECLARE_GLOBAL_DATA_PTR     register gd_t *gd asm ("r29")
#else /* We could use plain global data, but the resulting code is bigger */
#define XTRN_DECLARE_GLOBAL_DATA_PTR	extern
#define DECLARE_GLOBAL_DATA_PTR     XTRN_DECLARE_GLOBAL_DATA_PTR \
				    gd_t *gd
#endif

#endif /* __ASM_GBL_DATA_H */
