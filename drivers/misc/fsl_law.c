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
#include <asm/io.h>

#define LAWAR_EN	0x80000000

void set_law(u8 idx, phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	volatile u32 *base = (volatile u32 *)(CFG_IMMR + 0xc08);
	volatile u32 *lawbar = base + 8 * idx;
	volatile u32 *lawar = base + 8 * idx + 2;

	out_be32(lawbar, addr >> 12);
	out_be32(lawar, LAWAR_EN | ((u32)id << 20) | (u32)sz);

	return ;
}

void disable_law(u8 idx)
{
	volatile u32 *base = (volatile u32 *)(CFG_IMMR + 0xc08);
	volatile u32 *lawbar = base + 8 * idx;
	volatile u32 *lawar = base + 8 * idx + 2;

	out_be32(lawar, 0);
	out_be32(lawbar, 0);

	return;
}

void init_laws(void)
{
	int i;
	u8 law_idx = 0;

	for (i = 0; i < num_law_entries; i++) {
		if (law_table[i].index != -1)
			law_idx = law_table[i].index;

		set_law(law_idx++, law_table[i].addr,
			law_table[i].size, law_table[i].trgt_id);
	}

	return ;
}
