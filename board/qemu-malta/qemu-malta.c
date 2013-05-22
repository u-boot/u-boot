/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <common.h>

#include <asm/io.h>
#include <asm/malta.h>

phys_size_t initdram(int board_type)
{
	return CONFIG_SYS_MEM_SIZE;
}

int checkboard(void)
{
	puts("Board: MIPS Malta CoreLV (Qemu)\n");
	return 0;
}

void _machine_restart(void)
{
	void __iomem *reset_base;

	reset_base = (void __iomem *)CKSEG1ADDR(MALTA_RESET_BASE);
	__raw_writel(GORESET, reset_base);
}
