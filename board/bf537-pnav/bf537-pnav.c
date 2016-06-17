/*
 * U-Boot - main board file
 *
 * Copyright (c) 2005-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <netdev.h>
#include <net.h>
#include <asm/blackfin.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF537 PNAV board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

#ifdef CONFIG_BFIN_MAC
int board_eth_init(bd_t *bis)
{
	return bfin_EMAC_initialize(bis);
}
#endif
