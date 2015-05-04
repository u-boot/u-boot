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
#include "../cm-bf537e/gpio_cfi_flash.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: Bluetechnix CM-BF537U board\n");
	printf("       Support: http://www.bluetechnix.at/\n");
	return 0;
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
	gpio_cfi_flash_init();

	return 0;
}
