/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.aribaud@free.fr>
 *
 * Based on original Kirkwood support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <config.h>
#include <asm/arch/orion5x.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * orion5x_sdram_bar - reads SDRAM Base Address Register
 */
u32 orion5x_sdram_bar(enum memory_bank bank)
{
	struct orion5x_ddr_addr_decode_registers *winregs =
		(struct orion5x_ddr_addr_decode_registers *)
		ORION5X_CPU_WIN_BASE;

	u32 result = 0;
	u32 enable = 0x01 & winregs[bank].size;

	if ((!enable) || (bank > BANK3))
		return 0;

	result = winregs[bank].base;
	return result;
}

int dram_init(void)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = orion5x_sdram_bar(i);
		gd->bd->bi_dram[i].size = get_ram_size(
			(volatile long *) (gd->bd->bi_dram[i].start),
			CONFIG_MAX_RAM_BANK_SIZE);
	}
	return 0;
}
