/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <dwc3-uboot.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = 0x80000000;
	return 0;
}

int dram_init_banksize(void)
{
	/* Reserve 0x200000 for ATF bl31 */
	gd->bd->bi_dram[0].start = 0x200000;
	gd->bd->bi_dram[0].size = 0x7e000000;

	return 0;
}

int usb_gadget_handle_interrupts(void)
{
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}
