// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */
#include <config.h>
#include <common.h>
#include <asm/arch/stm32.h>

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

int board_late_init(void)
{
	return 0;
}

/* board dependent setup after realloc */
int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = STM32_DDR_BASE + 0x100;

	return 0;
}
