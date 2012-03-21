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
 * 0x0000_0000     0x1fff_ffff     DDR                     SYS_SDRAM_SIZE
 * 0xc000_0000     0xdfff_ffff     RapidIO (set elsewhere) 512M
 * 0xe000_0000     0xe000_ffff     CCSR    (set elsewhere) 1M
 * 0xf000_0000     0xffff_ffff     LBC options + FLASH     256M
 *
 * Notes:
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 *
 * LAW 0 is reserved for boot mapping
 */

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_SIZE_LOG - 1,
		LAW_TRGT_IF_DDR_1),
	SET_LAW(CONFIG_SYS_LBC_OPTION_BASE_PHYS, LAW_SIZE_256M,
		LAW_TRGT_IF_LBC)
};

int num_law_entries = ARRAY_SIZE(law_table);
