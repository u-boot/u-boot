/*
 * U-boot - main board file
 *
 * Copyright (c) 2007 David Rowe,
 *           (c) 2006 Ivan Danov
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
	printf("Board: IP04 IP-PBX\n");
	printf("       http://www.rowetel.com/ucasterisk/ip04.html\n");
	return 0;
}

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	return dm9000_initialize(bis);
}
#endif
