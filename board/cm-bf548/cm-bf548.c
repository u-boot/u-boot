/*
 * U-Boot - main board file
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <netdev.h>
#include <asm/blackfin.h>
#include <asm/portmux.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: Bluetechnix CM-BF548 board\n");
	printf("       Support: http://www.bluetechnix.at/\n");
	return 0;
}

int board_early_init_f(void)
{
	/* Set async addr lines as peripheral */
	const unsigned short pins[] = {
		P_A4, P_A5, P_A6, P_A7, P_A8, P_A9, P_A10, P_A11, P_A12,
		P_A13, P_A14, P_A15, P_A16, P_A17, P_A18, P_A19, P_A20,
		P_A21, P_A22, P_A23, P_A24, 0
	};
	return peripheral_request_list(pins, "async");
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
