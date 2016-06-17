/*
 * U-Boot - blackstamp.c BlackStamp board specific routines
 * Most code stolen from boards/bf533-stamp/bf533-stamp.c
 * Edited to the BlackStamp by Ben Matthews for UR LLE
 *
 * Copyright (c) 2005-2009 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <netdev.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: BlackStamp\n");
	printf("Support: http://blackfin.uclinux.org/gf/project/blackstamp/\n");
	return 0;
}

#ifdef SHARED_RESOURCES
void swap_to(int device_id)
{
	gpio_request(GPIO_PF0, "eth_flash_swap");
	gpio_direction_output(GPIO_PF0, device_id == ETHERNET);
	SSYNC();
}
#endif

#ifdef CONFIG_SMC91111
int board_eth_init(bd_t *bis)
{
	return smc91111_initialize(0, CONFIG_SMC91111_BASE);
}
#endif
