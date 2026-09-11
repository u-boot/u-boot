// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <asm/io.h>
#include <asm/arch/reset_manager.h>

/* Assert or de-assert SoCFPGA reset manager reset. */
void socfpga_per_reset(u32 reset, int set)
{
	unsigned long reg;

	if (RSTMGR_BANK(reset) == 0)
		reg = RSTMGR_SOC64_MPUMODRST;
	else if (RSTMGR_BANK(reset) == 1)
		reg = RSTMGR_SOC64_PER0MODRST;
	else if (RSTMGR_BANK(reset) == 2)
		reg = RSTMGR_SOC64_PER1MODRST;
	else if (RSTMGR_BANK(reset) == 3)
		reg = RSTMGR_SOC64_BRGMODRST;
	else	/* Invalid reset register, do nothing */
		return;

	if (set)
		setbits_le32(socfpga_get_rstmgr_addr() + reg,
			     1 << RSTMGR_RESET(reset));
	else
		clrbits_le32(socfpga_get_rstmgr_addr() + reg,
			     1 << RSTMGR_RESET(reset));
}

/*
 * Assert reset on every peripheral but L4WD0.
 * Watchdog must be kept intact to prevent glitches
 * and/or hangs.
 */
void socfpga_per_reset_all(void)
{
	const u32 l4wd0 = 1 << RSTMGR_RESET(SOCFPGA_RESET(L4WD0));

	/* disable all except OCP and l4wd0. OCP disable later */
	writel(~(l4wd0 | RSTMGR_PER0MODRST_OCP_MASK),
	       socfpga_get_rstmgr_addr() + RSTMGR_SOC64_PER0MODRST);
	writel(~l4wd0, socfpga_get_rstmgr_addr() + RSTMGR_SOC64_PER0MODRST);
	writel(0xffffffff, socfpga_get_rstmgr_addr() + RSTMGR_SOC64_PER1MODRST);
}

/*
 * Return non-zero if the CPU has been warm reset
 */
int cpu_has_been_warmreset(void)
{
	return readl(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_STATUS) &
			RSTMGR_L4WD_MPU_WARMRESET_MASK;
}

void print_reset_info(void)
{
	bool iswd;
	int n;
	u32 stat = cpu_has_been_warmreset();

	printf("Reset state: %s%s", stat ? "Warm " : "Cold",
	       (stat & RSTMGR_STAT_SDMWARMRST) ? "[from SDM] " : "");

	stat &= ~RSTMGR_STAT_SDMWARMRST;
	if (!stat) {
		puts("\n");
		return;
	}

	n = generic_ffs(stat) - 1;
	iswd = (n >= RSTMGR_STAT_L4WD0RST_BITPOS);
	printf("(Triggered by %s %d)\n", iswd ? "Watchdog" : "MPU",
	       iswd ? (n - RSTMGR_STAT_L4WD0RST_BITPOS) :
	       (n - RSTMGR_STAT_MPU0RST_BITPOS));
}
