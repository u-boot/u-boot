/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>

int checkboard(void)
{
	printf("Board: ADI BF506F EZ-Kit board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

int board_early_init_f(void)
{
	bfin_write_EBIU_MODE(1);
	SSYNC();
	bfin_write_FLASH_CONTROL_CLEAR(1);
	udelay(1);
	bfin_write_FLASH_CONTROL_SET(1);
	return 0;
}
