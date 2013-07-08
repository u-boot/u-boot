/*
 * Copyright (C) 2011 Renesas Electronics Europe Ltd.
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu
 *
 * Based on u-boot/board/rsk7264/rsk7203.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("BOARD: Renesas Technology RSK7264\n");
	return 0;
}

int board_init(void)
{
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
}

/*
 * The RSK board has the SMSC89218 wired up 'incorrectly'.
 * Byte-swapping is necessary, and so poor performance is inevitable.
 * This problem cannot evade by the swap function of CHIP, this can
 * evade by software Byte-swapping.
 * And this has problem by FIFO access only. pkt_data_pull/pkt_data_push
 * functions necessary to solve this problem.
 */
u32 pkt_data_pull(struct eth_device *dev, u32 addr)
{
	volatile u16 *addr_16 = (u16 *)(dev->iobase + addr);
	return (u32)((swab16(*addr_16) << 16) & 0xFFFF0000)\
				| swab16(*(addr_16 + 1));
}

void pkt_data_push(struct eth_device *dev, u32 addr, u32 val)
{
	addr += dev->iobase;
	*(volatile u16 *)(addr + 2) = swab16((u16)val);
	*(volatile u16 *)(addr) = swab16((u16)(val >> 16));
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
