/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

struct sdram_bank {
	u32	win_bar;
	u32	win_sz;
};

struct sdram_addr_dec {
	struct sdram_bank sdram_bank[4];
};

#define REG_CPUCS_WIN_ENABLE		(1 << 0)
#define REG_CPUCS_WIN_WR_PROTECT	(1 << 1)
#define REG_CPUCS_WIN_WIN0_CS(x)	(((x) & 0x3) << 2)
#define REG_CPUCS_WIN_SIZE(x)		(((x) & 0xff) << 24)

/*
 * mvebu_sdram_bar - reads SDRAM Base Address Register
 */
u32 mvebu_sdram_bar(enum memory_bank bank)
{
	struct sdram_addr_dec *base =
		(struct sdram_addr_dec *)MVEBU_SDRAM_BASE;
	u32 result = 0;
	u32 enable = 0x01 & readl(&base->sdram_bank[bank].win_sz);

	if ((!enable) || (bank > BANK3))
		return 0;

	result = readl(&base->sdram_bank[bank].win_bar);
	return result;
}

/*
 * mvebu_sdram_bs_set - writes SDRAM Bank size
 */
static void mvebu_sdram_bs_set(enum memory_bank bank, u32 size)
{
	struct sdram_addr_dec *base =
		(struct sdram_addr_dec *)MVEBU_SDRAM_BASE;
	/* Read current register value */
	u32 reg = readl(&base->sdram_bank[bank].win_sz);

	/* Clear window size */
	reg &= ~REG_CPUCS_WIN_SIZE(0xFF);

	/* Set new window size */
	reg |= REG_CPUCS_WIN_SIZE((size - 1) >> 24);

	writel(reg, &base->sdram_bank[bank].win_sz);
}

/*
 * mvebu_sdram_bs - reads SDRAM Bank size
 */
u32 mvebu_sdram_bs(enum memory_bank bank)
{
	struct sdram_addr_dec *base =
		(struct sdram_addr_dec *)MVEBU_SDRAM_BASE;
	u32 result = 0;
	u32 enable = 0x01 & readl(&base->sdram_bank[bank].win_sz);

	if ((!enable) || (bank > BANK3))
		return 0;
	result = 0xff000000 & readl(&base->sdram_bank[bank].win_sz);
	result += 0x01000000;
	return result;
}

void mvebu_sdram_size_adjust(enum memory_bank bank)
{
	u32 size;

	/* probe currently equipped RAM size */
	size = get_ram_size((void *)mvebu_sdram_bar(bank),
			    mvebu_sdram_bs(bank));

	/* adjust SDRAM window size accordingly */
	mvebu_sdram_bs_set(bank, size);
}

#ifndef CONFIG_SYS_BOARD_DRAM_INIT
int dram_init(void)
{
	int i;

	gd->ram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = mvebu_sdram_bar(i);
		gd->bd->bi_dram[i].size = mvebu_sdram_bs(i);
		/*
		 * It is assumed that all memory banks are consecutive
		 * and without gaps.
		 * If the gap is found, ram_size will be reported for
		 * consecutive memory only
		 */
		if (gd->bd->bi_dram[i].start != gd->ram_size)
			break;

		/*
		 * Don't report more than 3GiB of SDRAM, otherwise there is no
		 * address space left for the internal registers etc.
		 */
		if ((gd->ram_size + gd->bd->bi_dram[i].size != 0) &&
		    (gd->ram_size + gd->bd->bi_dram[i].size <= (3 << 30)))
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
