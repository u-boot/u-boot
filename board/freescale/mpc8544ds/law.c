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

struct law_entry law_table[] = {
	SET_LAW_ENTRY(2, CFG_PCI1_MEM_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_PCI),
	SET_LAW_ENTRY(3, CFG_PCI1_IO_PHYS, LAW_SIZE_64K, LAW_TRGT_IF_PCI),
	SET_LAW_ENTRY(4, CFG_LBC_CACHE_BASE, LAWAR_SIZE_256M, LAW_TRGT_IF_LBC),
	SET_LAW_ENTRY(5, CFG_PCIE1_MEM_PHYS, LAWAR_SIZE_256M, LAW_TRGT_IF_PCIE_1),
	SET_LAW_ENTRY(6, CFG_PCIE1_IO_PHYS, LAW_SIZE_64K, LAW_TRGT_IF_PCIE_1),
	SET_LAW_ENTRY(7, CFG_PCIE2_MEM_PHYS, LAWAR_SIZE_512M, LAW_TRGT_IF_PCIE_2),
	SET_LAW_ENTRY(8, CFG_PCIE2_IO_PHYS, LAW_SIZE_64K, LAW_TRGT_IF_PCIE_2),
	/* contains both PCIE3 MEM & IO space */
	SET_LAW_ENTRY(9, CFG_PCIE3_MEM_PHYS, LAW_SIZE_4M, LAW_TRGT_IF_PCIE_3),
};

int num_law_entries = ARRAY_SIZE(law_table);
