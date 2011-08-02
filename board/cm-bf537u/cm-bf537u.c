/*
 * U-boot - main board file
 *
 * Copyright (c) 2005-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <asm/blackfin.h>
#include <asm/net.h>
#include "../cm-bf537e/gpio_cfi_flash.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: Bluetechnix CM-BF537U board\n");
	printf("       Support: http://www.bluetechnix.at/\n");
	return 0;
}

static void board_init_enetaddr(char *var)
{
#ifdef CONFIG_NET_MULTI
	uchar enetaddr[6];

	if (eth_getenv_enetaddr(var, enetaddr))
		return;

	printf("Warning: %s: generating 'random' MAC address\n", var);
	bfin_gen_rand_mac(enetaddr);
	eth_setenv_enetaddr(var, enetaddr);
#endif
}

#ifndef CONFIG_BFIN_MAC
# define bfin_EMAC_initialize(x) 1
#endif
#ifndef CONFIG_SMC911X
# define smc911x_initialize(n, x) 1
#endif
int board_eth_init(bd_t *bis)
{
	/* return ok if at least 1 eth device works */
	return bfin_EMAC_initialize(bis) &
	       smc911x_initialize(0, CONFIG_SMC911X_BASE);
}

int misc_init_r(void)
{
	board_init_enetaddr("ethaddr");
	board_init_enetaddr("eth1addr");

	gpio_cfi_flash_init();

	return 0;
}
