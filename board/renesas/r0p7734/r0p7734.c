/*
 * Copyright (C) 2011 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2011 Renesas Solutions Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#define MODEMR			(0xFFCC0020)
#define MODEMR_MASK		(0x6)
#define MODEMR_533MHZ	(0x2)

int checkboard(void)
{
	u32 r = readl(MODEMR);
	if ((r & MODEMR_MASK) & MODEMR_533MHZ)
		puts("CPU Clock: 533MHz\n");
	else
		puts("CPU Clock: 400MHz\n");

	puts("BOARD: Renesas Technology Corp. R0P7734C00000RZ\n");
	return 0;
}

#define MSTPSR1			(0xFFC80044)
#define MSTPCR1			(0xFFC80034)
#define MSTPSR1_GETHER	(1 << 14)

int board_init(void)
{
#if defined(CONFIG_SH_ETHER)
	u32 r = readl(MSTPSR1);
	if (r & MSTPSR1_GETHER)
		writel((r & ~MSTPSR1_GETHER), MSTPCR1);
#endif
	return 0;
}

int dram_init(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	printf("DRAM:  %dMB\n", CONFIG_SYS_SDRAM_SIZE / (1024 * 1024));

	return 0;
}

#ifdef CONFIG_SMC911X
int board_eth_init(bd_t *bis)
{
	int rc = 0;
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
	return rc;
}
#endif

