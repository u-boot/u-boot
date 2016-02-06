/*
 * U-Boot - main board file
 *
 * Copyright (c) Switchfin Org. <dpn@switchfin.org>
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <net.h>
#include <netdev.h>

int checkboard(void)
{
	printf("Board: Switchvoice BR4 Appliance\n");
	printf("       Support: http://www.switchvoice.com/\n");
	return 0;
}

#ifdef CONFIG_BFIN_MAC
int board_eth_init(bd_t *bis)
{
	return bfin_EMAC_initialize(bis);
}
#endif
