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

void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
			  struct sdram_base_params *base)
{
	u32 bg;

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
}
