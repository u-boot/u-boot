// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Mihai Sain <mihai.sain@microchip.com>
 *
 */

#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama7g5.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	// Address of boot parameters
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

	return 0;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}
