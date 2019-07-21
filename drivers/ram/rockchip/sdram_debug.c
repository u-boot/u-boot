// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 * (C) Copyright 2019 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <asm/arch-rockchip/sdram_common.h>

void sdram_print_dram_type(unsigned char dramtype)
{
	switch (dramtype) {
	case DDR3:
		printascii("DDR3");
		break;
	case DDR4:
		printascii("DDR4");
		break;
	case LPDDR2:
		printascii("LPDDR2");
		break;
	case LPDDR3:
		printascii("LPDDR3");
		break;
	case LPDDR4:
		printascii("LPDDR4");
		break;
	default:
		printascii("Unknown Device");
		break;
	}
}

/**
 * cs  = 0, cs0
 * cs  = 1, cs1
 * cs => 2, cs0+cs1
 * note: it didn't consider about row_3_4
 */
u64 sdram_get_cs_cap(struct sdram_cap_info *cap_info, u32 cs, u32 dram_type)
{
	u32 bg;
	u64 cap[2];

	if (dram_type == DDR4)
		/* DDR4 8bit dram BG = 2(4bank groups),
		 * 16bit dram BG = 1 (2 bank groups)
		 */
		bg = (cap_info->dbw == 0) ? 2 : 1;
	else
		bg = 0;

	cap[0] = 1llu << (cap_info->bw + cap_info->col +
		 bg + cap_info->bk + cap_info->cs0_row);

	if (cap_info->rank == 2)
		cap[1] = 1llu << (cap_info->bw + cap_info->col +
			 bg + cap_info->bk + cap_info->cs1_row);
	else
		cap[1] = 0;

	if (cs == 0)
		return cap[0];
	else if (cs == 1)
		return cap[1];
	else
		return (cap[0] + cap[1]);
}

void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
			  struct sdram_base_params *base)
{
	u32 bg, cap;

	bg = (cap_info->dbw == 0) ? 2 : 1;

	sdram_print_dram_type(base->dramtype);

	printascii(", ");
	printdec(base->ddr_freq);
	printascii("MHz\n");

	printascii("BW=");
	printdec(8 << cap_info->bw);

	printascii(" Col=");
	printdec(cap_info->col);

	printascii(" Bk=");
	printdec(0x1 << cap_info->bk);
	if (base->dramtype == DDR4) {
		printascii(" BG=");
		printdec(1 << bg);
	}

	printascii(" CS0 Row=");
	printdec(cap_info->cs0_row);
	if (cap_info->rank > 1) {
		printascii(" CS1 Row=");
		printdec(cap_info->cs1_row);
	}

	printascii(" CS=");
	printdec(cap_info->rank);

	printascii(" Die BW=");
	printdec(8 << cap_info->dbw);

	cap = sdram_get_cs_cap(cap_info, 3, base->dramtype);
	if (cap_info->row_3_4)
		cap = cap * 3 / 4;

	printascii(" Size=");
	printdec(cap >> 20);
	printascii("MB\n");
}

void sdram_print_stride(unsigned int stride)
{
	switch (stride) {
	case 0xc:
		printf("128B stride\n");
		break;
	case 5:
	case 9:
	case 0xd:
	case 0x11:
	case 0x19:
		printf("256B stride\n");
		break;
	case 0xa:
	case 0xe:
	case 0x12:
		printf("512B stride\n");
		break;
	case 0xf:
		printf("4K stride\n");
		break;
	case 0x1f:
		printf("32MB + 256B stride\n");
		break;
	default:
		printf("no stride\n");
	}
}
