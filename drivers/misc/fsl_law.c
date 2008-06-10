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

DECLARE_GLOBAL_DATA_PTR;

#define LAWAR_EN	0x80000000
/* number of LAWs in the hw implementation */
#if defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
    defined(CONFIG_MPC8560) || defined(CONFIG_MPC8555)
#define FSL_HW_NUM_LAWS 8
#elif defined(CONFIG_MPC8548) || defined(CONFIG_MPC8544) || \
      defined(CONFIG_MPC8568) || \
      defined(CONFIG_MPC8641) || defined(CONFIG_MPC8610)
#define FSL_HW_NUM_LAWS 10
#elif defined(CONFIG_MPC8572)
#define FSL_HW_NUM_LAWS 12
#else
#error FSL_HW_NUM_LAWS not defined for this platform
#endif

void set_law(u8 idx, phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	volatile u32 *base = (volatile u32 *)(CFG_IMMR + 0xc08);
	volatile u32 *lawbar = base + 8 * idx;
	volatile u32 *lawar = base + 8 * idx + 2;

	gd->used_laws |= (1 << idx);

	out_be32(lawbar, addr >> 12);
	out_be32(lawar, LAWAR_EN | ((u32)id << 20) | (u32)sz);

	return ;
}

int set_next_law(phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	u32 idx = ffz(gd->used_laws);

	if (idx >= FSL_HW_NUM_LAWS)
		return -1;

	set_law(idx, addr, sz, id);

	return idx;
}

int set_last_law(phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	u32 idx;

	/* we have no LAWs free */
	if (gd->used_laws == -1)
		return -1;

	/* grab the last free law */
	idx = __ilog2(~(gd->used_laws));

	if (idx >= FSL_HW_NUM_LAWS)
		return -1;

	set_law(idx, addr, sz, id);

	return idx;
}

void disable_law(u8 idx)
{
	volatile u32 *base = (volatile u32 *)(CFG_IMMR + 0xc08);
	volatile u32 *lawbar = base + 8 * idx;
	volatile u32 *lawar = base + 8 * idx + 2;

	gd->used_laws &= ~(1 << idx);

	out_be32(lawar, 0);
	out_be32(lawbar, 0);

	return;
}

void print_laws(void)
{
	volatile u32 *base = (volatile u32 *)(CFG_IMMR + 0xc08);
	volatile u32 *lawbar = base;
	volatile u32 *lawar = base + 2;
	int i;

	printf("\nLocal Access Window Configuration\n");
	for(i = 0; i < FSL_HW_NUM_LAWS; i++) {
		printf("\tLAWBAR%d : 0x%08x, LAWAR%d : 0x%08x\n",
		       i, in_be32(lawbar), i, in_be32(lawar));
		lawbar += 8;
		lawar += 8;
	}

	return;
}

void init_laws(void)
{
	int i;

	gd->used_laws = ~((1 << FSL_HW_NUM_LAWS) - 1);

	for (i = 0; i < num_law_entries; i++) {
		if (law_table[i].index == -1)
			set_next_law(law_table[i].addr, law_table[i].size,
					law_table[i].trgt_id);
		else
			set_law(law_table[i].index, law_table[i].addr,
				law_table[i].size, law_table[i].trgt_id);
	}

	return ;
}
