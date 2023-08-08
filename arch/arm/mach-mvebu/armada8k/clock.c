/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

#include <common.h>
#include <mach/clock.h>
#include <mvebu/mvebu_chip_sar.h>

#ifndef CONFIG_PALLADIUM
#define CONFIG_MSS_FREQUENCY    (200 * 1000000)
#else
#define CONFIG_MSS_FREQUENCY    (384000)
#endif

u32 soc_ring_clk_get(void)
{
	struct sar_val sar;

	mvebu_sar_value_get(SAR_AP_FABRIC_FREQ, &sar);
	return sar.freq;
}

u32 soc_mss_clk_get(void)
{
	return CONFIG_MSS_FREQUENCY;
}

u32 soc_cpu_clk_get(void)
{
	struct sar_val sar;

	mvebu_sar_value_get(SAR_CPU_FREQ, &sar);
	return sar.freq;
}

u32 soc_ddr_clk_get(void)
{
	struct sar_val sar;

	mvebu_sar_value_get(SAR_DDR_FREQ, &sar);
	return sar.freq;
}

void soc_print_clock_info(void)
{
	printf("Clock:  CPU     %-4d [MHz]\n", soc_cpu_clk_get() / MHz);
	printf("\tDDR     %-4d [MHz]\n", soc_ddr_clk_get() / MHz);
	printf("\tFABRIC  %-4d [MHz]\n", soc_ring_clk_get() / MHz);
	printf("\tMSS     %-4d [MHz]\n", soc_mss_clk_get() / MHz);
}
