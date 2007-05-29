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

#include "asm/types.h"

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
#if defined(CONFIG_CPM2)
	/* There are many clocks on the MPC8260 - see page 9-5 */
	unsigned long	vco_out;
	unsigned long	cpm_clk;
	unsigned long	scc_clk;
	unsigned long	brg_clk;
#endif
#if defined(CONFIG_MPC7448HPC2)
	unsigned long   mem_clk;
#endif
#if defined(CONFIG_MPC83XX)
	/* There are other clocks in the MPC83XX */
	u32 csb_clk;
#if defined (CONFIG_MPC834X) || defined(CONFIG_MPC831X)
	u32 tsec1_clk;
	u32 tsec2_clk;
	u32 usbdr_clk;
#endif
#if defined (CONFIG_MPC834X)
	u32 usbmph_clk;
#endif /* CONFIG_MPC834X */
	u32 core_clk;
	u32 i2c1_clk;
	u32 i2c2_clk;
	u32 enc_clk;
	u32 lbiu_clk;
	u32 lclk_clk;
	u32 ddr_clk;
	u32 pci_clk;
#if defined(CONFIG_QE)
	u32 qe_clk;
	u32 brg_clk;
	uint mp_alloc_base;
	uint mp_alloc_top;
#endif /* CONFIG_QE */
#if defined (CONFIG_MPC8360)
	u32  ddr_sec_clk;
#endif /* CONFIG_MPC8360 */
#endif
#if defined(CONFIG_MPC5xxx)
	unsigned long	ipb_clk;
	unsigned long	pci_clk;
#endif
#if defined(CONFIG_MPC8220)
	unsigned long   bExtUart;
	unsigned long   inp_clk;
	unsigned long   pci_clk;
	unsigned long   vco_clk;
	unsigned long   pev_clk;
	unsigned long   flb_clk;
#endif
	unsigned long	ram_size;	/* RAM size */
	unsigned long	reloc_off;	/* Relocation Offset */
	unsigned long	reset_status;	/* reset status register at boot	*/
	unsigned long	env_addr;	/* Address  of Environment struct	*/
	unsigned long	env_valid;	/* Checksum of Environment valid?	*/
	unsigned long	have_console;	/* serial_init() was called		*/
#if defined(CFG_ALLOC_DPRAM) || defined(CONFIG_CPM2)
	unsigned int	dp_alloc_base;
	unsigned int	dp_alloc_top;
#endif
#if defined(CFG_GT_6426x)
	unsigned int	mirror_hack[16];
#endif
#if defined(CONFIG_A3000)	|| \
    defined(CONFIG_HIDDEN_DRAGON)  || \
    defined(CONFIG_MUSENKI)	||  \
    defined(CONFIG_SANDPOINT)
	void *		console_addr;
#endif
#ifdef CONFIG_AMIGAONEG3SE
	unsigned long	relocaddr;	/* Start address of U-Boot in RAM */
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	unsigned long	fb_base;	/* Base address of framebuffer memory	*/
#endif
#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
	unsigned long	post_log_word;  /* Record POST activities */
	unsigned long	post_init_f_time;  /* When post_init_f started */
#endif
#ifdef CONFIG_BOARD_TYPES
	unsigned long	board_type;
#endif
#ifdef CONFIG_MODEM_SUPPORT
	unsigned long do_mdm_init;
	unsigned long be_quiet;
#endif
#ifdef CONFIG_LWMON
	unsigned long kbd_status;
#endif
	void		**jt;		/* jump table */
} gd_t;

/*
 * Global Data Flags
 */
#define	GD_FLG_RELOC	0x00001		/* Code was relocated to RAM		*/
#define	GD_FLG_DEVINIT	0x00002		/* Devices have been initialized	*/
#define	GD_FLG_SILENT	0x00004		/* Silent mode				*/

#if 1
#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("r29")
#else /* We could use plain global data, but the resulting code is bigger */
#define XTRN_DECLARE_GLOBAL_DATA_PTR	extern
#define DECLARE_GLOBAL_DATA_PTR     XTRN_DECLARE_GLOBAL_DATA_PTR \
				    gd_t *gd
#endif

#endif /* __ASM_GBL_DATA_H */
