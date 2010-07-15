/*
 * U-boot - main board file
 *
 * Copyright (c) 2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/pll.h>

int checkboard(void)
{
	printf("Board: ADI BF527 AD7160-EVAL board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

int misc_init_r(void)
{
	/* CLKIN Buffer Output Enable */
	*pVR_CTL |= CLKBUFOE;
	return 0;
}
