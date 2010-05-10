/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2009 I-SYST.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: I-SYST IBF-DSP561 Micromodule\n");
	printf("       Support: http://www.i-syst.com/\n");
	return 0;
}

#ifdef CONFIG_DRIVER_AX88180
int board_eth_init(bd_t *bis)
{
	return ax88180_initialize(bis);
}
#endif
