/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <config.h>
#include <common.h>
#include <asm/arch/kirkwood.h>

DECLARE_GLOBAL_DATA_PTR;

#define KW_REG_CPUCS_WIN_BAR(x)		(KW_REGISTER(0x1500) + (x * 0x08))
#define KW_REG_CPUCS_WIN_SZ(x)		(KW_REGISTER(0x1504) + (x * 0x08))
/*
 * kw_sdram_bar - reads SDRAM Base Address Register
 */
u32 kw_sdram_bar(enum memory_bank bank)
{
	u32 result = 0;
	u32 enable = 0x01 & readl(KW_REG_CPUCS_WIN_SZ(bank));

	if ((!enable) || (bank > BANK3))
		return 0;

	result = readl(KW_REG_CPUCS_WIN_BAR(bank));
	return result;
}

/*
 * kw_sdram_bs - reads SDRAM Bank size
 */
u32 kw_sdram_bs(enum memory_bank bank)
{
	u32 result = 0;
	u32 enable = 0x01 & readl(KW_REG_CPUCS_WIN_SZ(bank));

	if ((!enable) || (bank > BANK3))
		return 0;
	result = 0xff000000 & readl(KW_REG_CPUCS_WIN_SZ(bank));
	result += 0x01000000;
	return result;
}

#ifndef CONFIG_SYS_BOARD_DRAM_INIT
int dram_init(void)
{
	int i;

	gd->ram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = kw_sdram_bar(i);
		gd->bd->bi_dram[i].size = kw_sdram_bs(i);
		/*
		 * It is assumed that all memory banks are consecutive
		 * and without gaps.
		 * If the gap is found, ram_size will be reported for
		 * consecutive memory only
		 */
		if (gd->bd->bi_dram[i].start != gd->ram_size)
			break;

		gd->ram_size += gd->bd->bi_dram[i].size;

	}

	for (; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* If above loop terminated prematurely, we need to set
		 * remaining banks' start address & size as 0. Otherwise other
		 * u-boot functions and Linux kernel gets wrong values which
		 * could result in crash */
		gd->bd->bi_dram[i].start = 0;
		gd->bd->bi_dram[i].size = 0;
	}

	return 0;
}

/*
 * If this function is not defined here,
 * board.c alters dram bank zero configuration defined above.
 */
void dram_init_banksize(void)
{
	dram_init();
}
#endif /* CONFIG_SYS_BOARD_DRAM_INIT */
