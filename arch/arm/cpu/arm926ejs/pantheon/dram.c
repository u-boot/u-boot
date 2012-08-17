/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>,
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/pantheon.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Pantheon DRAM controller supports upto 8 banks
 * for chip select 0 and 1
 */

/*
 * DDR Memory Control Registers
 * Refer Datasheet 4.4
 */
struct panthddr_map_registers {
	u32	cs;	/* Memory Address Map Register -CS */
	u32	pad[3];
};

struct panthddr_registers {
	u8	pad[0x100 - 0x000];
	struct panthddr_map_registers mmap[2];
};

/*
 * panth_sdram_base - reads SDRAM Base Address Register
 */
u32 panth_sdram_base(int chip_sel)
{
	struct panthddr_registers *ddr_regs =
		(struct panthddr_registers *)PANTHEON_DRAM_BASE;
	u32 result = 0;
	u32 CS_valid = 0x01 & readl(&ddr_regs->mmap[chip_sel].cs);

	if (!CS_valid)
		return 0;

	result = readl(&ddr_regs->mmap[chip_sel].cs) & 0xFF800000;
	return result;
}

/*
 * panth_sdram_size - reads SDRAM size
 */
u32 panth_sdram_size(int chip_sel)
{
	struct panthddr_registers *ddr_regs =
		(struct panthddr_registers *)PANTHEON_DRAM_BASE;
	u32 result = 0;
	u32 CS_valid = 0x01 & readl(&ddr_regs->mmap[chip_sel].cs);

	if (!CS_valid)
		return 0;

	result = readl(&ddr_regs->mmap[chip_sel].cs);
	result = (result >> 16) & 0xF;
	if (result < 0x7) {
		printf("Unknown DRAM Size\n");
		return -1;
	} else {
		return ((0x8 << (result - 0x7)) * 1024 * 1024);
	}
}

#ifndef CONFIG_SYS_BOARD_DRAM_INIT
int dram_init(void)
{
	int i;

	gd->ram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = panth_sdram_base(i);
		gd->bd->bi_dram[i].size = panth_sdram_size(i);
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
		/*
		 * If above loop terminated prematurely, we need to set
		 * remaining banks' start address & size as 0. Otherwise other
		 * u-boot functions and Linux kernel gets wrong values which
		 * could result in crash
		 */
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
