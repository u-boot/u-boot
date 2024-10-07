// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <asm/arch-adi/sc5xx/soc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <cpu_func.h>

#ifdef CONFIG_SC58X
	#define RCU0_CTL                0x3108B000
	#define RCU0_STAT               0x3108B004
	#define RCU0_CRCTL              0x3108B008
	#define RCU0_CRSTAT             0x3108B00C
	#define RCU0_SIDIS              0x3108B010
	#define RCU0_MSG_SET            0x3108B064
#elif defined(CONFIG_SC57X) || defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	#define RCU0_CTL                0x3108C000
	#define RCU0_STAT               0x3108C004
	#define RCU0_CRCTL              0x3108C008
	#define RCU0_CRSTAT             0x3108C00C
	#define RCU0_SIDIS              0x3108C01C
	#define RCU0_MSG_SET            0x3108C070
#else
	#error "No SC5xx SoC CONFIG_ enabled"
#endif

#define BITP_RCU_STAT_BMODE                  8
#define BITM_RCU_STAT_BMODE         0x00000F00

#define REG_ARMPMU0_PMCR            0x31121E04
#define REG_ARMPMU0_PMUSERENR       0x31121E08
#define REG_ARMPMU0_PMLAR           0x31121FB0

DECLARE_GLOBAL_DATA_PTR;

void reset_cpu(void)
{
	u32 val = readl(RCU0_CTL);
	writel(val | 1, RCU0_CTL);
}

void enable_caches(void)
{
	if (!IS_ENABLED(CONFIG_SYS_DCACHE_OFF))
		dcache_enable();
}

void sc5xx_enable_ns_sharc_access(uintptr_t securec0_base)
{
	writel(0, securec0_base);
	writel(0, securec0_base + 0x4);
	writel(0, securec0_base + 0x8);
}

void sc5xx_disable_spu0(uintptr_t spu0_start, uintptr_t spu0_end)
{
	for (uintptr_t i = spu0_start; i <= spu0_end; i += 4)
		writel(0, i);
}

/**
 * PMU is only available on armv7 platforms and all share the same location
 */
void sc5xx_enable_pmu(void)
{
	if (!IS_ENABLED(CONFIG_SC59X_64)) {
		writel(readl(REG_ARMPMU0_PMUSERENR) | 0x01, REG_ARMPMU0_PMUSERENR);
		writel(0xc5acce55, REG_ARMPMU0_PMLAR);
		writel(readl(REG_ARMPMU0_PMCR) | (1 << 1), REG_ARMPMU0_PMCR);
	}
}

const char *sc5xx_get_boot_mode(u32 *bmode)
{
	static const char * const bmodes[] = {
		"JTAG/BOOTROM",
		"QSPI Master",
		"QSPI Slave",
		"UART",
		"LP0 Slave",
		"OSPI",
#ifdef CONFIG_SC59X_64
		"eMMC"
#endif
	};
	u32 local_mode;

	local_mode = (readl(RCU0_STAT) & BITM_RCU_STAT_BMODE) >> BITP_RCU_STAT_BMODE;

#if CONFIG_ADI_SPL_FORCE_BMODE != 0
	/*
	 * In case we want to force boot sequences such as:
	 * QSPI -> OSPI
	 * QSPI -> eMMC
	 * If this is not set, then we will always try to use the BMODE setting
	 * for both stages... i.e.
	 * QSPI -> QSPI
	 */

	// (Don't allow skipping JTAG/UART BMODE settings)
	if (local_mode != 0 && local_mode != 3)
		local_mode = CONFIG_ADI_SPL_FORCE_BMODE;
#endif

	*bmode = local_mode;

	if (local_mode >= 0 && local_mode <= ARRAY_SIZE(bmodes))
		return bmodes[local_mode];
	return "unknown";
}

void print_cpu_id(void)
{
	if (!IS_ENABLED(CONFIG_ARM64)) {
		u32 cpuid = 0;

		__asm__ __volatile__("mrc p15, 0, %0, c0, c0, 0" : "=r"(cpuid));

		printf("Detected Revision: %d.%d\n", cpuid & 0xf00000 >> 20, cpuid & 0xf);
	}
}

int print_cpuinfo(void)
{
	u32 bmode;

	printf("CPU:   ADSP %s (%s boot)\n", CONFIG_LDR_CPU, sc5xx_get_boot_mode(&bmode));
	print_cpu_id();

	return 0;
}

void fixup_dp83867_phy(struct phy_device *phydev)
{
	int phy_data = 0;

	phy_data = phy_read(phydev, MDIO_DEVAD_NONE, 0x32);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x32, (1 << 7) | phy_data);
	int cfg3 = 0;
	#define MII_DP83867_CFG3    (0x1e)
	/*
	 * Pin INT/PWDN on DP83867 should be configured as an Interrupt Output
	 * instead of a Power-Down Input on ADI SC5XX boards in order to
	 * prevent the signal interference from other peripherals during they
	 * are running at the same time.
	 */
	cfg3 = phy_read(phydev, MDIO_DEVAD_NONE, MII_DP83867_CFG3);
	cfg3 |= (1 << 7);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_DP83867_CFG3, cfg3);

	// Mystery second port fixup on ezkits with two PHYs
	if (CONFIG_DW_PORTS & 2)
		phy_write(phydev, MDIO_DEVAD_NONE, 0x11, 3);

	if (IS_ENABLED(CONFIG_ADI_BUG_EZKHW21)) {
		phydev->advertising &= PHY_BASIC_FEATURES;
		phydev->speed = SPEED_100;
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	if (IS_ENABLED(CONFIG_ADI_BUG_EZKHW21))
		phy_write(phydev, MDIO_DEVAD_NONE, 0, 0x3100);
}

extern char __bss_start, __bss_end;
extern char __rel_dyn_end;

void bss_clear(void)
{
	char *bss_start = &__bss_start;
	char *bss_end = &__bss_end;
	char *rel_dyn_end = &__rel_dyn_end;

	char *start;

	if (rel_dyn_end >= bss_start && rel_dyn_end <= bss_end)
		start = rel_dyn_end;
	else
		start = bss_start;

	u32 *pt;
	size_t sz = bss_end - start;

	for (int i = 0; i < sz; i += 4) {
		pt = (u32 *)(start + i);
		*pt = 0;
	}
}

int board_early_init_f(void)
{
	bss_clear();
	return 0;
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = CFG_SYS_SDRAM_SIZE;
	return 0;
}
