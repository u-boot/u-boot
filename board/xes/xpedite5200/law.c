/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
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
 * Notes:
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 */

struct law_entry law_table[] = {
	/* LBC window - maps 256M 0xf0000000 -> 0xffffffff */
	SET_LAW(CONFIG_SYS_FLASH_BASE2, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_NAND_BASE, LAW_SIZE_1M, LAW_TRGT_IF_LBC),
#if CONFIG_SYS_PCI1_MEM_PHYS
	SET_LAW(CONFIG_SYS_PCI1_MEM_PHYS, LAW_SIZE_1G, LAW_TRGT_IF_PCI_1),
	SET_LAW(CONFIG_SYS_PCI1_IO_PHYS, LAW_SIZE_8M, LAW_TRGT_IF_PCI_1),
#endif
#if CONFIG_SYS_PCI2_MEM_PHYS
	SET_LAW(CONFIG_SYS_PCI2_MEM_PHYS, LAW_SIZE_256M, LAW_TRGT_IF_PCI_2),
	SET_LAW(CONFIG_SYS_PCI2_IO_PHYS, LAW_SIZE_8M, LAW_TRGT_IF_PCI_2),
#endif
};

int num_law_entries = ARRAY_SIZE(law_table);
