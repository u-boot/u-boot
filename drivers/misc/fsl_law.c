/*
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
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

/* number of LAWs in the hw implementation */
#if defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
    defined(CONFIG_MPC8560) || defined(CONFIG_MPC8555)
#define FSL_HW_NUM_LAWS 8
#elif defined(CONFIG_MPC8548) || defined(CONFIG_MPC8544) || \
      defined(CONFIG_MPC8568) || defined(CONFIG_MPC8569) || \
      defined(CONFIG_MPC8641) || defined(CONFIG_MPC8610)
#define FSL_HW_NUM_LAWS 10
#elif defined(CONFIG_MPC8536) || defined(CONFIG_MPC8572) || \
      defined(CONFIG_P1011) || defined(CONFIG_P1020) || \
      defined(CONFIG_P1012) || defined(CONFIG_P1021) || \
      defined(CONFIG_P1013) || defined(CONFIG_P1022) || \
      defined(CONFIG_P2010) || defined(CONFIG_P2020)
#define FSL_HW_NUM_LAWS 12
#elif defined(CONFIG_PPC_P4080)
#define FSL_HW_NUM_LAWS 32
#else
#error FSL_HW_NUM_LAWS not defined for this platform
#endif

#ifdef CONFIG_FSL_CORENET
void set_law(u8 idx, phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	volatile ccsr_local_t *ccm = (void *)(CONFIG_SYS_FSL_CORENET_CCM_ADDR);

	gd->used_laws |= (1 << idx);

	out_be32(&ccm->law[idx].lawar, 0);
	out_be32(&ccm->law[idx].lawbarh, ((u64)addr >> 32));
	out_be32(&ccm->law[idx].lawbarl, addr & 0xffffffff);
	out_be32(&ccm->law[idx].lawar, LAW_EN | ((u32)id << 20) | (u32)sz);

	/* Read back so that we sync the writes */
	in_be32(&ccm->law[idx].lawar);
}

void disable_law(u8 idx)
{
	volatile ccsr_local_t *ccm = (void *)(CONFIG_SYS_FSL_CORENET_CCM_ADDR);

	gd->used_laws &= ~(1 << idx);

	out_be32(&ccm->law[idx].lawar, 0);
	out_be32(&ccm->law[idx].lawbarh, 0);
	out_be32(&ccm->law[idx].lawbarl, 0);

	/* Read back so that we sync the writes */
	in_be32(&ccm->law[idx].lawar);

	return;
}

#ifndef CONFIG_NAND_SPL
static int get_law_entry(u8 i, struct law_entry *e)
{
	volatile ccsr_local_t *ccm = (void *)(CONFIG_SYS_FSL_CORENET_CCM_ADDR);
	u32 lawar;

	lawar = in_be32(&ccm->law[i].lawar);

	if (!(lawar & LAW_EN))
		return 0;

	e->addr = ((u64)in_be32(&ccm->law[i].lawbarh) << 32) |
			in_be32(&ccm->law[i].lawbarl);
	e->size = lawar & 0x3f;
	e->trgt_id = (lawar >> 20) & 0xff;

	return 1;
}
#endif
#else
void set_law(u8 idx, phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	volatile u32 *base = (volatile u32 *)(CONFIG_SYS_IMMR + 0xc08);
	volatile u32 *lawbar = base + 8 * idx;
	volatile u32 *lawar = base + 8 * idx + 2;

	gd->used_laws |= (1 << idx);

	out_be32(lawar, 0);
	out_be32(lawbar, addr >> 12);
	out_be32(lawar, LAW_EN | ((u32)id << 20) | (u32)sz);

	/* Read back so that we sync the writes */
	in_be32(lawar);
}

void disable_law(u8 idx)
{
	volatile u32 *base = (volatile u32 *)(CONFIG_SYS_IMMR + 0xc08);
	volatile u32 *lawbar = base + 8 * idx;
	volatile u32 *lawar = base + 8 * idx + 2;

	gd->used_laws &= ~(1 << idx);

	out_be32(lawar, 0);
	out_be32(lawbar, 0);

	/* Read back so that we sync the writes */
	in_be32(lawar);

	return;
}

#ifndef CONFIG_NAND_SPL
static int get_law_entry(u8 i, struct law_entry *e)
{
	volatile u32 *base = (volatile u32 *)(CONFIG_SYS_IMMR + 0xc08);
	volatile u32 *lawbar = base + 8 * i;
	volatile u32 *lawar = base + 8 * i + 2;
	u32 temp;

	temp = in_be32(lawar);

	if (!(temp & LAW_EN))
		return 0;

	e->addr = (u64)in_be32(lawbar) << 12;
	e->size = temp & 0x3f;
	e->trgt_id = (temp >> 20) & 0xff;

	return 1;
}
#endif
#endif

int set_next_law(phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	u32 idx = ffz(gd->used_laws);

	if (idx >= FSL_HW_NUM_LAWS)
		return -1;

	set_law(idx, addr, sz, id);

	return idx;
}

#ifndef CONFIG_NAND_SPL
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

struct law_entry find_law(phys_addr_t addr)
{
	struct law_entry entry;
	int i;

	entry.index = -1;
	entry.addr = 0;
	entry.size = 0;
	entry.trgt_id = 0;

	for (i = 0; i < FSL_HW_NUM_LAWS; i++) {
		u64 upper;

		if (!get_law_entry(i, &entry))
			continue;

		upper = entry.addr + (2ull << entry.size);
		if ((addr >= entry.addr) && (addr < upper)) {
			entry.index = i;
			break;
		}
	}

	return entry;
}

void print_laws(void)
{
	volatile u32 *base = (volatile u32 *)(CONFIG_SYS_IMMR + 0xc08);
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

/* use up to 2 LAWs for DDR, used the last available LAWs */
int set_ddr_laws(u64 start, u64 sz, enum law_trgt_if id)
{
	u64 start_align, law_sz;
	int law_sz_enc;

	if (start == 0)
		start_align = 1ull << (LAW_SIZE_32G + 1);
	else
		start_align = 1ull << (ffs64(start) - 1);
	law_sz = min(start_align, sz);
	law_sz_enc = __ilog2_u64(law_sz) - 1;

	if (set_last_law(start, law_sz_enc, id) < 0)
		return -1;

	/* recalculate size based on what was actually covered by the law */
	law_sz = 1ull << __ilog2_u64(law_sz);

	/* do we still have anything to map */
	sz = sz - law_sz;
	if (sz) {
		start += law_sz;

		start_align = 1ull << (ffs64(start) - 1);
		law_sz = min(start_align, sz);
		law_sz_enc = __ilog2_u64(law_sz) - 1;

		if (set_last_law(start, law_sz_enc, id) < 0)
			return -1;
	} else {
		return 0;
	}

	/* do we still have anything to map */
	sz = sz - law_sz;
	if (sz)
		return 1;

	return 0;
}
#endif

void init_laws(void)
{
	int i;

#if FSL_HW_NUM_LAWS < 32
	gd->used_laws = ~((1 << FSL_HW_NUM_LAWS) - 1);
#elif FSL_HW_NUM_LAWS == 32
	gd->used_laws = 0;
#else
#error FSL_HW_NUM_LAWS can not be greater than 32 w/o code changes
#endif

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
