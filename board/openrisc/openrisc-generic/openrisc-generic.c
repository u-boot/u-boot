/*
 * Based on nios2-generic.c:
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

int board_early_init_f(void)
{
	return 0;
}

int checkboard(void)
{
	printf("BOARD: %s\n", CONFIG_BOARD_NAME);
	return 0;
}

phys_size_t initdram(int board_type)
{
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_ETHOC
	rc += ethoc_initialize(0, CONFIG_SYS_ETHOC_BASE);
#endif
	return rc;
}
#endif
