/*
 * Copyright (C) 2007,2008
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * Copyright (C) 2007
 * Kenati Technologies, Inc.
 *
 * board/ms7722se/ms7722se.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#define LED_BASE	0xB0800000

int checkboard(void)
{
	puts("BOARD: Hitachi UL MS7722SE\n");
	return 0;
}

int board_init(void)
{
	/* Setup PTXMD[1:0] for /CS6A */
	outw(inw(PXCR) & ~0xf000, PXCR);

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	printf("DRAM:  %dMB\n", CONFIG_SYS_SDRAM_SIZE / (1024 * 1024));
	return 0;
}

void led_set_state(unsigned short value)
{
	writew(value & 0xFF, LED_BASE);
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
