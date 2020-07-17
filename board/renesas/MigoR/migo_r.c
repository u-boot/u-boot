// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * Copyright (C) 2007
 * Kenati Technologies, Inc.
 *
 * board/MigoR/migo_r.c
 */

#include <common.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/processor.h>

int checkboard(void)
{
	puts("BOARD: Renesas MigoR\n");
	return 0;
}

int board_init(void)
{
	return 0;
}

void led_set_state (unsigned short value)
{
}

#ifdef CONFIG_CMD_NET
int board_eth_init(struct bd_info *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
	return rc;
}
#endif
