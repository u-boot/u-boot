/*
 * Copyright (C) 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * Copyright (C) 2007
 * Kenati Technologies, Inc.
 * 
 * board/ms7722se/ms7722se.c
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

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_memstart = CFG_SDRAM_BASE;
	gd->bd->bi_memsize = CFG_SDRAM_SIZE;
	printf("DRAM:  %dMB\n", CFG_SDRAM_SIZE / (1024 * 1024));
	return 0;
}

void led_set_state (unsigned short value)
{
	*((volatile unsigned short *) LED_BASE) = (value & 0xFF);
}

