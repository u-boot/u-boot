/*
 * (C) Copyright 2008
 * Daniel Hellstrom, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <config.h>
#include <asm/leon.h>

phys_size_t initdram(int board_type)
{
	return 1;
}

int checkboard(void)
{
	puts("Board: EP2S60 GRLIB\n");
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
	return rc;
}
#endif
