/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <netdev.h>

#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/malta.h>
#include <pci_gt64120.h>

phys_size_t initdram(int board_type)
{
	return CONFIG_SYS_MEM_SIZE;
}

int checkboard(void)
{
	puts("Board: MIPS Malta CoreLV (Qemu)\n");
	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

void _machine_restart(void)
{
	void __iomem *reset_base;

	reset_base = (void __iomem *)CKSEG1ADDR(MALTA_RESET_BASE);
	__raw_writel(GORESET, reset_base);
}

void pci_init_board(void)
{
	set_io_port_base(CKSEG1ADDR(MALTA_IO_PORT_BASE));

	gt64120_pci_init((void *)CKSEG1ADDR(MALTA_GT_BASE),
			 0x00000000, 0x00000000, CONFIG_SYS_MEM_SIZE,
			 0x10000000, 0x10000000, 128 * 1024 * 1024,
			 0x00000000, 0x00000000, 0x20000);
}
