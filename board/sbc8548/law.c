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
 * 0x0000_0000	0x0fff_ffff	DDR			256M
 * 0x8000_0000	0x9fff_ffff	PCI1 MEM		512M
 * 0xe000_0000	0xe000_ffff	CCSR			1M
 * 0xe200_0000	0xe2ff_ffff	PCI1 IO			16M
 * 0xf000_0000	0xf7ff_ffff	SDRAM			128M
 * 0xf8b0_0000	0xf80f_ffff	EEPROM			1M
 * 0xfb80_0000	0xff7f_ffff	FLASH (2nd bank)	64M
 * 0xff80_0000	0xffff_ffff	FLASH (boot bank)	8M
 *
 * Notes:
 * 	CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *	If flash is 8M at default position (last 8M), no LAW needed.
 */

struct law_entry law_table[] = {
#ifndef CONFIG_SPD_EEPROM
	SET_LAW_ENTRY(1, CFG_DDR_SDRAM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_DDR),
#endif
	SET_LAW_ENTRY(2, CFG_PCI1_MEM_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_PCI),
	SET_LAW_ENTRY(3, CFG_PCI1_IO_PHYS, LAW_SIZE_16M, LAW_TRGT_IF_PCI),
	/* LBC window - maps 256M 0xf0000000 -> 0xffffffff */
	SET_LAW_ENTRY(4, CFG_LBC_SDRAM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
};

int num_law_entries = ARRAY_SIZE(law_table);
