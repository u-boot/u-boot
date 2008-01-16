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

/* LAW(Local Access Window) configuration:
 * 0000_0000-0800_0000: DDR(128M) -or- larger
 * f000_0000-f3ff_ffff: PCI(256M)
 * f400_0000-f7ff_ffff: RapidIO(128M)
 * f800_0000-ffff_ffff: localbus(128M)
 *   f800_0000-fbff_ffff: LBC SDRAM(64M)
 *   fc00_0000-fdef_ffff: LBC BCSR,RTC,etc(31M)
 *   fdf0_0000-fdff_ffff: CCSRBAR(1M)
 *   fe00_0000-ffff_ffff: Flash(32M)
 * Note: CCSRBAR and L2-as-SRAM don't need configure Local Access
 *       Window.
 * Note: If flash is 8M at default position(last 8M),no LAW needed.
 */

struct law_entry law_table[] = {
#ifndef CONFIG_SPD_EEPROM
	SET_LAW_ENTRY(1, CFG_DDR_SDRAM_BASE, LAW_SIZE_128M, LAW_TRGT_IF_DDR),
#endif
	SET_LAW_ENTRY(2, CFG_PCI_MEM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_PCI),
#ifndef CONFIG_RAM_AS_FLASH
	SET_LAW_ENTRY(3, CFG_LBC_SDRAM_BASE, LAW_SIZE_128M, LAW_TRGT_IF_LBC),
#endif
};

int num_law_entries = ARRAY_SIZE(law_table);
