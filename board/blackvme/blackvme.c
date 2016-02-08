/* U-Boot - blackvme.c  board specific routines
 * (c) Wojtek Skulski 2010 info@skutek.com
 * Board info: http://www.skutek.com
 * Copyright (c) 2005-2009 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <netdev.h>

int checkboard(void)
{
	printf("Board: BlackVME\n");
	printf("Support: http://www.skutek.com/\n");
	return 0;
}

#ifdef CONFIG_DRIVER_AX88180
/*
 * The ax88180 driver had to be patched to work around a bug
 * in Marvell 88E1111 B2 silicon. E-mail me for explanations.
 */
int board_eth_init(bd_t *bis)
{
	return ax88180_initialize(bis);
}
#endif	/* CONFIG_DRIVER_AX88180 */
