// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Marvell International Ltd.
 *
 * Marvell Armada 8K SoC info: SAR, Clock frequencies, LLC status
 * Ported from Marvell U-Boot 2015.01 to mainline U-Boot.
 */

#include <config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <stdio.h>
#include <asm/io.h>
#include <asm/arch/soc.h>

/* Clock frequency units */
#define KHz			1000
#define MHz			1000000
#define GHz			1000000000

/* AP806 SAR (Sample-At-Reset) register */
#define AP806_SAR_REG_BASE		(SOC_REGS_PHY_BASE + 0x6F4400)
#define SAR_CLOCK_FREQ_MODE_OFFSET	0
#define SAR_CLOCK_FREQ_MODE_MASK	(0x1f << SAR_CLOCK_FREQ_MODE_OFFSET)

/* LLC (Last Level Cache) registers */
#define LLC_BASE			(SOC_REGS_PHY_BASE + 0x8000)
#define LLC_CTRL			0x100
#define LLC_CTRL_EN			0x1
#define LLC_EXCLUSIVE_EN		0x100

/* MSS clock is fixed at 200MHz on AP806 */
#define AP806_MSS_CLOCK			(200 * MHz)

/* Clock ID indices in PLL frequency table */
#define CPU_CLOCK_ID	0
#define DDR_CLOCK_ID	1
#define RING_CLOCK_ID	2

/* Clocking options (SAR field values) */
enum clocking_options {
	CPU_2000_DDR_1200_RCLK_1200 = 0x0,
	CPU_2000_DDR_1050_RCLK_1050 = 0x1,
	CPU_1600_DDR_800_RCLK_800 = 0x4,
	CPU_1800_DDR_1200_RCLK_1200 = 0x6,
	CPU_1800_DDR_1050_RCLK_1050 = 0x7,
	CPU_1600_DDR_900_RCLK_900 = 0x0b,
	CPU_1600_DDR_1050_RCLK_1050 = 0x0d,
	CPU_1600_DDR_900_RCLK_900_2 = 0x0e,
	CPU_1000_DDR_650_RCLK_650 = 0x13,
	CPU_1300_DDR_800_RCLK_800 = 0x14,
	CPU_1300_DDR_650_RCLK_650 = 0x17,
	CPU_1200_DDR_800_RCLK_800 = 0x19,
	CPU_1400_DDR_800_RCLK_800 = 0x1a,
	CPU_600_DDR_800_RCLK_800 = 0x1b,
	CPU_800_DDR_800_RCLK_800 = 0x1c,
	CPU_1000_DDR_800_RCLK_800 = 0x1d,
};

/*
 * PLL frequency table: maps SAR clock mode to actual frequencies.
 * Format: { CPU_freq, DDR_freq, RING_freq, SAR_value }
 */
static const u32 pll_freq_tbl[16][4] = {
	/* CPU */	/* DDR */	/* Ring */
	{2000 * MHz,	1200 * MHz,	1200 * MHz,	CPU_2000_DDR_1200_RCLK_1200},
	{2000 * MHz,	1050 * MHz,	1050 * MHz,	CPU_2000_DDR_1050_RCLK_1050},
	{1800 * MHz,	1200 * MHz,	1200 * MHz,	CPU_1800_DDR_1200_RCLK_1200},
	{1800 * MHz,	1050 * MHz,	1050 * MHz,	CPU_1800_DDR_1050_RCLK_1050},
	{1600 * MHz,	1050 * MHz,	1050 * MHz,	CPU_1600_DDR_1050_RCLK_1050},
	{1600 * MHz,	900 * MHz,	900 * MHz,	CPU_1600_DDR_900_RCLK_900_2},
	{1300 * MHz,	800 * MHz,	800 * MHz,	CPU_1300_DDR_800_RCLK_800},
	{1300 * MHz,	650 * MHz,	650 * MHz,	CPU_1300_DDR_650_RCLK_650},
	{1600 * MHz,	800 * MHz,	800 * MHz,	CPU_1600_DDR_800_RCLK_800},
	{1600 * MHz,	900 * MHz,	900 * MHz,	CPU_1600_DDR_900_RCLK_900},
	{1000 * MHz,	650 * MHz,	650 * MHz,	CPU_1000_DDR_650_RCLK_650},
	{1200 * MHz,	800 * MHz,	800 * MHz,	CPU_1200_DDR_800_RCLK_800},
	{1400 * MHz,	800 * MHz,	800 * MHz,	CPU_1400_DDR_800_RCLK_800},
	{600 * MHz,	800 * MHz,	800 * MHz,	CPU_600_DDR_800_RCLK_800},
	{800 * MHz,	800 * MHz,	800 * MHz,	CPU_800_DDR_800_RCLK_800},
	{1000 * MHz,	800 * MHz,	800 * MHz,	CPU_1000_DDR_800_RCLK_800}
};

/*
 * Get the clock frequency mode index from SAR register.
 * Returns index into pll_freq_tbl, or -1 if not found.
 */
static int sar_get_clock_freq_mode(void)
{
	u32 i;
	u32 clock_freq;

	clock_freq = (readl(AP806_SAR_REG_BASE) & SAR_CLOCK_FREQ_MODE_MASK)
			>> SAR_CLOCK_FREQ_MODE_OFFSET;

	for (i = 0; i < ARRAY_SIZE(pll_freq_tbl); i++) {
		if (pll_freq_tbl[i][3] == clock_freq)
			return i;
	}

	pr_err("SAR: unsupported clock freq mode %d\n", clock_freq);
	return -1;
}

/*
 * Get CPU clock frequency in Hz.
 */
static u32 soc_cpu_clk_get(void)
{
	int mode = sar_get_clock_freq_mode();

	if (mode < 0)
		return 0;
	return pll_freq_tbl[mode][CPU_CLOCK_ID];
}

/*
 * Get DDR clock frequency in Hz.
 */
static u32 soc_ddr_clk_get(void)
{
	int mode = sar_get_clock_freq_mode();

	if (mode < 0)
		return 0;
	return pll_freq_tbl[mode][DDR_CLOCK_ID];
}

/*
 * Get Ring (Fabric) clock frequency in Hz.
 */
static u32 soc_ring_clk_get(void)
{
	int mode = sar_get_clock_freq_mode();

	if (mode < 0)
		return 0;
	return pll_freq_tbl[mode][RING_CLOCK_ID];
}

/*
 * Get MSS clock frequency in Hz.
 */
static u32 soc_mss_clk_get(void)
{
	return AP806_MSS_CLOCK;
}

/*
 * Get LLC status and mode.
 * Returns 1 if LLC is enabled, 0 otherwise.
 * If excl_mode is not NULL, sets it to 1 if exclusive mode is enabled.
 */
static int llc_mode_get(int *excl_mode)
{
	u32 val;
	int ret = 0, excl = 0;

	val = readl(LLC_BASE + LLC_CTRL);
	if (val & LLC_CTRL_EN) {
		ret = 1;
		if (val & LLC_EXCLUSIVE_EN)
			excl = 1;
	}
	if (excl_mode)
		*excl_mode = excl;

	return ret;
}

/*
 * Print SoC clock information.
 */
void soc_print_clock_info(void)
{
	printf("Clock:  CPU     %-4d [MHz]\n", soc_cpu_clk_get() / MHz);
	printf("\tDDR     %-4d [MHz]\n", soc_ddr_clk_get() / MHz);
	printf("\tFABRIC  %-4d [MHz]\n", soc_ring_clk_get() / MHz);
	printf("\tMSS     %-4d [MHz]\n", soc_mss_clk_get() / MHz);
}

/*
 * Print SoC-specific information: DDR width and LLC status.
 */
void soc_print_soc_info(void)
{
	int llc_en, llc_excl_mode;

	printf("\tDDR 64 Bit width\n");

	llc_en = llc_mode_get(&llc_excl_mode);
	printf("\tLLC %s%s\n", llc_en ? "Enabled" : "Disabled",
	       llc_excl_mode ? " (Exclusive Mode)" : "");
}
