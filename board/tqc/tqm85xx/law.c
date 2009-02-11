/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
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

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

/*
 * LAW(Local Access Window) configuration:
 *
 * Standard mapping:
 *
 * 0x0000_0000	   0x7fff_ffff	   DDR			   2G
 * 0x8000_0000	   0x9fff_ffff	   PCI1 MEM		   512M
 * 0xc000_0000	   0xdfff_ffff	   RapidIO or PCI express  512M
 * 0xe000_0000	   0xe000_ffff	   CCSR			   1M
 * 0xe200_0000	   0xe2ff_ffff	   PCI1 IO		   16M
 * 0xe300_0000	   0xe3ff_ffff	   CAN and NAND Flash	   16M
 * 0xef00_0000	   0xefff_ffff     PCI express IO          16M
 * 0xfc00_0000	   0xffff_ffff	   FLASH (boot bank)	   128M
 *
 * Big FLASH mapping:
 *
 * 0x0000_0000	   0x7fff_ffff	   DDR			   2G
 * 0x8000_0000	   0x9fff_ffff	   PCI1 MEM		   512M
 * 0xa000_0000	   0xa000_ffff	   CCSR			   1M
 * 0xa200_0000	   0xa2ff_ffff	   PCI1 IO		   16M
 * 0xa300_0000	   0xa3ff_ffff	   CAN and NAND Flash	   16M
 * 0xaf00_0000	   0xafff_ffff     PCI express IO          16M
 * 0xb000_0000	   0xbfff_ffff	   RapidIO or PCI express  256M
 * 0xc000_0000	   0xffff_ffff	   FLASH (boot bank)	   1G
 *
 * Notes:
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 */

#ifdef CONFIG_TQM_BIGFLASH
#define LAW_3_SIZE LAW_SIZE_1G
#define LAW_5_SIZE LAW_SIZE_256M
#else
#define LAW_3_SIZE LAW_SIZE_128M
#define LAW_5_SIZE LAW_SIZE_512M
#endif

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE, LAW_SIZE_2G, LAW_TRGT_IF_DDR),
	SET_LAW(CONFIG_SYS_PCI1_MEM_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_PCI),
	SET_LAW(CONFIG_SYS_LBC_FLASH_BASE, LAW_3_SIZE, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_PCI1_IO_PHYS, LAW_SIZE_16M, LAW_TRGT_IF_PCI),
#ifdef CONFIG_PCIE1
	SET_LAW(CONFIG_SYS_PCIE1_MEM_BASE, LAW_5_SIZE, LAW_TRGT_IF_PCIE_1),
#else /* !CONFIG_PCIE1 */
	SET_LAW(CONFIG_SYS_RIO_MEM_BASE, LAW_5_SIZE, LAW_TRGT_IF_RIO),
#endif /* CONFIG_PCIE1 */
#if defined(CONFIG_CAN_DRIVER) || defined(CONFIG_NAND)
	SET_LAW(CONFIG_SYS_CAN_BASE, LAW_SIZE_16M, LAW_TRGT_IF_LBC),
#endif /* CONFIG_CAN_DRIVER || CONFIG_NAND */
#ifdef CONFIG_PCIE1
	SET_LAW(CONFIG_SYS_PCIE1_IO_BASE, LAW_SIZE_16M, LAW_TRGT_IF_PCIE_1),
#endif /* CONFIG_PCIE */
};

int num_law_entries = ARRAY_SIZE (law_table);
