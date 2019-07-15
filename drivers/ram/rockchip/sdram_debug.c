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
