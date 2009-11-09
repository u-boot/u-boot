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
 * LAW (Local Access Window) configuration:
 *
 * 0x0000_0000	DDR			256M
 * 0x1000_0000	DDR2			256M
 * 0x8000_0000	PCI1 MEM		512M
 * 0xa000_0000	PCI2 MEM		512M
 * 0xc000_0000	RapidIO			512M
 * 0xe200_0000	PCI1 IO			16M
 * 0xe300_0000	PCI2 IO			16M
 * 0xf800_0000	CCSRBAR			2M
 * 0xfe00_0000	FLASH (boot bank)	32M
 *
 */


struct law_entry law_table[] = {
#if !defined(CONFIG_SPD_EEPROM)
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_DDR_1),
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000,
		 LAW_SIZE_256M, LAW_TRGT_IF_DDR_2),
#endif
	SET_LAW(CONFIG_SYS_PCI1_MEM_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_PCI_1),
	SET_LAW(CONFIG_SYS_PCI2_MEM_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_PCI_2),
	SET_LAW(0xf8000000, LAW_SIZE_2M, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_PCI1_IO_PHYS, LAW_SIZE_16M, LAW_TRGT_IF_PCI_1),
	SET_LAW(CONFIG_SYS_PCI2_IO_PHYS, LAW_SIZE_16M, LAW_TRGT_IF_PCI_2),
	SET_LAW(0xfe000000, LAW_SIZE_32M, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_RIO_MEM_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_RIO)
};

int num_law_entries = ARRAY_SIZE(law_table);
