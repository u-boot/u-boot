/*
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

const ulong patterns[]      = {	0x00000000,
				0xFFFFFFFF,
				0xFF00FF00,
				0x0F0F0F0F,
				0xF0F0F0F0};
const ulong NBR_OF_PATTERNS = sizeof(patterns) / sizeof(*patterns);
const ulong OFFS_PATTERN    = 3;
const ulong REPEAT_PATTERN  = 1000;

void bootcount_store(ulong a)
{
	ulong *save_addr;
	ulong size = 0;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size += gd->bd->bi_dram[i].size;
	save_addr = (ulong *)(size - BOOTCOUNT_ADDR);
	writel(a, save_addr);
	writel(BOOTCOUNT_MAGIC, &save_addr[1]);

	for (i = 0; i < REPEAT_PATTERN; i++)
		writel(patterns[i % NBR_OF_PATTERNS],
			&save_addr[i + OFFS_PATTERN]);

}

ulong bootcount_load(void)
{
	ulong *save_addr;
	ulong size = 0;
	ulong counter = 0;
	int i, tmp;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size += gd->bd->bi_dram[i].size;
	save_addr = (ulong *)(size - BOOTCOUNT_ADDR);

	counter = readl(&save_addr[0]);

	/* Is the counter reliable, check in the big pattern for bit errors */
	for (i = 0; (i < REPEAT_PATTERN) && (counter != 0); i++) {
		tmp = readl(&save_addr[i + OFFS_PATTERN]);
		if (tmp != patterns[i % NBR_OF_PATTERNS])
			counter = 0;
	}
	return counter;
}
