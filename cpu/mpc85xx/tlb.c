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
#include <asm/processor.h>
#include <asm/mmu.h>

void set_tlb(u8 tlb, u32 epn, u64 rpn,
	     u8 perms, u8 wimge,
	     u8 ts, u8 esel, u8 tsize, u8 iprot)
{
	u32 _mas0, _mas1, _mas2, _mas3, _mas7;

	_mas0 = FSL_BOOKE_MAS0(tlb, esel, 0);
	_mas1 = FSL_BOOKE_MAS1(1, iprot, 0, ts, tsize);
	_mas2 = FSL_BOOKE_MAS2(epn, wimge);
	_mas3 = FSL_BOOKE_MAS3(rpn, 0, perms);
	_mas7 = rpn >> 32;

	mtspr(MAS0, _mas0);
	mtspr(MAS1, _mas1);
	mtspr(MAS2, _mas2);
	mtspr(MAS3, _mas3);
#ifdef CONFIG_ENABLE_36BIT_PHYS
	mtspr(MAS7, _mas7);
#endif
	asm volatile("isync;msync;tlbwe;isync");
}

void disable_tlb(u8 esel)
{
	u32 _mas0, _mas1, _mas2, _mas3, _mas7;

	_mas0 = FSL_BOOKE_MAS0(1, esel, 0);
	_mas1 = 0;
	_mas2 = 0;
	_mas3 = 0;
	_mas7 = 0;

	mtspr(MAS0, _mas0);
	mtspr(MAS1, _mas1);
	mtspr(MAS2, _mas2);
	mtspr(MAS3, _mas3);
#ifdef CONFIG_ENABLE_36BIT_PHYS
	mtspr(MAS7, _mas7);
#endif
	asm volatile("isync;msync;tlbwe;isync");
}

void invalidate_tlb(u8 tlb)
{
	if (tlb == 0)
		mtspr(MMUCSR0, 0x4);
	if (tlb == 1)
		mtspr(MMUCSR0, 0x2);
}

void init_tlbs(void)
{
	int i;

	for (i = 0; i < num_tlb_entries; i++) {
		set_tlb(tlb_table[i].tlb, tlb_table[i].epn, tlb_table[i].rpn,
			tlb_table[i].perms, tlb_table[i].wimge,
			tlb_table[i].ts, tlb_table[i].esel, tlb_table[i].tsize,
			tlb_table[i].iprot);
	}

	return ;
}
