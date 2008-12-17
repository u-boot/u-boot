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

unsigned int setup_ddr_tlbs(unsigned int memsize_in_meg)
{
	unsigned int tlb_size;
	unsigned int ram_tlb_index;
	unsigned int ram_tlb_address;

	/*
	 * Determine size of each TLB1 entry.
	 */
	switch (memsize_in_meg) {
	case 16:
	case 32:
		tlb_size = BOOKE_PAGESZ_16M;
		break;
	case 64:
	case 128:
		tlb_size = BOOKE_PAGESZ_64M;
		break;
	case 256:
	case 512:
		tlb_size = BOOKE_PAGESZ_256M;
		break;
	case 1024:
	case 2048:
		if (PVR_VER(get_pvr()) > PVR_VER(PVR_85xx))
			tlb_size = BOOKE_PAGESZ_1G;
		else
			tlb_size = BOOKE_PAGESZ_256M;
		break;
	default:
		puts("DDR: only 16M, 32M, 64M, 128M, 256M, 512M, 1G"
			" and 2G are supported.\n");

		/*
		 * The memory was not able to be mapped.
		 * Default to a small size.
		 */
		tlb_size = BOOKE_PAGESZ_64M;
		memsize_in_meg = 64;
		break;
	}

	/*
	 * Configure DDR TLB1 entries.
	 * Starting at TLB1 8, use no more than 8 TLB1 entries.
	 */
	ram_tlb_index = 8;
	ram_tlb_address = (unsigned int)CONFIG_SYS_DDR_SDRAM_BASE;
	while (ram_tlb_address < (memsize_in_meg * 1024 * 1024)
	      && ram_tlb_index < 16) {
		set_tlb(1, ram_tlb_address, ram_tlb_address,
			MAS3_SX|MAS3_SW|MAS3_SR, 0,
			0, ram_tlb_index, tlb_size, 1);

		ram_tlb_address += (0x1000 << ((tlb_size - 1) * 2));
		ram_tlb_index++;
	}

	/*
	 * Confirm that the requested amount of memory was mapped.
	 */
	return memsize_in_meg;
}
