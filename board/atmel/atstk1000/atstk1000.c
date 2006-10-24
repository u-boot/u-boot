/*
 * Copyright (C) 2005-2006 Atmel Corporation
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

#include <asm/io.h>
#include <asm/sdram.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct sdram_info sdram = {
	.phys_addr	= CFG_SDRAM_BASE,
	.row_bits	= 11,
	.col_bits	= 8,
	.bank_bits	= 2,
	.cas		= 3,
	.twr		= 2,
	.trc		= 7,
	.trp		= 2,
	.trcd		= 2,
	.tras		= 5,
	.txsr		= 5,
};

void board_init_memories(void)
{
	gd->sdram_size = sdram_init(&sdram);
}

void board_init_info(void)
{
	gd->bd->bi_phy_id[0] = 0x10;
	gd->bd->bi_phy_id[1] = 0x11;
}
