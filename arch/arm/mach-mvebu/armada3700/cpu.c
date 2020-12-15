// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 * Copyright (C) 2020 Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/libfdt.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>
#include <sort.h>

/* Armada 3700 */
#define MVEBU_GPIO_NB_REG_BASE		(MVEBU_REGISTER(0x13800))

#define MVEBU_TEST_PIN_LATCH_N		(MVEBU_GPIO_NB_REG_BASE + 0x8)
#define MVEBU_XTAL_MODE_MASK		BIT(9)
#define MVEBU_XTAL_MODE_OFFS		9
#define MVEBU_XTAL_CLOCK_25MHZ		0x0
#define MVEBU_XTAL_CLOCK_40MHZ		0x1

#define MVEBU_NB_WARM_RST_REG		(MVEBU_GPIO_NB_REG_BASE + 0x40)
#define MVEBU_NB_WARM_RST_MAGIC_NUM	0x1d1e

/* Armada 3700 CPU Address Decoder registers */
#define MVEBU_CPU_DEC_WIN_REG_BASE	(size_t)(MVEBU_REGISTER(0xcf00))
#define MVEBU_CPU_DEC_WIN_CTRL(w) \
	(MVEBU_CPU_DEC_WIN_REG_BASE + ((w) << 4))
#define MVEBU_CPU_DEC_WIN_CTRL_EN	BIT(0)
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_MASK	0xf
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_OFFS	4
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM	0
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_PCIE	2
#define MVEBU_CPU_DEC_WIN_SIZE(w)	(MVEBU_CPU_DEC_WIN_CTRL(w) + 0x4)
#define MVEBU_CPU_DEC_WIN_BASE(w)	(MVEBU_CPU_DEC_WIN_CTRL(w) + 0x8)
#define MVEBU_CPU_DEC_WIN_REMAP(w)	(MVEBU_CPU_DEC_WIN_CTRL(w) + 0xc)
#define MVEBU_CPU_DEC_WIN_GRANULARITY	16
#define MVEBU_CPU_DEC_WINS		5

#define MAX_MEM_MAP_REGIONS		(MVEBU_CPU_DEC_WINS + 2)

#define A3700_PTE_BLOCK_NORMAL \
	(PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE)
#define A3700_PTE_BLOCK_DEVICE \
	(PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE)

#define PCIE_PATH			"/soc/pcie@d0070000"

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region mvebu_mem_map[MAX_MEM_MAP_REGIONS] = {
	{
		/*
		 * SRAM, MMIO regions
		 * Don't remove this, a3700_build_mem_map needs it.
		 */
		.phys = SOC_REGS_PHY_BASE,
		.virt = SOC_REGS_PHY_BASE,
		.size = 0x02000000UL,	/* 32MiB internal registers */
		.attrs = A3700_PTE_BLOCK_DEVICE
	},
};

struct mm_region *mem_map = mvebu_mem_map;

static int get_cpu_dec_win(int win, u32 *tgt, u32 *base, u32 *size)
{
	u32 reg;

	reg = readl(MVEBU_CPU_DEC_WIN_CTRL(win));
	if (!(reg & MVEBU_CPU_DEC_WIN_CTRL_EN))
		return -1;

	if (tgt) {
		reg >>= MVEBU_CPU_DEC_WIN_CTRL_TGT_OFFS;
		reg &= MVEBU_CPU_DEC_WIN_CTRL_TGT_MASK;
		*tgt = reg;
	}

	if (base) {
		reg = readl(MVEBU_CPU_DEC_WIN_BASE(win));
		*base = reg << MVEBU_CPU_DEC_WIN_GRANULARITY;
	}

	if (size) {
		/*
		 * Window size is encoded as the number of 1s from LSB to MSB,
		 * followed by 0s. The number of 1s specifies the size in 64 KiB
		 * granularity.
		 */
		reg = readl(MVEBU_CPU_DEC_WIN_SIZE(win));
		*size = ((reg + 1) << MVEBU_CPU_DEC_WIN_GRANULARITY);
	}

	return 0;
}

/*
 * Builds mem_map according to CPU Address Decoder settings, which were set by
 * the TIMH image on the Cortex-M3 secure processor, or by ARM Trusted Firmware
 */
static void build_mem_map(void)
{
	int win, region;

	region = 1;
	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt, size;
		u64 attrs;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, &size))
			continue;

		if (tgt == MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM)
			attrs = A3700_PTE_BLOCK_NORMAL;
		else if (tgt == MVEBU_CPU_DEC_WIN_CTRL_TGT_PCIE)
			attrs = A3700_PTE_BLOCK_DEVICE;
		else
			/* skip windows with other targets */
			continue;

		mvebu_mem_map[region].phys = base;
		mvebu_mem_map[region].virt = base;
		mvebu_mem_map[region].size = size;
		mvebu_mem_map[region].attrs = attrs;
		++region;
	}

	/* add list terminator */
	mvebu_mem_map[region].size = 0;
	mvebu_mem_map[region].attrs = 0;
}

void enable_caches(void)
{
	build_mem_map();

	icache_enable();
	dcache_enable();
}

int a3700_dram_init(void)
{
	int win;

	gd->ram_size = 0;
	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt, size;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, &size))
			continue;

		/* skip non-DRAM windows */
		if (tgt != MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM)
			continue;

		/*
		 * It is possible that one image was built for boards with
		 * different RAM sizes, for example 512 MiB and 1 GiB.
		 * We therefore try to determine the actual RAM size in the
		 * window with get_ram_size.
		 */
		gd->ram_size += get_ram_size((void *)(size_t)base, size);
	}

	return 0;
}

struct a3700_dram_window {
	size_t base, size;
};

static int dram_win_cmp(const void *a, const void *b)
{
	size_t ab, bb;

	ab = ((const struct a3700_dram_window *)a)->base;
	bb = ((const struct a3700_dram_window *)b)->base;

	if (ab < bb)
		return -1;
	else if (ab > bb)
		return 1;
	else
		return 0;
}

int a3700_dram_init_banksize(void)
{
	struct a3700_dram_window dram_wins[MVEBU_CPU_DEC_WINS];
	int bank, win, ndram_wins;
	u32 last_end;
	size_t size;

	ndram_wins = 0;
	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt, size;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, &size))
			continue;

		/* skip non-DRAM windows */
		if (tgt != MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM)
			continue;

		dram_wins[win].base = base;
		dram_wins[win].size = size;
		++ndram_wins;
	}

	qsort(dram_wins, ndram_wins, sizeof(dram_wins[0]), dram_win_cmp);

	bank = 0;
	last_end = -1;

	for (win = 0; win < ndram_wins; ++win) {
		/* again determining actual RAM size as in a3700_dram_init */
		size = get_ram_size((void *)dram_wins[win].base,
				    dram_wins[win].size);

		/*
		 * Check if previous window ends as the current starts. If yes,
		 * merge these windows into one "bank". This is possible by this
		 * simple check thanks to mem_map regions being qsorted in
		 * build_mem_map.
		 */
		if (last_end == dram_wins[win].base) {
			gd->bd->bi_dram[bank - 1].size += size;
			last_end += size;
		} else {
			if (bank == CONFIG_NR_DRAM_BANKS) {
				printf("Need more CONFIG_NR_DRAM_BANKS\n");
				return -ENOBUFS;
			}

			gd->bd->bi_dram[bank].start = dram_wins[win].base;
			gd->bd->bi_dram[bank].size = size;
			last_end = dram_wins[win].base + size;
			++bank;
		}
	}

	/*
	 * If there is more place for DRAM BANKS definitions than needed, fill
	 * the rest with zeros.
	 */
	for (; bank < CONFIG_NR_DRAM_BANKS; ++bank) {
		gd->bd->bi_dram[bank].start = 0;
		gd->bd->bi_dram[bank].size = 0;
	}

	return 0;
}

static u32 find_pcie_window_base(void)
{
	int win;

	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, NULL))
			continue;

		if (tgt == MVEBU_CPU_DEC_WIN_CTRL_TGT_PCIE)
			return base;
	}

	return -1;
}

int a3700_fdt_fix_pcie_regions(void *blob)
{
	u32 new_ranges[14], base;
	const u32 *ranges;
	int node, len;

	node = fdt_path_offset(blob, PCIE_PATH);
	if (node < 0)
		return node;

	ranges = fdt_getprop(blob, node, "ranges", &len);
	if (!ranges)
		return -ENOENT;

	if (len != sizeof(new_ranges))
		return -EINVAL;

	memcpy(new_ranges, ranges, len);

	base = find_pcie_window_base();
	if (base == -1)
		return -ENOENT;

	new_ranges[2] = cpu_to_fdt32(base);
	new_ranges[4] = new_ranges[2];

	new_ranges[9] = cpu_to_fdt32(base + 0x1000000);
	new_ranges[11] = new_ranges[9];

	return fdt_setprop_inplace(blob, node, "ranges", new_ranges, len);
}

void reset_cpu(void)
{
	/*
	 * Write magic number of 0x1d1e to North Bridge Warm Reset register
	 * to trigger warm reset
	 */
	writel(MVEBU_NB_WARM_RST_MAGIC_NUM, MVEBU_NB_WARM_RST_REG);
}

/*
 * get_ref_clk
 *
 * return: reference clock in MHz (25 or 40)
 */
u32 get_ref_clk(void)
{
	u32 regval;

	regval = (readl(MVEBU_TEST_PIN_LATCH_N) & MVEBU_XTAL_MODE_MASK) >>
		MVEBU_XTAL_MODE_OFFS;

	if (regval == MVEBU_XTAL_CLOCK_25MHZ)
		return 25;
	else
		return 40;
}
