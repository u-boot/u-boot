/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
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
 * 0x0000_0000	   0x2fff_ffff	   DDR			   512M
 * 0x8000_0000	   0x9fff_ffff	   PCI1 MEM		   512M
 * 0xc000_0000	   0xc00f_ffff	   FPGA			   1M
 * 0xc800_0000	   0xcbff_ffff	   LIME			   64M
 * 0xe000_0000	   0xe00f_ffff	   CCSR			   1M (mapped by CCSRBAR)
 * 0xe200_0000	   0xe2ff_ffff	   PCI1 IO		   16M
 * 0xfc00_0000	   0xffff_ffff	   FLASH		   64M
 *
 * Notes:
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 */

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE, LAW_SIZE_512M, LAW_TRGT_IF_DDR),
	SET_LAW(CONFIG_SYS_PCI1_MEM_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_PCI),
	SET_LAW(CONFIG_SYS_LBC_FLASH_BASE, LAW_SIZE_64M, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_PCI1_IO_PHYS, LAW_SIZE_16M, LAW_TRGT_IF_PCI),
#if defined(CONFIG_SYS_FPGA_BASE)
	SET_LAW(CONFIG_SYS_FPGA_BASE, LAW_SIZE_1M, LAW_TRGT_IF_LBC),
#endif
	SET_LAW(CONFIG_SYS_LIME_BASE, LAW_SIZE_64M, LAW_TRGT_IF_LBC),
};

int num_law_entries = ARRAY_SIZE(law_table);
