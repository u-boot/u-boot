// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 SpacemiT Technology Co. Ltd
 *
 * Port of Linux drivers/clk/spacemit/ccu-k3.c for U-Boot.
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <regmap.h>
#include <linux/clk-provider.h>
#include <soc/spacemit/k3-reset.h>
#include <soc/spacemit/k3-syscon.h>

#include "clk_common.h"
#include "clk_ddn.h"
#include "clk_mix.h"
#include "clk_pll.h"

#include <dt-bindings/clock/spacemit,k3-clocks.h>

/* Region base IDs — must exceed per-region binding ID ranges */
#define K3_PLL_ID		200
#define K3_MPMU_ID		400
#define K3_APBC_ID		600
#define K3_APMU_ID		900
#define K3_DCIU_ID		1200

struct spacemit_ccu_data {
	struct clk **clks;
	size_t num;
	unsigned long offset;
};

/* APBS clocks start, APBS region contains and only contains all PLL clocks */

/*
 * PLL{1,2} must run at fixed frequencies to provide clocks in correct rates for
 * peripherals.
 */
static const struct ccu_pll_rate_tbl pll1_rate_tbl[] = {
	CCU_PLLA_RATE(2457600000UL, 0x0b330ccc, 0x0000cd00, 0xa0558989),
};

static const struct ccu_pll_rate_tbl pll2_rate_tbl[] = {
	CCU_PLLA_RATE(3000000000UL, 0x0b3e2000, 0x00000000, 0xa0558c8c),
};

static const struct ccu_pll_rate_tbl pll3_rate_tbl[] = {
	CCU_PLLA_RATE(1050000000UL, 0x0b2b3000, 0x00000000, 0xa0458080),
	CCU_PLLA_RATE(1100000000UL, 0x0b2d3555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1150000000UL, 0x0b2f3aaa, 0x0000ab00, 0xa0458181),
	CCU_PLLA_RATE(1200000000UL, 0x0b320000, 0x00000000, 0xa0458181),
	CCU_PLLA_RATE(1250000000UL, 0x0b340555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1300000000UL, 0x0b360aaa, 0x0000ab00, 0xa0458282),
	CCU_PLLA_RATE(1350000000UL, 0x0b381000, 0x00000000, 0xa0458282),
	CCU_PLLA_RATE(1400000000UL, 0x0b3a1555, 0x00005500, 0xa0458282),
	CCU_PLLA_RATE(1450000000UL, 0x0b3c1aaa, 0x0000ab00, 0xa0458383),
	CCU_PLLA_RATE(1500000000UL, 0x0b3e2000, 0x00000000, 0xa0458383),
	CCU_PLLA_RATE(1550000000UL, 0x0b402555, 0x00005500, 0xa0458383),
	CCU_PLLA_RATE(1600000000UL, 0x0b422aaa, 0x0000ab00, 0xa0458484),
	CCU_PLLA_RATE(1650000000UL, 0x0b443000, 0x00000000, 0xa0458484),
	CCU_PLLA_RATE(1700000000UL, 0x0b463555, 0x00005500, 0xa0458484),
	CCU_PLLA_RATE(1750000000UL, 0x0b483aaa, 0x0000ab00, 0xa0458585),
	CCU_PLLA_RATE(1800000000UL, 0x0b4b0000, 0x00000000, 0xa0458585),
	CCU_PLLA_RATE(1850000000UL, 0x0b4d0555, 0x00005500, 0xa0458585),
	CCU_PLLA_RATE(1900000000UL, 0x0b4f0aaa, 0x0000ab00, 0xa0458686),
	CCU_PLLA_RATE(1950000000UL, 0x0b511000, 0x00000000, 0xa0458686),
	CCU_PLLA_RATE(2000000000UL, 0x0b292aaa, 0x0000ab00, 0xa0558686),
	CCU_PLLA_RATE(2050000000UL, 0x0b2a2d55, 0x00005500, 0xa0558686),
	CCU_PLLA_RATE(2100000000UL, 0x0b2b3000, 0x00000000, 0xa0558787),
	CCU_PLLA_RATE(2150000000UL, 0x0b2c32aa, 0x0000ab00, 0xa0558787),
	CCU_PLLA_RATE(2200000000UL, 0x0b2d3555, 0x00005500, 0xa0558787),
	CCU_PLLA_RATE(2250000000UL, 0x0b2e3800, 0x00000000, 0xa0558888),
	CCU_PLLA_RATE(2300000000UL, 0x0b2f3aaa, 0x0000ab00, 0xa0558888),
	CCU_PLLA_RATE(2350000000UL, 0x0b303d55, 0x00005500, 0xa0558888),
	CCU_PLLA_RATE(2400000000UL, 0x0b320000, 0x00000000, 0xa0558989),
};

static const struct ccu_pll_rate_tbl pll4_rate_tbl[] = {
	CCU_PLLA_RATE(1050000000UL, 0x0b2b3000, 0x00000000, 0xa0458080),
	CCU_PLLA_RATE(1100000000UL, 0x0b2d3555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1150000000UL, 0x0b2f3aaa, 0x0000ab00, 0xa0458181),
	CCU_PLLA_RATE(1200000000UL, 0x0b320000, 0x00000000, 0xa0458181),
	CCU_PLLA_RATE(1250000000UL, 0x0b340555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1300000000UL, 0x0b360aaa, 0x0000ab00, 0xa0458282),
	CCU_PLLA_RATE(1350000000UL, 0x0b381000, 0x00000000, 0xa0458282),
	CCU_PLLA_RATE(1400000000UL, 0x0b3a1555, 0x00005500, 0xa0458282),
	CCU_PLLA_RATE(1450000000UL, 0x0b3c1aaa, 0x0000ab00, 0xa0458383),
	CCU_PLLA_RATE(1500000000UL, 0x0b3e2000, 0x00000000, 0xa0458383),
	CCU_PLLA_RATE(1550000000UL, 0x0b402555, 0x00005500, 0xa0458383),
	CCU_PLLA_RATE(1600000000UL, 0x0b422aaa, 0x0000ab00, 0xa0458484),
	CCU_PLLA_RATE(1650000000UL, 0x0b443000, 0x00000000, 0xa0458484),
	CCU_PLLA_RATE(1700000000UL, 0x0b463555, 0x00005500, 0xa0458484),
	CCU_PLLA_RATE(1750000000UL, 0x0b483aaa, 0x0000ab00, 0xa0458585),
	CCU_PLLA_RATE(1800000000UL, 0x0b4b0000, 0x00000000, 0xa0458585),
	CCU_PLLA_RATE(1850000000UL, 0x0b4d0555, 0x00005500, 0xa0458585),
	CCU_PLLA_RATE(1900000000UL, 0x0b4f0aaa, 0x0000ab00, 0xa0458686),
	CCU_PLLA_RATE(1950000000UL, 0x0b511000, 0x00000000, 0xa0458686),
	CCU_PLLA_RATE(2000000000UL, 0x0b292aaa, 0x0000ab00, 0xa0558686),
	CCU_PLLA_RATE(2050000000UL, 0x0b2a2d55, 0x00005500, 0xa0558686),
	CCU_PLLA_RATE(2100000000UL, 0x0b2b3000, 0x00000000, 0xa0558787),
	CCU_PLLA_RATE(2150000000UL, 0x0b2c32aa, 0x0000ab00, 0xa0558787),
	CCU_PLLA_RATE(2200000000UL, 0x0b2d3555, 0x00005500, 0xa0558787),
	CCU_PLLA_RATE(2250000000UL, 0x0b2e3800, 0x00000000, 0xa0558888),
	CCU_PLLA_RATE(2300000000UL, 0x0b2f3aaa, 0x0000ab00, 0xa0558888),
	CCU_PLLA_RATE(2350000000UL, 0x0b303d55, 0x00005500, 0xa0558888),
	CCU_PLLA_RATE(2400000000UL, 0x0b320000, 0x00000000, 0xa0558989),
};

static const struct ccu_pll_rate_tbl pll5_rate_tbl[] = {
	CCU_PLLA_RATE(1050000000UL, 0x0b2b3000, 0x00000000, 0xa0458080),
	CCU_PLLA_RATE(1100000000UL, 0x0b2d3555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1150000000UL, 0x0b2f3aaa, 0x0000ab00, 0xa0458181),
	CCU_PLLA_RATE(1200000000UL, 0x0b320000, 0x00000000, 0xa0458181),
	CCU_PLLA_RATE(1250000000UL, 0x0b340555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1300000000UL, 0x0b360aaa, 0x0000ab00, 0xa0458282),
	CCU_PLLA_RATE(1350000000UL, 0x0b381000, 0x00000000, 0xa0458282),
	CCU_PLLA_RATE(1400000000UL, 0x0b3a1555, 0x00005500, 0xa0458282),
	CCU_PLLA_RATE(1450000000UL, 0x0b3c1aaa, 0x0000ab00, 0xa0458383),
	CCU_PLLA_RATE(1500000000UL, 0x0b3e2000, 0x00000000, 0xa0458383),
	CCU_PLLA_RATE(1550000000UL, 0x0b402555, 0x00005500, 0xa0458383),
	CCU_PLLA_RATE(1600000000UL, 0x0b422aaa, 0x0000ab00, 0xa0458484),
	CCU_PLLA_RATE(1650000000UL, 0x0b443000, 0x00000000, 0xa0458484),
	CCU_PLLA_RATE(1700000000UL, 0x0b463555, 0x00005500, 0xa0458484),
	CCU_PLLA_RATE(1750000000UL, 0x0b483aaa, 0x0000ab00, 0xa0458585),
	CCU_PLLA_RATE(1800000000UL, 0x0b4b0000, 0x00000000, 0xa0458585),
	CCU_PLLA_RATE(1850000000UL, 0x0b4d0555, 0x00005500, 0xa0458585),
	CCU_PLLA_RATE(1900000000UL, 0x0b4f0aaa, 0x0000ab00, 0xa0458686),
	CCU_PLLA_RATE(1950000000UL, 0x0b511000, 0x00000000, 0xa0458686),
	CCU_PLLA_RATE(2000000000UL, 0x0b292aaa, 0x0000ab00, 0xa0558686),
};

static const struct ccu_pll_rate_tbl pll6_rate_tbl[] = {
	CCU_PLLA_RATE(3200000000UL, 0x0b422aaa, 0x0000ab00, 0xa0558e8e),
};

static const struct ccu_pll_rate_tbl pll7_rate_tbl[] = {
	CCU_PLLA_RATE(2800000000UL, 0x0b3a1555, 0x00005500, 0xa0558b8b),
};

static const struct ccu_pll_rate_tbl pll8_rate_tbl[] = {
	CCU_PLLA_RATE(1050000000UL, 0x0b2b3000, 0x00000000, 0xa0458080),
	CCU_PLLA_RATE(1100000000UL, 0x0b2d3555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1150000000UL, 0x0b2f3aaa, 0x0000ab00, 0xa0458181),
	CCU_PLLA_RATE(1200000000UL, 0x0b320000, 0x00000000, 0xa0458181),
	CCU_PLLA_RATE(1250000000UL, 0x0b340555, 0x00005500, 0xa0458181),
	CCU_PLLA_RATE(1300000000UL, 0x0b360aaa, 0x0000ab00, 0xa0458282),
	CCU_PLLA_RATE(1350000000UL, 0x0b381000, 0x00000000, 0xa0458282),
	CCU_PLLA_RATE(1400000000UL, 0x0b3a1555, 0x00005500, 0xa0458282),
	CCU_PLLA_RATE(1450000000UL, 0x0b3c1aaa, 0x0000ab00, 0xa0458383),
	CCU_PLLA_RATE(1500000000UL, 0x0b3e2000, 0x00000000, 0xa0458383),
	CCU_PLLA_RATE(1550000000UL, 0x0b402555, 0x00005500, 0xa0458383),
	CCU_PLLA_RATE(1600000000UL, 0x0b422aaa, 0x0000ab00, 0xa0458484),
	CCU_PLLA_RATE(1650000000UL, 0x0b443000, 0x00000000, 0xa0458484),
	CCU_PLLA_RATE(1700000000UL, 0x0b463555, 0x00005500, 0xa0458484),
	CCU_PLLA_RATE(1750000000UL, 0x0b483aaa, 0x0000ab00, 0xa0458585),
	CCU_PLLA_RATE(1800000000UL, 0x0b4b0000, 0x00000000, 0xa0458585),
	CCU_PLLA_RATE(1850000000UL, 0x0b4d0555, 0x00005500, 0xa0458585),
	CCU_PLLA_RATE(1900000000UL, 0x0b4f0aaa, 0x0000ab00, 0xa0458686),
	CCU_PLLA_RATE(1950000000UL, 0x0b511000, 0x00000000, 0xa0458686),
	CCU_PLLA_RATE(2000000000UL, 0x0b292aaa, 0x0000ab00, 0xa0558686),
};

CCU_PLLA_DEFINE(CLK_PLL1, pll1, pll1, "clock-24m", pll1_rate_tbl,
		APBS_PLL1_SWCR1, APBS_PLL1_SWCR2, APBS_PLL1_SWCR3, MPMU_POSR, POSR_PLL1_LOCK,
		CLK_SET_RATE_GATE);
CCU_PLLA_DEFINE(CLK_PLL2, pll2, pll2, "clock-24m", pll2_rate_tbl,
		APBS_PLL2_SWCR1, APBS_PLL2_SWCR2, APBS_PLL2_SWCR3, MPMU_POSR, POSR_PLL2_LOCK,
		CLK_SET_RATE_GATE);
CCU_PLLA_DEFINE(CLK_PLL3, pll3, pll3, "clock-24m", pll3_rate_tbl,
		APBS_PLL3_SWCR1, APBS_PLL3_SWCR2, APBS_PLL3_SWCR3, MPMU_POSR, POSR_PLL3_LOCK,
		CLK_SET_RATE_GATE);
CCU_PLLA_DEFINE(CLK_PLL4, pll4, pll4, "clock-24m", pll4_rate_tbl,
		APBS_PLL4_SWCR1, APBS_PLL4_SWCR2, APBS_PLL4_SWCR3, MPMU_POSR, POSR_PLL4_LOCK,
		CLK_SET_RATE_GATE);
CCU_PLLA_DEFINE(CLK_PLL5, pll5, pll5, "clock-24m", pll5_rate_tbl,
		APBS_PLL5_SWCR1, APBS_PLL5_SWCR2, APBS_PLL5_SWCR3, MPMU_POSR, POSR_PLL5_LOCK,
		CLK_SET_RATE_GATE);
CCU_PLLA_DEFINE(CLK_PLL6, pll6, pll6, "clock-24m", pll6_rate_tbl,
		APBS_PLL6_SWCR1, APBS_PLL6_SWCR2, APBS_PLL6_SWCR3, MPMU_POSR, POSR_PLL6_LOCK,
		CLK_SET_RATE_GATE);
CCU_PLLA_DEFINE(CLK_PLL7, pll7, pll7, "clock-24m", pll7_rate_tbl,
		APBS_PLL7_SWCR1, APBS_PLL7_SWCR2, APBS_PLL7_SWCR3, MPMU_POSR, POSR_PLL7_LOCK,
		CLK_SET_RATE_GATE);
CCU_PLLA_DEFINE(CLK_PLL8, pll8, pll8, "clock-24m", pll8_rate_tbl,
		APBS_PLL8_SWCR1, APBS_PLL8_SWCR2, APBS_PLL8_SWCR3, MPMU_POSR, POSR_PLL8_LOCK,
		CLK_SET_RATE_GATE);

CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D2, pll1_d2, pll1_d2, "pll1",
		       APBS_PLL1_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D3, pll1_d3, pll1_d3, "pll1",
		       APBS_PLL1_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D4, pll1_d4, pll1_d4, "pll1",
		       APBS_PLL1_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D5, pll1_d5, pll1_d5, "pll1",
		       APBS_PLL1_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D6, pll1_d6, pll1_d6, "pll1",
		       APBS_PLL1_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D7, pll1_d7, pll1_d7, "pll1",
		       APBS_PLL1_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_FLAGS_DEFINE(CLK_PLL1_D8, pll1_d8, pll1_d8, "pll1",
			     APBS_PLL1_SWCR2, BIT(7), 8, 1, CLK_IS_CRITICAL);
CCU_DIV_GATE_DEFINE(CLK_PLL1_DX, pll1_dx, pll1_dx, "pll1",
		    APBS_PLL1_SWCR2, 23, 5, BIT(22), 0);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D64, pll1_d64_38p4, pll1_d64_38p4, "pll1",
		       APBS_PLL1_SWCR2, BIT(31), 64, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D10_AUD, pll1_aud_245p7, pll1_aud_245p7, "pll1",
		       APBS_PLL1_SWCR2, BIT(21), 10, 1);
CCU_FACTOR_DEFINE(CLK_PLL1_D100_AUD, pll1_aud_24p5, pll1_aud_24p5, "pll1_aud_245p7", 10, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D1, pll2_d1, pll2_d1, "pll2",
		       APBS_PLL2_SWCR2, BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D2, pll2_d2, pll2_d2, "pll2",
		       APBS_PLL2_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D3, pll2_d3, pll2_d3, "pll2",
		       APBS_PLL2_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D4, pll2_d4, pll2_d4, "pll2",
		       APBS_PLL2_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D5, pll2_d5, pll2_d5, "pll2",
		       APBS_PLL2_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D6, pll2_d6, pll2_d6, "pll2",
		       APBS_PLL2_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D7, pll2_d7, pll2_d7, "pll2",
		       APBS_PLL2_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D8, pll2_d8, pll2_d8, "pll2",
		       APBS_PLL2_SWCR2, BIT(7), 8, 1);
CCU_FACTOR_DEFINE(CLK_PLL2_66, pll2_66, pll2_66, "pll2_d5", 9, 1);
CCU_FACTOR_DEFINE(CLK_PLL2_33, pll2_33, pll2_33, "pll2_66", 2, 1);
CCU_FACTOR_DEFINE(CLK_PLL2_50, pll2_50, pll2_50, "pll2_d5", 12, 1);
CCU_FACTOR_DEFINE(CLK_PLL2_25, pll2_25, pll2_25, "pll2_50", 2, 1);
CCU_FACTOR_DEFINE(CLK_PLL2_20, pll2_20, pll2_20, "pll2_d5", 30, 1);
CCU_FACTOR_DEFINE(CLK_PLL2_D24_125, pll2_d24_125, pll2_d24_125, "pll2_d3", 8, 1);
CCU_FACTOR_DEFINE(CLK_PLL2_D120_25, pll2_d120_25, pll2_d120_25, "pll2_d3", 40, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D1, pll3_d1, pll3_d1, "pll3",
		       APBS_PLL3_SWCR2, BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D2, pll3_d2, pll3_d2, "pll3",
		       APBS_PLL3_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D3, pll3_d3, pll3_d3, "pll3",
		       APBS_PLL3_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D4, pll3_d4, pll3_d4, "pll3",
		       APBS_PLL3_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D5, pll3_d5, pll3_d5, "pll3",
		       APBS_PLL3_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D6, pll3_d6, pll3_d6, "pll3",
		       APBS_PLL3_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D7, pll3_d7, pll3_d7, "pll3",
		       APBS_PLL3_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D8, pll3_d8, pll3_d8, "pll3",
		       APBS_PLL3_SWCR2, BIT(7), 8, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D1, pll4_d1, pll4_d1, "pll4",
		       APBS_PLL4_SWCR2, BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D2, pll4_d2, pll4_d2, "pll4",
		       APBS_PLL4_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D3, pll4_d3, pll4_d3, "pll4",
		       APBS_PLL4_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D4, pll4_d4, pll4_d4, "pll4",
		       APBS_PLL4_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D5, pll4_d5, pll4_d5, "pll4",
		       APBS_PLL4_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D6, pll4_d6, pll4_d6, "pll4",
		       APBS_PLL4_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D7, pll4_d7, pll4_d7, "pll4",
		       APBS_PLL4_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL4_D8, pll4_d8, pll4_d8, "pll4",
		       APBS_PLL4_SWCR2, BIT(7), 8, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D1, pll5_d1, pll5_d1, "pll5",
		       APBS_PLL5_SWCR2, BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D2, pll5_d2, pll5_d2, "pll5",
		       APBS_PLL5_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D3, pll5_d3, pll5_d3, "pll5",
		       APBS_PLL5_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D4, pll5_d4, pll5_d4, "pll5",
		       APBS_PLL5_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D5, pll5_d5, pll5_d5, "pll5",
		       APBS_PLL5_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D6, pll5_d6, pll5_d6, "pll5",
		       APBS_PLL5_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D7, pll5_d7, pll5_d7, "pll5",
		       APBS_PLL5_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL5_D8, pll5_d8, pll5_d8, "pll5",
		       APBS_PLL5_SWCR2, BIT(7), 8, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D1, pll6_d1, pll6_d1, "pll6",
		       APBS_PLL6_SWCR2, BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D2, pll6_d2, pll6_d2, "pll6",
		       APBS_PLL6_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D3, pll6_d3, pll6_d3, "pll6",
		       APBS_PLL6_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D4, pll6_d4, pll6_d4, "pll6",
		       APBS_PLL6_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D5, pll6_d5, pll6_d5, "pll6",
		       APBS_PLL6_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D6, pll6_d6, pll6_d6, "pll6",
		       APBS_PLL6_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D7, pll6_d7, pll6_d7, "pll6",
		       APBS_PLL6_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL6_D8, pll6_d8, pll6_d8, "pll6",
		       APBS_PLL6_SWCR2, BIT(7), 8, 1);
CCU_FACTOR_DEFINE(CLK_PLL6_80, pll6_80, pll6_80, "pll6_d5", 8, 1);
CCU_FACTOR_DEFINE(CLK_PLL6_40, pll6_40, pll6_40, "pll6_d5", 16, 1);
CCU_FACTOR_DEFINE(CLK_PLL6_20, pll6_20, pll6_20, "pll6_d5", 32, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D1, pll7_d1, pll7_d1, "pll7",
		       APBS_PLL7_SWCR2, BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D2, pll7_d2, pll7_d2, "pll7",
		       APBS_PLL7_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D3, pll7_d3, pll7_d3, "pll7",
		       APBS_PLL7_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D4, pll7_d4, pll7_d4, "pll7",
		       APBS_PLL7_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D5, pll7_d5, pll7_d5, "pll7",
		       APBS_PLL7_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D6, pll7_d6, pll7_d6, "pll7",
		       APBS_PLL7_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D7, pll7_d7, pll7_d7, "pll7",
		       APBS_PLL7_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL7_D8, pll7_d8, pll7_d8, "pll7",
		       APBS_PLL7_SWCR2, BIT(7), 8, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D1, pll8_d1, pll8_d1, "pll8",
		       APBS_PLL8_SWCR2, BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D2, pll8_d2, pll8_d2, "pll8",
		       APBS_PLL8_SWCR2, BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D3, pll8_d3, pll8_d3, "pll8",
		       APBS_PLL8_SWCR2, BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D4, pll8_d4, pll8_d4, "pll8",
		       APBS_PLL8_SWCR2, BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D5, pll8_d5, pll8_d5, "pll8",
		       APBS_PLL8_SWCR2, BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D6, pll8_d6, pll8_d6, "pll8",
		       APBS_PLL8_SWCR2, BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D7, pll8_d7, pll8_d7, "pll8",
		       APBS_PLL8_SWCR2, BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL8_D8, pll8_d8, pll8_d8, "pll8",
		       APBS_PLL8_SWCR2, BIT(7), 8, 1);
/* APBS clocks end */

/* MPMU clocks start */
CCU_GATE_DEFINE(CLK_MPMU_PLL1_307P2, pll1_d8_307p2, pll1_d8_307p2, "pll1_d8",
		MPMU_ACGR, BIT(13), 0);
CCU_FACTOR_DEFINE(CLK_MPMU_PLL1_76P8, pll1_d32_76p8, pll1_d32_76p8, "pll1_d8_307p2", 4, 1);
CCU_FACTOR_DEFINE(CLK_MPMU_PLL1_61P44, pll1_d40_61p44, pll1_d40_61p44, "pll1_d8_307p2", 5, 1);
CCU_FACTOR_DEFINE(CLK_MPMU_PLL1_153P6, pll1_d16_153p6, pll1_d16_153p6, "pll1_d8", 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_102P4, pll1_d24_102p4, pll1_d24_102p4, "pll1_d8",
		       MPMU_ACGR, BIT(12), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_51P2, pll1_d48_51p2, pll1_d48_51p2, "pll1_d8",
		       MPMU_ACGR, BIT(7), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_51P2_AP, pll1_d48_51p2_ap, pll1_d48_51p2_ap, "pll1_d8",
		       MPMU_ACGR, BIT(11), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_57P6, pll1_m3d128_57p6, pll1_m3d128_57p6, "pll1_d8",
		       MPMU_ACGR, BIT(8), 16, 3);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_25P6, pll1_d96_25p6, pll1_d96_25p6, "pll1_d8",
		       MPMU_ACGR, BIT(4), 12, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_12P8, pll1_d192_12p8, pll1_d192_12p8, "pll1_d8",
		       MPMU_ACGR, BIT(3), 24, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_12P8_WDT, pll1_d192_12p8_wdt, pll1_d192_12p8_wdt, "pll1_d8",
		       MPMU_ACGR, BIT(19), 24, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_6P4, pll1_d384_6p4, pll1_d384_6p4, "pll1_d8",
		       MPMU_ACGR, BIT(2), 48, 1);

CCU_FACTOR_DEFINE(CLK_MPMU_PLL1_3P2, pll1_d768_3p2, pll1_d768_3p2, "pll1_d384_6p4", 2, 1);
CCU_FACTOR_DEFINE(CLK_MPMU_PLL1_1P6, pll1_d1536_1p6, pll1_d1536_1p6, "pll1_d384_6p4", 4, 1);
CCU_FACTOR_DEFINE(CLK_MPMU_PLL1_0P8, pll1_d3072_0p8, pll1_d3072_0p8, "pll1_d384_6p4", 8, 1);

CCU_GATE_DEFINE(CLK_MPMU_PLL1_409P6, pll1_d6_409p6, pll1_d6_409p6, "pll1_d6",
		MPMU_ACGR, BIT(0), 0);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_204P8, pll1_d12_204p8, pll1_d12_204p8, "pll1_d6",
		       MPMU_ACGR, BIT(5), 2, 1);

CCU_GATE_DEFINE(CLK_MPMU_PLL1_491, pll1_d5_491p52, pll1_d5_491p52, "pll1_d5",
		MPMU_ACGR, BIT(21), 0);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_245P76, pll1_d10_245p76, pll1_d10_245p76, "pll1_d5",
		       MPMU_ACGR, BIT(18), 2, 1);

CCU_GATE_DEFINE(CLK_MPMU_PLL1_614, pll1_d4_614p4, pll1_d4_614p4, "pll1_d4",
		MPMU_ACGR, BIT(15), 0);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_47P26, pll1_d52_47p26, pll1_d52_47p26, "pll1_d4",
		       MPMU_ACGR, BIT(10), 13, 1);
CCU_FACTOR_GATE_DEFINE(CLK_MPMU_PLL1_31P5, pll1_d78_31p5, pll1_d78_31p5, "pll1_d4",
		       MPMU_ACGR, BIT(6), 39, 2);

CCU_GATE_DEFINE(CLK_MPMU_PLL1_819, pll1_d3_819p2, pll1_d3_819p2, "pll1_d3",
		MPMU_ACGR, BIT(14), 0);

CCU_GATE_DEFINE(CLK_MPMU_PLL1_1228, pll1_d2_1228p8, pll1_d2_1228p8, "pll1_d2",
		MPMU_ACGR, BIT(16), 0);

static const char * const apb_parents[] = {
	"pll1_d96_25p6", "pll1_d48_51p2", "pll1_d96_25p6", "pll1_d24_102p4",
};
CCU_MUX_DEFINE(CLK_MPMU_APB, apb_clk, apb_clk, apb_parents, ARRAY_SIZE(apb_parents),
		MPMU_APBCSCR, 0, 2, 0);

CCU_GATE_DEFINE(CLK_MPMU_SLOW_UART, slow_uart, slow_uart, "clock-32k",
		MPMU_ACGR, BIT(1), CLK_IGNORE_UNUSED);
CCU_DDN_DEFINE(CLK_MPMU_SLOW_UART1, slow_uart1_14p74, slow_uart1_14p74, "pll1_d16_153p6", MPMU_SUCCR,
		CCU_DDN_MASK(16, 13), 16, CCU_DDN_MASK(0, 13), 0,
		2, 0);
CCU_DDN_DEFINE(CLK_MPMU_SLOW_UART2, slow_uart2_48, slow_uart2_48, "pll1_d4_614p4", MPMU_SUCCR_1,
		CCU_DDN_MASK(16, 13), 16, CCU_DDN_MASK(0, 13), 0,
		2, 0);

CCU_GATE_DEFINE(CLK_MPMU_WDT, wdt_clk, wdt_clk, "pll1_d96_25p6",
		MPMU_WDTPCR, BIT(1), 0);
CCU_GATE_DEFINE(CLK_MPMU_WDT_BUS, wdt_bus_clk, wdt_bus_clk, "apb_clk",
		MPMU_WDTPCR, BIT(0), 0);

CCU_GATE_DEFINE(CLK_MPMU_RIPC, r_ipc_clk, r_ipc_clk, "apb_clk",
		MPMU_RIPCCR, BIT(0), 0);

CCU_FACTOR_DEFINE(CLK_MPMU_I2S_153P6, i2s_153p6, i2s_153p6, "pll1_d8_307p2", 2, 1);

static const char * const i2s_153p6_base_parents[] = {
	"i2s_153p6", "pll1_d8_307p2",
};
CCU_MUX_DEFINE(CLK_MPMU_I2S_153P6_BASE, i2s_153p6_base, i2s_153p6_base, i2s_153p6_base_parents, ARRAY_SIZE(i2s_153p6_base_parents),
		MPMU_FCCR, 29, 1, 0);

static const char * const i2s_sysclk_src_parents[] = {
	"pll1_d96_25p6", "i2s_153p6_base",
};
CCU_MUX_GATE_DEFINE(CLK_MPMU_I2S_SYSCLK_SRC, i2s_sysclk_src, i2s_sysclk_src, i2s_sysclk_src_parents, ARRAY_SIZE(i2s_sysclk_src_parents),
		    MPMU_ISCCR, 30, 1, BIT(31), 0);

CCU_DDN_DEFINE(CLK_MPMU_I2S1_SYSCLK, i2s1_sysclk, i2s1_sysclk, "i2s_sysclk_src", MPMU_ISCCR,
		CCU_DDN_MASK(0, 15), 0, CCU_DDN_MASK(15, 12), 15,
		1, 0);

CCU_DIV_GATE_DEFINE(CLK_MPMU_I2S_BCLK, i2s_bclk, i2s_bclk, "i2s1_sysclk",
		    MPMU_ISCCR, 27, 2, BIT(29), 0);

static const char * const i2s_sysclk_parents[] = {
	"pll1_d4_614p4", "clock-24m", "pll2_d5", "clock-24m",
};
CCU_MUX_DEFINE(CLK_MPMU_I2S0_SYSCLK_SEL, i2s0_sysclk_sel, i2s0_sysclk_sel, i2s_sysclk_parents, ARRAY_SIZE(i2s_sysclk_parents),
		MPMU_I2S_SYSCLK_CTRL, 0, 2, 0);
CCU_MUX_DEFINE(CLK_MPMU_I2S2_SYSCLK_SEL, i2s2_sysclk_sel, i2s2_sysclk_sel, i2s_sysclk_parents, ARRAY_SIZE(i2s_sysclk_parents),
		MPMU_I2S_SYSCLK_CTRL, 4, 2, 0);
CCU_MUX_DEFINE(CLK_MPMU_I2S3_SYSCLK_SEL, i2s3_sysclk_sel, i2s3_sysclk_sel, i2s_sysclk_parents, ARRAY_SIZE(i2s_sysclk_parents),
		MPMU_I2S_SYSCLK_CTRL, 12, 2, 0);
CCU_MUX_DEFINE(CLK_MPMU_I2S4_SYSCLK_SEL, i2s4_sysclk_sel, i2s4_sysclk_sel, i2s_sysclk_parents, ARRAY_SIZE(i2s_sysclk_parents),
		MPMU_I2S_SYSCLK_CTRL, 16, 2, 0);
CCU_MUX_DEFINE(CLK_MPMU_I2S5_SYSCLK_SEL, i2s5_sysclk_sel, i2s5_sysclk_sel, i2s_sysclk_parents, ARRAY_SIZE(i2s_sysclk_parents),
		MPMU_I2S_SYSCLK_CTRL, 20, 2, 0);

CCU_DDN_DEFINE(CLK_MPMU_I2S0_SYSCLK_DIV, i2s0_sysclk_div, i2s0_sysclk_div, "i2s0_sysclk_sel", MPMU_I2S0_SYSCLK,
		CCU_DDN_MASK(0, 16), 0, CCU_DDN_MASK(16, 16), 16,
		1, 0);
CCU_DDN_DEFINE(CLK_MPMU_I2S2_SYSCLK_DIV, i2s2_sysclk_div, i2s2_sysclk_div, "i2s2_sysclk_sel", MPMU_I2S2_SYSCLK,
		CCU_DDN_MASK(0, 16), 0, CCU_DDN_MASK(16, 16), 16,
		1, 0);
CCU_DDN_DEFINE(CLK_MPMU_I2S3_SYSCLK_DIV, i2s3_sysclk_div, i2s3_sysclk_div, "i2s3_sysclk_sel", MPMU_I2S3_SYSCLK,
		CCU_DDN_MASK(0, 16), 0, CCU_DDN_MASK(16, 16), 16,
		1, 0);
CCU_DDN_DEFINE(CLK_MPMU_I2S4_SYSCLK_DIV, i2s4_sysclk_div, i2s4_sysclk_div, "i2s4_sysclk_sel", MPMU_I2S4_SYSCLK,
		CCU_DDN_MASK(0, 16), 0, CCU_DDN_MASK(16, 16), 16,
		1, 0);
CCU_DDN_DEFINE(CLK_MPMU_I2S5_SYSCLK_DIV, i2s5_sysclk_div, i2s5_sysclk_div, "i2s5_sysclk_sel", MPMU_I2S5_SYSCLK,
		CCU_DDN_MASK(0, 16), 0, CCU_DDN_MASK(16, 16), 16,
		1, 0);

static const char * const i2s2_sysclk_parents[] = {
	"i2s1_sysclk", "i2s2_sysclk_div",
};
CCU_GATE_DEFINE(CLK_MPMU_I2S0_SYSCLK, i2s0_sysclk, i2s0_sysclk, "i2s0_sysclk_div",
		MPMU_I2S_SYSCLK_CTRL, BIT(2), 0);
CCU_MUX_GATE_DEFINE(CLK_MPMU_I2S2_SYSCLK, i2s2_sysclk, i2s2_sysclk, i2s2_sysclk_parents, ARRAY_SIZE(i2s2_sysclk_parents),
		    MPMU_I2S_SYSCLK_CTRL, 8, 1, BIT(6), 0);
CCU_GATE_DEFINE(CLK_MPMU_I2S3_SYSCLK, i2s3_sysclk, i2s3_sysclk, "i2s3_sysclk_div",
		MPMU_I2S_SYSCLK_CTRL, BIT(14), 0);
CCU_GATE_DEFINE(CLK_MPMU_I2S4_SYSCLK, i2s4_sysclk, i2s4_sysclk, "i2s4_sysclk_div",
		MPMU_I2S_SYSCLK_CTRL, BIT(18), 0);
CCU_GATE_DEFINE(CLK_MPMU_I2S5_SYSCLK, i2s5_sysclk, i2s5_sysclk, "i2s5_sysclk_div",
		MPMU_I2S_SYSCLK_CTRL, BIT(22), 0);
/* MPMU clocks end */

/* APBC clocks start */
static const char * const uart_clk_parents[] = {
	"pll1_m3d128_57p6", "slow_uart1_14p74", "slow_uart2_48",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_UART0, uart0_clk, uart0_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART0_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART2, uart2_clk, uart2_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART2_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART3, uart3_clk, uart3_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART3_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART4, uart4_clk, uart4_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART4_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART5, uart5_clk, uart5_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART5_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART6, uart6_clk, uart6_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART6_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART7, uart7_clk, uart7_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART7_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART8, uart8_clk, uart8_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART8_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART9, uart9_clk, uart9_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART9_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_UART10, uart10_clk, uart10_clk, uart_clk_parents, ARRAY_SIZE(uart_clk_parents),
		    APBC_UART10_CLK_RST, 4, 3, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_UART0_BUS, uart0_bus_clk, uart0_bus_clk, "apb_clk",
		APBC_UART0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART2_BUS, uart2_bus_clk, uart2_bus_clk, "apb_clk",
		APBC_UART2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART3_BUS, uart3_bus_clk, uart3_bus_clk, "apb_clk",
		APBC_UART3_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART4_BUS, uart4_bus_clk, uart4_bus_clk, "apb_clk",
		APBC_UART4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART5_BUS, uart5_bus_clk, uart5_bus_clk, "apb_clk",
		APBC_UART5_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART6_BUS, uart6_bus_clk, uart6_bus_clk, "apb_clk",
		APBC_UART6_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART7_BUS, uart7_bus_clk, uart7_bus_clk, "apb_clk",
		APBC_UART7_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART8_BUS, uart8_bus_clk, uart8_bus_clk, "apb_clk",
		APBC_UART8_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART9_BUS, uart9_bus_clk, uart9_bus_clk, "apb_clk",
		APBC_UART9_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_UART10_BUS, uart10_bus_clk, uart10_bus_clk, "apb_clk",
		APBC_UART10_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_APBC_GPIO, gpio_clk, gpio_clk, "clock-24m",
		APBC_GPIO_CLK_RST, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_GPIO_BUS, gpio_bus_clk, gpio_bus_clk, "apb_clk",
		APBC_GPIO_CLK_RST, BIT(0), 0);

static const char * const pwm_parents[] = {
	"pll1_d192_12p8", "clock-32k",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM0, pwm0_clk, pwm0_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM0_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM1, pwm1_clk, pwm1_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM1_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM2, pwm2_clk, pwm2_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM2_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM3, pwm3_clk, pwm3_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM3_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM4, pwm4_clk, pwm4_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM4_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM5, pwm5_clk, pwm5_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM5_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM6, pwm6_clk, pwm6_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM6_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM7, pwm7_clk, pwm7_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM7_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM8, pwm8_clk, pwm8_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM8_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM9, pwm9_clk, pwm9_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM9_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM10, pwm10_clk, pwm10_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM10_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM11, pwm11_clk, pwm11_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM11_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM12, pwm12_clk, pwm12_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM12_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM13, pwm13_clk, pwm13_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM13_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM14, pwm14_clk, pwm14_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM14_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM15, pwm15_clk, pwm15_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM15_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM16, pwm16_clk, pwm16_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM16_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM17, pwm17_clk, pwm17_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM17_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM18, pwm18_clk, pwm18_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM18_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_PWM19, pwm19_clk, pwm19_clk, pwm_parents, ARRAY_SIZE(pwm_parents),
		    APBC_PWM19_CLK_RST, 4, 3, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_PWM0_BUS, pwm0_bus_clk, pwm0_bus_clk, "apb_clk",
		APBC_PWM0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM1_BUS, pwm1_bus_clk, pwm1_bus_clk, "apb_clk",
		APBC_PWM1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM2_BUS, pwm2_bus_clk, pwm2_bus_clk, "apb_clk",
		APBC_PWM2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM3_BUS, pwm3_bus_clk, pwm3_bus_clk, "apb_clk",
		APBC_PWM3_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM4_BUS, pwm4_bus_clk, pwm4_bus_clk, "apb_clk",
		APBC_PWM4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM5_BUS, pwm5_bus_clk, pwm5_bus_clk, "apb_clk",
		APBC_PWM5_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM6_BUS, pwm6_bus_clk, pwm6_bus_clk, "apb_clk",
		APBC_PWM6_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM7_BUS, pwm7_bus_clk, pwm7_bus_clk, "apb_clk",
		APBC_PWM7_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM8_BUS, pwm8_bus_clk, pwm8_bus_clk, "apb_clk",
		APBC_PWM8_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM9_BUS, pwm9_bus_clk, pwm9_bus_clk, "apb_clk",
		APBC_PWM9_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM10_BUS, pwm10_bus_clk, pwm10_bus_clk, "apb_clk",
		APBC_PWM10_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM11_BUS, pwm11_bus_clk, pwm11_bus_clk, "apb_clk",
		APBC_PWM11_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM12_BUS, pwm12_bus_clk, pwm12_bus_clk, "apb_clk",
		APBC_PWM12_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM13_BUS, pwm13_bus_clk, pwm13_bus_clk, "apb_clk",
		APBC_PWM13_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM14_BUS, pwm14_bus_clk, pwm14_bus_clk, "apb_clk",
		APBC_PWM14_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM15_BUS, pwm15_bus_clk, pwm15_bus_clk, "apb_clk",
		APBC_PWM15_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM16_BUS, pwm16_bus_clk, pwm16_bus_clk, "apb_clk",
		APBC_PWM16_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM17_BUS, pwm17_bus_clk, pwm17_bus_clk, "apb_clk",
		APBC_PWM17_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM18_BUS, pwm18_bus_clk, pwm18_bus_clk, "apb_clk",
		APBC_PWM18_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_PWM19_BUS, pwm19_bus_clk, pwm19_bus_clk, "apb_clk",
		APBC_PWM19_CLK_RST, BIT(0), 0);

static const char * const i2s_bclk_parents[] = {
	"clock-1m", "i2s_bclk",
};
CCU_MUX_DEFINE(CLK_APBC_SPI0_I2S_BCLK, spi0_i2s_bclk, spi0_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSP0_CLK_RST, 3, 1, 0);
CCU_MUX_DEFINE(CLK_APBC_SPI1_I2S_BCLK, spi1_i2s_bclk, spi1_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSP1_CLK_RST, 3, 1, 0);
CCU_MUX_DEFINE(CLK_APBC_SPI3_I2S_BCLK, spi3_i2s_bclk, spi3_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSP3_CLK_RST, 3, 1, 0);

static const char * const spi0_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "spi0_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_SPI0, spi0_clk, spi0_clk, spi0_parents, ARRAY_SIZE(spi0_parents),
		    APBC_SSP0_CLK_RST, 4, 3, BIT(1), 0);

static const char * const spi1_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "spi1_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_SPI1, spi1_clk, spi1_clk, spi1_parents, ARRAY_SIZE(spi1_parents),
		    APBC_SSP1_CLK_RST, 4, 3, BIT(1), 0);

static const char * const spi3_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "spi3_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_SPI3, spi3_clk, spi3_clk, spi3_parents, ARRAY_SIZE(spi3_parents),
		    APBC_SSP3_CLK_RST, 4, 3, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_SPI0_BUS, spi0_bus_clk, spi0_bus_clk, "apb_clk",
		APBC_SSP0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_SPI1_BUS, spi1_bus_clk, spi1_bus_clk, "apb_clk",
		APBC_SSP1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_SPI3_BUS, spi3_bus_clk, spi3_bus_clk, "apb_clk",
		APBC_SSP3_CLK_RST, BIT(0), 0);


CCU_GATE_DEFINE(CLK_APBC_RTC, rtc_clk, rtc_clk, "clock-32k",
		APBC_RTC_CLK_RST, BIT(7) | BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_RTC_BUS, rtc_bus_clk, rtc_bus_clk, "apb_clk",
		APBC_RTC_CLK_RST, BIT(0), 0);

static const char * const twsi_parents[] = {
	"pll1_d78_31p5", "pll1_d48_51p2", "pll1_d40_61p44",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_TWSI0, twsi0_clk, twsi0_clk, twsi_parents, ARRAY_SIZE(twsi_parents),
		    APBC_TWSI0_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TWSI1, twsi1_clk, twsi1_clk, twsi_parents, ARRAY_SIZE(twsi_parents),
		    APBC_TWSI1_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TWSI2, twsi2_clk, twsi2_clk, twsi_parents, ARRAY_SIZE(twsi_parents),
		    APBC_TWSI2_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TWSI4, twsi4_clk, twsi4_clk, twsi_parents, ARRAY_SIZE(twsi_parents),
		    APBC_TWSI4_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TWSI5, twsi5_clk, twsi5_clk, twsi_parents, ARRAY_SIZE(twsi_parents),
		    APBC_TWSI5_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TWSI6, twsi6_clk, twsi6_clk, twsi_parents, ARRAY_SIZE(twsi_parents),
		    APBC_TWSI6_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TWSI8, twsi8_clk, twsi8_clk, twsi_parents, ARRAY_SIZE(twsi_parents),
		    APBC_TWSI8_CLK_RST, 4, 3, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_TWSI0_BUS, twsi0_bus_clk, twsi0_bus_clk, "apb_clk",
		APBC_TWSI0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TWSI1_BUS, twsi1_bus_clk, twsi1_bus_clk, "apb_clk",
		APBC_TWSI1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TWSI2_BUS, twsi2_bus_clk, twsi2_bus_clk, "apb_clk",
		APBC_TWSI2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TWSI4_BUS, twsi4_bus_clk, twsi4_bus_clk, "apb_clk",
		APBC_TWSI4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TWSI5_BUS, twsi5_bus_clk, twsi5_bus_clk, "apb_clk",
		APBC_TWSI5_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TWSI6_BUS, twsi6_bus_clk, twsi6_bus_clk, "apb_clk",
		APBC_TWSI6_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TWSI8_BUS, twsi8_bus_clk, twsi8_bus_clk, "apb_clk",
		APBC_TWSI8_CLK_RST, BIT(0), 0);

static const char * const timer_parents[] = {
	"pll1_d192_12p8", "clock-32k", "pll1_d384_6p4", "clock-3m", "clock-1m",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS0, timers0_clk, timers0_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS0_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS1, timers1_clk, timers1_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS1_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS2, timers2_clk, timers2_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS2_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS3, timers3_clk, timers3_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS3_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS4, timers4_clk, timers4_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS4_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS5, timers5_clk, timers5_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS5_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS6, timers6_clk, timers6_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS6_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_TIMERS7, timers7_clk, timers7_clk, timer_parents, ARRAY_SIZE(timer_parents),
		    APBC_TIMERS7_CLK_RST, 4, 3, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_TIMERS0_BUS, timers0_bus_clk, timers0_bus_clk, "apb_clk",
		APBC_TIMERS0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TIMERS1_BUS, timers1_bus_clk, timers1_bus_clk, "apb_clk",
		APBC_TIMERS1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TIMERS2_BUS, timers2_bus_clk, timers2_bus_clk, "apb_clk",
		APBC_TIMERS2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TIMERS3_BUS, timers3_bus_clk, timers3_bus_clk, "apb_clk",
		APBC_TIMERS3_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TIMERS4_BUS, timers4_bus_clk, timers4_bus_clk, "apb_clk",
		APBC_TIMERS4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TIMERS5_BUS, timers5_bus_clk, timers5_bus_clk, "apb_clk",
		APBC_TIMERS5_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TIMERS6_BUS, timers6_bus_clk, timers6_bus_clk, "apb_clk",
		APBC_TIMERS6_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_TIMERS7_BUS, timers7_bus_clk, timers7_bus_clk, "apb_clk",
		APBC_TIMERS7_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_APBC_AIB, aib_clk, aib_clk, "clock-24m",
		APBC_AIB_CLK_RST, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_AIB_BUS, aib_bus_clk, aib_bus_clk, "apb_clk",
		APBC_AIB_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_APBC_ONEWIRE, onewire_clk, onewire_clk, "clock-24m",
		APBC_ONEWIRE_CLK_RST, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_ONEWIRE_BUS, onewire_bus_clk, onewire_bus_clk, "apb_clk",
		APBC_ONEWIRE_CLK_RST, BIT(0), 0);

/*
 * When i2s_bclk is selected as the parent clock of sspa,
 * the hardware requires bit3 to be set
 */

CCU_MUX_DEFINE(CLK_APBC_I2S0_BCLK, i2s0_i2s_bclk, i2s0_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSPA0_CLK_RST, 3, 1, 0);
CCU_MUX_DEFINE(CLK_APBC_I2S1_BCLK, i2s1_i2s_bclk, i2s1_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSPA1_CLK_RST, 3, 1, 0);
CCU_MUX_DEFINE(CLK_APBC_I2S2_BCLK, i2s2_i2s_bclk, i2s2_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSPA2_CLK_RST, 3, 1, 0);
CCU_MUX_DEFINE(CLK_APBC_I2S3_BCLK, i2s3_i2s_bclk, i2s3_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSPA3_CLK_RST, 3, 1, 0);
CCU_MUX_DEFINE(CLK_APBC_I2S4_BCLK, i2s4_i2s_bclk, i2s4_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSPA4_CLK_RST, 3, 1, 0);
CCU_MUX_DEFINE(CLK_APBC_I2S5_BCLK, i2s5_i2s_bclk, i2s5_i2s_bclk, i2s_bclk_parents, ARRAY_SIZE(i2s_bclk_parents),
		APBC_SSPA5_CLK_RST, 3, 1, 0);

static const char * const i2s0_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "i2s0_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_I2S0, i2s0_clk, i2s0_clk, i2s0_parents, ARRAY_SIZE(i2s0_parents),
		    APBC_SSPA0_CLK_RST, 4, 3, BIT(1), 0);

static const char * const i2s1_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "i2s1_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_I2S1, i2s1_clk, i2s1_clk, i2s1_parents, ARRAY_SIZE(i2s1_parents),
		    APBC_SSPA1_CLK_RST, 4, 3, BIT(1), 0);

static const char * const i2s2_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "i2s2_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_I2S2, i2s2_clk, i2s2_clk, i2s2_parents, ARRAY_SIZE(i2s2_parents),
		    APBC_SSPA2_CLK_RST, 4, 3, BIT(1), 0);

static const char * const i2s3_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "i2s3_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_I2S3, i2s3_clk, i2s3_clk, i2s3_parents, ARRAY_SIZE(i2s3_parents),
		    APBC_SSPA3_CLK_RST, 4, 3, BIT(1), 0);

static const char * const i2s4_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "i2s4_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_I2S4, i2s4_clk, i2s4_clk, i2s4_parents, ARRAY_SIZE(i2s4_parents),
		    APBC_SSPA4_CLK_RST, 4, 3, BIT(1), 0);

static const char * const i2s5_parents[] = {
	"pll1_d384_6p4", "pll1_d192_12p8", "pll1_d96_25p6", "pll1_d48_51p2", "pll1_d768_3p2", "pll1_d1536_1p6", "pll1_d3072_0p8", "i2s5_i2s_bclk",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_I2S5, i2s5_clk, i2s5_clk, i2s5_parents, ARRAY_SIZE(i2s5_parents),
		    APBC_SSPA5_CLK_RST, 4, 3, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_I2S0_BUS, i2s0_bus_clk, i2s0_bus_clk, "apb_clk",
		APBC_SSPA0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_I2S1_BUS, i2s1_bus_clk, i2s1_bus_clk, "apb_clk",
		APBC_SSPA1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_I2S2_BUS, i2s2_bus_clk, i2s2_bus_clk, "apb_clk",
		APBC_SSPA2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_I2S3_BUS, i2s3_bus_clk, i2s3_bus_clk, "apb_clk",
		APBC_SSPA3_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_I2S4_BUS, i2s4_bus_clk, i2s4_bus_clk, "apb_clk",
		APBC_SSPA4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_I2S5_BUS, i2s5_bus_clk, i2s5_bus_clk, "apb_clk",
		APBC_SSPA5_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_APBC_DRO, dro_clk, dro_clk, "apb_clk",
		APBC_DRO_CLK_RST, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_IR0, ir0_clk, ir0_clk, "apb_clk",
		APBC_IR0_CLK_RST, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_IR1, ir1_clk, ir1_clk, "apb_clk",
		APBC_IR1_CLK_RST, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_TSEN, tsen_clk, tsen_clk, "apb_clk",
		APBC_TSEN_CLK_RST, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_TSEN_BUS, tsen_bus_clk, tsen_bus_clk, "apb_clk",
		APBC_TSEN_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_APBC_IPC_AP2RCPU, ipc_ap2rcpu_clk, ipc_ap2rcpu_clk, "apb_clk",
		APBC_IPC_AP2AUD_CLK_RST, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APBC_IPC_AP2RCPU_BUS, ipc_ap2rcpu_bus_clk, ipc_ap2rcpu_bus_clk, "apb_clk",
		APBC_IPC_AP2AUD_CLK_RST, BIT(0), 0);

static const char * const can_parents[] = {
	"pll6_20", "pll6_40", "pll6_80",
};
CCU_MUX_GATE_DEFINE(CLK_APBC_CAN0, can0_clk, can0_clk, can_parents, ARRAY_SIZE(can_parents),
		    APBC_CAN0_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_CAN1, can1_clk, can1_clk, can_parents, ARRAY_SIZE(can_parents),
		    APBC_CAN1_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_CAN2, can2_clk, can2_clk, can_parents, ARRAY_SIZE(can_parents),
		    APBC_CAN2_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_CAN3, can3_clk, can3_clk, can_parents, ARRAY_SIZE(can_parents),
		    APBC_CAN3_CLK_RST, 4, 3, BIT(1), 0);
CCU_MUX_GATE_DEFINE(CLK_APBC_CAN4, can4_clk, can4_clk, can_parents, ARRAY_SIZE(can_parents),
		    APBC_CAN4_CLK_RST, 4, 3, BIT(1), 0);

CCU_GATE_DEFINE(CLK_APBC_CAN0_BUS, can0_bus_clk, can0_bus_clk, "apb_clk",
		APBC_CAN0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_CAN1_BUS, can1_bus_clk, can1_bus_clk, "apb_clk",
		APBC_CAN1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_CAN2_BUS, can2_bus_clk, can2_bus_clk, "apb_clk",
		APBC_CAN2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_CAN3_BUS, can3_bus_clk, can3_bus_clk, "apb_clk",
		APBC_CAN3_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APBC_CAN4_BUS, can4_bus_clk, can4_bus_clk, "apb_clk",
		APBC_CAN4_CLK_RST, BIT(0), 0);
/* APBC clocks end */

/* APMU clocks start */
static const char * const axi_clk_parents[] = {
	"pll1_d8_307p2", "pll1_d6_409p6",
};
CCU_MUX_DIV_FC_DEFINE(CLK_APMU_AXICLK, axi_clk, axi_clk, axi_clk_parents, ARRAY_SIZE(axi_clk_parents),
		      APMU_ACLK_CLK_CTRL, APMU_ACLK_CLK_CTRL, 1, 2, BIT(4),
		      0, 1, 0);

static const char * const cci550_clk_parents[] = {
	"pll1_d10_245p76", "pll1_d6_409p6", "pll1_d4_614p4", "pll1_d3_819p2", "pll7_d3", "pll2_d3", "pll1_d2_1228p8", "pll7_d2",
};
CCU_MUX_DIV_FC_DEFINE(CLK_APMU_CCI550, cci550_clk, cci550_clk, cci550_clk_parents, ARRAY_SIZE(cci550_clk_parents),
		      APMU_CCI550_CLK_CTRL, APMU_CCI550_CLK_CTRL, 8, 2, BIT(12),
		      0, 3, CLK_IS_CRITICAL);

static const char * const cpu_c0_clk_parents[] = {
	"pll1_d3_819p2", "pll1_d5_491p52", "pll1_d4_614p4", "pll2_d3", "pll3_d2", "pll1_d2_1228p8", "pll2_d2", "pll3_d1",
};
CCU_MUX_DIV_FC_DEFINE(CLK_APMU_CPU_C0_CORE, cpu_c0_core_clk, cpu_c0_core_clk, cpu_c0_clk_parents, ARRAY_SIZE(cpu_c0_clk_parents),
		      APMU_CPU_C0_CLK_CTRL, APMU_CPU_C0_CLK_CTRL, 3, 3, BIT(12),
		      0, 3, CLK_IS_CRITICAL);

static const char * const cpu_c1_clk_parents[] = {
	"pll1_d3_819p2", "pll1_d5_491p52", "pll1_d4_614p4", "pll2_d3", "pll4_d2", "pll1_d2_1228p8", "pll2_d2", "pll4_d1",
};
CCU_MUX_DIV_FC_DEFINE(CLK_APMU_CPU_C1_CORE, cpu_c1_core_clk, cpu_c1_core_clk, cpu_c1_clk_parents, ARRAY_SIZE(cpu_c1_clk_parents),
		      APMU_CPU_C1_CLK_CTRL, APMU_CPU_C1_CLK_CTRL, 3, 3, BIT(12),
		      0, 3, CLK_IS_CRITICAL);

static const char * const cpu_c2_clk_parents[] = {
	"pll1_d3_819p2", "pll1_d5_491p52", "pll1_d4_614p4", "pll2_d3", "pll5_d2", "pll1_d2_1228p8", "pll2_d2", "pll5_d1",
};
CCU_MUX_DIV_FC_DEFINE(CLK_APMU_CPU_C2_CORE, cpu_c2_core_clk, cpu_c2_core_clk, cpu_c2_clk_parents, ARRAY_SIZE(cpu_c2_clk_parents),
		      APMU_CPU_C2_CLK_CTRL, APMU_CPU_C2_CLK_CTRL, 3, 3, BIT(12),
		      0, 3, CLK_IS_CRITICAL);

static const char * const cpu_c3_clk_parents[] = {
	"pll1_d3_819p2", "pll1_d5_491p52", "pll1_d4_614p4", "pll2_d3", "pll8_d2", "pll1_d2_1228p8", "pll2_d2", "pll8_d1",
};
CCU_MUX_DIV_FC_DEFINE(CLK_APMU_CPU_C3_CORE, cpu_c3_core_clk, cpu_c3_core_clk, cpu_c3_clk_parents, ARRAY_SIZE(cpu_c3_clk_parents),
		      APMU_CPU_C3_CLK_CTRL, APMU_CPU_C3_CLK_CTRL, 3, 3, BIT(12),
		      0, 3, CLK_IS_CRITICAL);

static const char * const ccic2phy_parents[] = {
	"pll1_d24_102p4", "pll1_d48_51p2_ap",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_CCIC2PHY, ccic2phy_clk, ccic2phy_clk, ccic2phy_parents, ARRAY_SIZE(ccic2phy_parents),
		    APMU_CSI_CCIC2_CLK_RES_CTRL, 7, 1, BIT(5), 0);

static const char * const ccic3phy_parents[] = {
	"pll1_d24_102p4", "pll1_d48_51p2_ap",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_CCIC3PHY, ccic3phy_clk, ccic3phy_clk, ccic3phy_parents, ARRAY_SIZE(ccic3phy_parents),
		    APMU_CSI_CCIC2_CLK_RES_CTRL, 31, 1, BIT(30), 0);

static const char * const csi_parents[] = {
	"pll1_d5_491p52", "pll1_d6_409p6", "pll1_d4_614p4", "pll1_d3_819p2", "pll2_d2", "pll2_d3", "pll2_d4", "pll1_d2_1228p8",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_CSI, csi_clk, csi_clk, csi_parents, ARRAY_SIZE(csi_parents),
			   APMU_CSI_CCIC2_CLK_RES_CTRL, 20, 3, BIT(15),
			   16, 3, BIT(4), 0);

static const char * const isp_bus_parents[] = {
	"pll1_d6_409p6", "pll1_d5_491p52", "pll1_d4_614p4", "pll1_d10_245p76",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_ISP_BUS, isp_bus_clk, isp_bus_clk, isp_bus_parents, ARRAY_SIZE(isp_bus_parents),
			   APMU_ISP_CLK_RES_CTRL, 18, 3, BIT(23),
			   21, 2, BIT(17), 0);

CCU_GATE_DEFINE(CLK_APMU_D1P_1228P8, d1p_1228p8, d1p_1228p8, "pll1_d2_1228p8",
		APMU_PMU_CLK_GATE_CTRL, BIT(31), 0);
CCU_GATE_DEFINE(CLK_APMU_D1P_819P2, d1p_819p2, d1p_819p2, "pll1_d3_819p2",
		APMU_PMU_CLK_GATE_CTRL, BIT(30), 0);
CCU_GATE_DEFINE(CLK_APMU_D1P_614P4, d1p_614p4, d1p_614p4, "pll1_d4_614p4",
		APMU_PMU_CLK_GATE_CTRL, BIT(29), 0);
CCU_GATE_DEFINE(CLK_APMU_D1P_491P52, d1p_491p52, d1p_491p52, "pll1_d5_491p52",
		APMU_PMU_CLK_GATE_CTRL, BIT(28), 0);
CCU_GATE_DEFINE(CLK_APMU_D1P_409P6, d1p_409p6, d1p_409p6, "pll1_d6_409p6",
		APMU_PMU_CLK_GATE_CTRL, BIT(27), 0);
CCU_GATE_DEFINE(CLK_APMU_D1P_307P2, d1p_307p2, d1p_307p2, "pll1_d8_307p2",
		APMU_PMU_CLK_GATE_CTRL, BIT(26), 0);
CCU_GATE_DEFINE(CLK_APMU_D1P_245P76, d1p_245p76, d1p_245p76, "pll1_d10_245p76",
		APMU_PMU_CLK_GATE_CTRL, BIT(22), 0);

static const char * const v2d_parents[] = {
	"pll1_d5_491p52", "pll2_d4", "pll1_d8_307p2", "pll1_d4_614p4",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_V2D, v2d_clk, v2d_clk, v2d_parents, ARRAY_SIZE(v2d_parents),
			   APMU_LCD_CLK_RES_CTRL1, 9, 3, BIT(28),
			   12, 2, BIT(8), 0);

static const char * const dsiesc_parents[] = {
	"pll1_d48_51p2_ap", "pll1_d52_47p26", "pll1_d96_25p6", "pll1_d32_76p8",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_DSI_ESC, dsi_esc_clk, dsi_esc_clk, dsiesc_parents, ARRAY_SIZE(dsiesc_parents),
		    APMU_LCD_CLK_RES_CTRL1, 0, 2, BIT(2), 0);

CCU_GATE_DEFINE(CLK_APMU_LCD_HCLK, lcd_hclk, lcd_hclk, "axi_clk",
		APMU_LCD_CLK_RES_CTRL1, BIT(5), 0);

static const char * const lcd_dsc_parents[] = {
	"pll1_d4_614p4", "pll1_d5_491p52", "pll1_d10_245p76", "pll7_d5", "pll2_d7", "pll1_d6_409p6", "pll1_d48_51p2_ap", "pll2_d8",
};
CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_APMU_LCD_DSC, lcd_dsc_clk, lcd_dsc_clk, lcd_dsc_parents, ARRAY_SIZE(lcd_dsc_parents),
				 APMU_LCD_CLK_RES_CTRL2, APMU_LCD_CLK_RES_CTRL1, 25, 3, BIT(26),
				 29, 3, BIT(14), 0);

static const char * const lcdpx_parents[] = {
	"pll1_d4_614p4", "pll1_d5_491p52", "pll1_d10_245p76", "pll7_d5", "pll2_d7", "pll2_d4", "pll1_d48_51p2_ap", "pll2_d8",
};
CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_APMU_LCD_PXCLK, lcd_pxclk, lcd_pxclk, lcdpx_parents, ARRAY_SIZE(lcdpx_parents),
				 APMU_LCD_CLK_RES_CTRL2, APMU_LCD_CLK_RES_CTRL1, 17, 3, BIT(30),
				 21, 3, BIT(16), 0);

static const char * const lcdmclk_parents[] = {
	"pll1_d6_409p6", "pll1_d5_491p52", "pll1_d4_614p4", "pll1_d8_307p2",
};
CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_APMU_LCD_MCLK, lcd_mclk, lcd_mclk, lcdmclk_parents, ARRAY_SIZE(lcdmclk_parents),
				 APMU_LCD_CLK_RES_CTRL2, APMU_LCD_CLK_RES_CTRL1, 1, 4, BIT(29),
				 5, 3, BIT(0), 0);

static const char * const ccic_4x_parents[] = {
	"pll1_d5_491p52", "pll1_d6_409p6", "pll1_d4_614p4", "pll1_d3_819p2", "pll2_d2", "pll2_d3", "pll2_d4", "pll1_d2_1228p8",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_CCIC_4X, ccic_4x_clk, ccic_4x_clk, ccic_4x_parents, ARRAY_SIZE(ccic_4x_parents),
			   APMU_CCIC_CLK_RES_CTRL, 18, 3, BIT(15),
			   23, 2, BIT(4), 0);

static const char * const ccic1phy_parents[] = {
	"pll1_d24_102p4", "pll1_d48_51p2_ap",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_CCIC1PHY, ccic1phy_clk, ccic1phy_clk, ccic1phy_parents, ARRAY_SIZE(ccic1phy_parents),
		    APMU_CCIC_CLK_RES_CTRL, 7, 1, BIT(5), 0);


static const char * const sc2hclk_parents[] = {
	"pll1_d8_307p2", "pll1_d4_614p4", "pll1_d5_491p52", "pll2_d4",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_SC2_HCLK, sc2_hclk, sc2_hclk, sc2hclk_parents, ARRAY_SIZE(sc2hclk_parents),
			   APMU_CCIC_CLK_RES_CTRL, 10, 3, BIT(16),
			   8, 2, BIT(3), 0);

CCU_GATE_DEFINE(CLK_APMU_SDH_AXI, sdh_axi_aclk, sdh_axi_aclk, "axi_clk",
		APMU_SDH0_CLK_RES_CTRL, BIT(3), 0);
static const char * const sdh01_parents[] = {
	"pll1_d6_409p6", "pll1_d4_614p4", "pll2_d8", "pll2_d5", "reserved_clk", "reserved_clk", "pll1_dx",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_SDH0, sdh0_clk, sdh0_clk, sdh01_parents, ARRAY_SIZE(sdh01_parents),
			   APMU_SDH0_CLK_RES_CTRL, 8, 3, BIT(11),
			   5, 3, BIT(4), 0);
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_SDH1, sdh1_clk, sdh1_clk, sdh01_parents, ARRAY_SIZE(sdh01_parents),
			   APMU_SDH1_CLK_RES_CTRL, 8, 3, BIT(11),
			   5, 3, BIT(4), 0);
static const char * const sdh2_parents[] = {
	"pll1_d6_409p6", "pll1_d4_614p4", "pll2_d8", "pll1_d3_819p2", "reserved_clk", "reserved_clk", "pll1_dx",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_SDH2, sdh2_clk, sdh2_clk, sdh2_parents, ARRAY_SIZE(sdh2_parents),
			   APMU_SDH2_CLK_RES_CTRL, 8, 3, BIT(11),
			   5, 3, BIT(4), 0);

CCU_GATE_DEFINE(CLK_APMU_USB2_BUS, usb2_bus_clk, usb2_bus_clk, "axi_clk",
		APMU_USB_CLK_RES_CTRL, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APMU_USB3_PORTA_BUS, usb3_porta_bus_clk, usb3_porta_bus_clk, "axi_clk",
		APMU_USB_CLK_RES_CTRL, BIT(4), 0);
CCU_GATE_DEFINE(CLK_APMU_USB3_PORTB_BUS, usb3_portb_bus_clk, usb3_portb_bus_clk, "axi_clk",
		APMU_USB_CLK_RES_CTRL, BIT(8), 0);
CCU_GATE_DEFINE(CLK_APMU_USB3_PORTC_BUS, usb3_portc_bus_clk, usb3_portc_bus_clk, "axi_clk",
		APMU_USB_CLK_RES_CTRL, BIT(12), 0);
CCU_GATE_DEFINE(CLK_APMU_USB3_PORTD_BUS, usb3_portd_bus_clk, usb3_portd_bus_clk, "axi_clk",
		APMU_USB_CLK_RES_CTRL, BIT(16), 0);

static const char * const qspi_parents[] = {
	"pll1_d6_409p6", "pll2_d8", "pll1_d8_307p2", "pll1_d10_245p76", "reserved_clk", "pll1_dx", "pll1_d5_491p52", "reserved_clk",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_QSPI, qspi_clk, qspi_clk, qspi_parents, ARRAY_SIZE(qspi_parents),
			   APMU_QSPI_CLK_RES_CTRL, 9, 3, BIT(12),
			   6, 3, BIT(4), 0);
CCU_GATE_DEFINE(CLK_APMU_QSPI_BUS, qspi_bus_clk, qspi_bus_clk, "axi_clk",
		APMU_QSPI_CLK_RES_CTRL, BIT(3), 0);

CCU_GATE_DEFINE(CLK_APMU_DMA, dma_clk, dma_clk, "axi_clk",
		APMU_DMA_CLK_RES_CTRL, BIT(3), 0);

static const char * const aes_wtm_parents[] = {
	"pll1_d12_204p8", "pll1_d24_102p4",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_AES_WTM, aes_wtm_clk, aes_wtm_clk, aes_wtm_parents, ARRAY_SIZE(aes_wtm_parents),
		    APMU_AES_CLK_RES_CTRL, 6, 1, BIT(5), 0);

static const char * const vpu_parents[] = {
	"pll1_d4_614p4", "pll1_d5_491p52", "pll1_d3_819p2", "pll1_d6_409p6", "pll1_d2_1228p8", "pll2_d3", "pll2_d4", "pll2_d5",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_VPU, vpu_clk, vpu_clk, vpu_parents, ARRAY_SIZE(vpu_parents),
			   APMU_VPU_CLK_RES_CTRL, 13, 3, BIT(21),
			   10, 3, BIT(3), 0);

CCU_GATE_DEFINE(CLK_APMU_DTC, dtc_clk, dtc_clk, "axi_clk",
		APMU_DTC_CLK_RES_CTRL, BIT(3), 0);

static const char * const gpu_parents[] = {
	"pll1_d4_614p4", "pll1_d5_491p52", "pll1_d3_819p2", "pll1_d6_409p6", "pll1_d2_1228p8", "pll2_d3", "pll2_d4", "pll2_d5",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_GPU, gpu_clk, gpu_clk, gpu_parents, ARRAY_SIZE(gpu_parents),
			   APMU_GPU_CLK_RES_CTRL, 12, 3, BIT(15),
			   18, 3, BIT(4), 0);

CCU_GATE_DEFINE(CLK_APMU_MC_AHB, mc_ahb_clk, mc_ahb_clk, "axi_clk",
		APMU_PMUA_MC_CTRL, BIT(1), 0);

static const char * const top_parents[] = {
	"pll1_d8_307p2", "pll1_d6_409p6", "pll3_d4", "pll6_d5", "pll7_d4", "pll6_d4", "pll7_d3", "pll6_d3",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_TOP_DCLK, top_dclk, top_dclk, top_parents, ARRAY_SIZE(top_parents),
			   APMU_TOP_DCLK_CTRL, 5, 3, BIT(8),
			   2, 3, BIT(1), CLK_IS_CRITICAL);

static const char * const ucie_parents[] = {
	"pll1_d8_307p2", "pll1_d6_409p6", "pll3_d4", "pll6_d5", "pll7_d4", "pll6_d4",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_UCIE, ucie_clk, ucie_clk, ucie_parents, ARRAY_SIZE(ucie_parents),
		    APMU_UCIE_CTRL, 4, 3, BIT(0), 0);
CCU_GATE_DEFINE(CLK_APMU_UCIE_SBCLK, ucie_sbclk, ucie_sbclk, "axi_clk",
		APMU_UCIE_CTRL, BIT(8), 0);

static const char * const rcpu_clk_parents[] = {
	"pll1_aud_245p7", "pll1_d8_307p2", "pll1_d5_491p52", "pll1_d6_409p6",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_RCPU, rcpu_clk, rcpu_clk, rcpu_clk_parents, ARRAY_SIZE(rcpu_clk_parents),
			   APMU_RCPU_CLK_RES_CTRL, 4, 3, BIT(15),
			   7, 3, BIT(12), 0);

static const char * const dsi4ln2_dsi_esc_parents[] = {
	"pll1_d48_51p2_ap", "pll1_d52_47p26", "pll1_d96_25p6", "pll1_d32_76p8",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_DSI4LN2_DSI_ESC, dsi4ln2_dsi_esc_clk, dsi4ln2_dsi_esc_clk, dsi4ln2_dsi_esc_parents, ARRAY_SIZE(dsi4ln2_dsi_esc_parents),
		    APMU_LCD_CLK_RES_CTRL3, 0, 1, BIT(2), 0);

static const char * const dsi4ln2_lcd_dsc_parents[] = {
	"pll1_d4_614p4", "pll1_d5_491p52", "pll7_d5", "pll6_d6", "pll2_d7", "pll1_d6_409p6", "pll1_d48_51p2_ap",
};
CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_APMU_DSI4LN2_LCD_DSC, dsi4ln2_lcd_dsc_clk, dsi4ln2_lcd_dsc_clk, dsi4ln2_lcd_dsc_parents, ARRAY_SIZE(dsi4ln2_lcd_dsc_parents),
				 APMU_LCD_CLK_RES_CTRL4, APMU_LCD_CLK_RES_CTRL3, 25, 3, BIT(26),
				 29, 3, BIT(14), 0);

static const char * const dsi4ln2_lcdpx_parents[] = {
	"pll1_d4_614p4", "pll1_d5_491p52", "pll7_d5", "pll6_d6", "pll2_d7", "pll2_d4", "pll1_d48_51p2_ap", "pll2_d8",
};
CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_APMU_DSI4LN2_LCD_PXCLK, dsi4ln2_lcd_pxclk, dsi4ln2_lcd_pxclk, dsi4ln2_lcdpx_parents, ARRAY_SIZE(dsi4ln2_lcdpx_parents),
				 APMU_LCD_CLK_RES_CTRL4, APMU_LCD_CLK_RES_CTRL3, 17, 3, BIT(30),
				 21, 3, BIT(16), 0);

static const char * const dsi4ln2_lcd_mclk_parents[] = {
	"pll1_d6_409p6", "pll1_d5_491p52", "pll1_d4_614p4", "pll1_d8_307p2",
};
CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_APMU_DSI4LN2_LCD_MCLK, dsi4ln2_lcd_mclk, dsi4ln2_lcd_mclk, dsi4ln2_lcd_mclk_parents, ARRAY_SIZE(dsi4ln2_lcd_mclk_parents),
				 APMU_LCD_CLK_RES_CTRL4, APMU_LCD_CLK_RES_CTRL3, 1, 4, BIT(29),
				 5, 3, BIT(0), 0);

static const char * const dpu_aclk_parents[] = {
	"pll1_d6_409p6", "pll1_d5_491p52", "pll1_d4_614p4", "pll1_d8_307p2", "pll2_d4",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_DSI4LN2_DPU_ACLK, dsi4ln2_dpu_aclk, dsi4ln2_dpu_aclk, dpu_aclk_parents, ARRAY_SIZE(dpu_aclk_parents),
			   APMU_LCD_CLK_RES_CTRL5, 2, 3, BIT(30),
			   5, 3, BIT(1), 0);

CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_DPU_ACLK, dpu_aclk, dpu_aclk, dpu_aclk_parents, ARRAY_SIZE(dpu_aclk_parents),
			   APMU_LCD_CLK_RES_CTRL5, 17, 3, BIT(31),
			   20, 3, BIT(16), 0);

static const char * const ufs_aclk_parents[] = {
	"pll1_d5_491p52", "pll1_d6_409p6", "pll2_d6", "pll2_d5",
};
CCU_MUX_DIV_GATE_FC_DEFINE(CLK_APMU_UFS_ACLK, ufs_aclk, ufs_aclk, ufs_aclk_parents, ARRAY_SIZE(ufs_aclk_parents),
			   APMU_UFS_CLK_RES_CTRL, 5, 3, BIT(8),
			   2, 3, BIT(1), 0);

static const char * const edp0_pclk_parents[] = {
	"lcd_pxclk", "external_clk",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_EDP0_PXCLK, edp0_pxclk, edp0_pxclk, edp0_pclk_parents, ARRAY_SIZE(edp0_pclk_parents),
		    APMU_LCD_EDP_CTRL, 2, 1, BIT(1), 0);

static const char * const edp1_pclk_parents[] = {
	"dsi4ln2_lcd_pxclk", "external_clk",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_EDP1_PXCLK, edp1_pxclk, edp1_pxclk, edp1_pclk_parents, ARRAY_SIZE(edp1_pclk_parents),
		    APMU_LCD_EDP_CTRL, 18, 1, BIT(17), 0);

CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTA_MSTE, pciea_mstr_clk, pciea_mstr_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_A, BIT(2), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTA_SLV, pciea_slv_clk, pciea_slv_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_A, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTB_MSTE, pcieb_mstr_clk, pcieb_mstr_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_B, BIT(2), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTB_SLV, pcieb_slv_clk, pcieb_slv_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_B, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTC_MSTE, pciec_mstr_clk, pciec_mstr_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_C, BIT(2), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTC_SLV, pciec_slv_clk, pciec_slv_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_C, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTD_MSTE, pcied_mstr_clk, pcied_mstr_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_D, BIT(2), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTD_SLV, pcied_slv_clk, pcied_slv_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_D, BIT(1), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTE_MSTE, pciee_mstr_clk, pciee_mstr_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_E, BIT(2), 0);
CCU_GATE_DEFINE(CLK_APMU_PCIE_PORTE_SLV, pciee_slv_clk, pciee_slv_clk, "pll2_d6",
		APMU_PCIE_CLK_RES_CTRL_E, BIT(1), 0);

static const char * const emac_1588_parents[] = {
	"clock-24m", "pll2_d24_125",
};
CCU_GATE_DEFINE(CLK_APMU_EMAC0_BUS, emac0_bus_clk, emac0_bus_clk, "axi_clk",
		APMU_EMAC0_CLK_RES_CTRL, BIT(0), 0);
CCU_GATE_FLAGS_DEFINE(CLK_APMU_EMAC0_REF, emac0_ref_clk, emac0_ref_clk, "pll2_d120_25",
		      APMU_EMAC0_CLK_RES_CTRL, BIT(14), true, 0);
CCU_MUX_DEFINE(CLK_APMU_EMAC0_1588, emac0_1588_clk, emac0_1588_clk, emac_1588_parents, ARRAY_SIZE(emac_1588_parents),
		APMU_EMAC0_CLK_RES_CTRL, 15, 1, 0);
CCU_GATE_DEFINE(CLK_APMU_EMAC0_RGMII_TX, emac0_rgmii_tx_clk, emac0_rgmii_tx_clk, "pll2_d24_125",
		APMU_EMAC0_CLK_RES_CTRL, BIT(8), 0);
CCU_GATE_DEFINE(CLK_APMU_EMAC1_BUS, emac1_bus_clk, emac1_bus_clk, "axi_clk",
		APMU_EMAC1_CLK_RES_CTRL, BIT(0), 0);
CCU_GATE_FLAGS_DEFINE(CLK_APMU_EMAC1_REF, emac1_ref_clk, emac1_ref_clk, "pll2_d120_25",
		      APMU_EMAC1_CLK_RES_CTRL, BIT(14), true, 0);
CCU_MUX_DEFINE(CLK_APMU_EMAC1_1588, emac1_1588_clk, emac1_1588_clk, emac_1588_parents, ARRAY_SIZE(emac_1588_parents),
		APMU_EMAC1_CLK_RES_CTRL, 15, 1, 0);
CCU_GATE_DEFINE(CLK_APMU_EMAC1_RGMII_TX, emac1_rgmii_tx_clk, emac1_rgmii_tx_clk, "pll2_d24_125",
		APMU_EMAC1_CLK_RES_CTRL, BIT(8), 0);
CCU_GATE_DEFINE(CLK_APMU_EMAC2_BUS, emac2_bus_clk, emac2_bus_clk, "axi_clk",
		APMU_EMAC2_CLK_RES_CTRL, BIT(0), 0);
CCU_GATE_FLAGS_DEFINE(CLK_APMU_EMAC2_REF, emac2_ref_clk, emac2_ref_clk, "pll2_d120_25",
		      APMU_EMAC2_CLK_RES_CTRL, BIT(14), true, 0);
CCU_MUX_DEFINE(CLK_APMU_EMAC2_1588, emac2_1588_clk, emac2_1588_clk, emac_1588_parents, ARRAY_SIZE(emac_1588_parents),
		APMU_EMAC2_CLK_RES_CTRL, 15, 1, 0);
CCU_GATE_DEFINE(CLK_APMU_EMAC2_RGMII_TX, emac2_rgmii_tx_clk, emac2_rgmii_tx_clk, "pll2_d24_125",
		APMU_EMAC2_CLK_RES_CTRL, BIT(8), 0);

static const char * const espi_sclk_src_parents[] = {
	"pll2_20", "pll2_25", "pll2_33", "pll2_50", "pll2_66",
};
CCU_MUX_DEFINE(CLK_APMU_ESPI_SCLK_SRC, espi_sclk_src, espi_sclk_src, espi_sclk_src_parents, ARRAY_SIZE(espi_sclk_src_parents),
		APMU_ESPI_CLK_RES_CTRL, 4, 3, 0);

static const char * const espi_sclk_parents[] = {
	"external_clk", "espi_sclk_src",
};
CCU_MUX_GATE_DEFINE(CLK_APMU_ESPI_SCLK, espi_sclk, espi_sclk, espi_sclk_parents, ARRAY_SIZE(espi_sclk_parents),
		    APMU_ESPI_CLK_RES_CTRL, 7, 1, BIT(3), 0);

CCU_GATE_DEFINE(CLK_APMU_ESPI_MCLK, espi_mclk, espi_mclk, "axi_clk",
		APMU_ESPI_CLK_RES_CTRL, BIT(1), 0);

CCU_FACTOR_DEFINE(CLK_APMU_CAM_SRC1, cam_src1_clk, cam_src1_clk, "pll1_d6_409p6", 15, 1);
CCU_FACTOR_DEFINE(CLK_APMU_CAM_SRC2, cam_src2_clk, cam_src2_clk, "pll2_d5", 25, 1);
CCU_FACTOR_DEFINE(CLK_APMU_CAM_SRC3, cam_src3_clk, cam_src3_clk, "pll2_d6", 20, 1);
CCU_FACTOR_DEFINE(CLK_APMU_CAM_SRC4, cam_src4_clk, cam_src4_clk, "pll1_d6_409p6", 16, 1);

static const char * const isim_vclk_parents[] = {
	"cam_src1_clk", "cam_src2_clk", "cam_src3_clk", "cam_src4_clk",
};
CCU_MUX_DIV_GATE_DEFINE(CLK_APMU_ISIM_VCLK0, isim_vclk_out0, isim_vclk_out0, isim_vclk_parents, ARRAY_SIZE(isim_vclk_parents),
			APMU_SNR_ISIM_VCLK_CTRL, 3, 4, 1, 2,
			BIT(0), 0);
CCU_MUX_DIV_GATE_DEFINE(CLK_APMU_ISIM_VCLK1, isim_vclk_out1, isim_vclk_out1, isim_vclk_parents, ARRAY_SIZE(isim_vclk_parents),
			APMU_SNR_ISIM_VCLK_CTRL, 11, 4, 9, 2,
			BIT(8), 0);
CCU_MUX_DIV_GATE_DEFINE(CLK_APMU_ISIM_VCLK2, isim_vclk_out2, isim_vclk_out2, isim_vclk_parents, ARRAY_SIZE(isim_vclk_parents),
			APMU_SNR_ISIM_VCLK_CTRL, 19, 4, 17, 2,
			BIT(16), 0);
CCU_MUX_DIV_GATE_DEFINE(CLK_APMU_ISIM_VCLK3, isim_vclk_out3, isim_vclk_out3, isim_vclk_parents, ARRAY_SIZE(isim_vclk_parents),
			APMU_SNR_ISIM_VCLK_CTRL, 27, 4, 25, 2,
			BIT(24), 0);
/* APMU clocks end */

/* DCIU clocks start */
CCU_GATE_DEFINE(CLK_DCIU_HDMA, hdma_clk, hdma_clk, "axi_clk",
		DCIU_DMASYS_CLK_EN, BIT(0), 0);
CCU_GATE_DEFINE(CLK_DCIU_DMA350, dma350_clk, dma350_clk, "axi_clk",
		DCIU_DMASYS_SDMA_CLK_EN, BIT(0), 0);
CCU_GATE_DEFINE(CLK_DCIU_C2_TCM_PIPE, c2_tcm_pipe_clk, c2_tcm_pipe_clk, "axi_clk",
		DCIU_C2_TCM_PIPE_CLK, BIT(0), 0);
CCU_GATE_DEFINE(CLK_DCIU_C3_TCM_PIPE, c3_tcm_pipe_clk, c3_tcm_pipe_clk, "axi_clk",
		DCIU_C3_TCM_PIPE_CLK, BIT(0), 0);
/* DCIU clocks end */












static struct clk *k3_ccu_pll_clks[] = {
	&pll1.common.clk,
	&pll2.common.clk,
	&pll3.common.clk,
	&pll4.common.clk,
	&pll5.common.clk,
	&pll6.common.clk,
	&pll7.common.clk,
	&pll8.common.clk,
	&pll1_d2.common.clk,
	&pll1_d3.common.clk,
	&pll1_d4.common.clk,
	&pll1_d5.common.clk,
	&pll1_d6.common.clk,
	&pll1_d7.common.clk,
	&pll1_d8.common.clk,
	&pll1_dx.common.clk,
	&pll1_d64_38p4.common.clk,
	&pll1_aud_245p7.common.clk,
	&pll1_aud_24p5.common.clk,
	&pll2_d1.common.clk,
	&pll2_d2.common.clk,
	&pll2_d3.common.clk,
	&pll2_d4.common.clk,
	&pll2_d5.common.clk,
	&pll2_d6.common.clk,
	&pll2_d7.common.clk,
	&pll2_d8.common.clk,
	&pll2_66.common.clk,
	&pll2_33.common.clk,
	&pll2_50.common.clk,
	&pll2_25.common.clk,
	&pll2_20.common.clk,
	&pll2_d24_125.common.clk,
	&pll2_d120_25.common.clk,
	&pll3_d1.common.clk,
	&pll3_d2.common.clk,
	&pll3_d3.common.clk,
	&pll3_d4.common.clk,
	&pll3_d5.common.clk,
	&pll3_d6.common.clk,
	&pll3_d7.common.clk,
	&pll3_d8.common.clk,
	&pll4_d1.common.clk,
	&pll4_d2.common.clk,
	&pll4_d3.common.clk,
	&pll4_d4.common.clk,
	&pll4_d5.common.clk,
	&pll4_d6.common.clk,
	&pll4_d7.common.clk,
	&pll4_d8.common.clk,
	&pll5_d1.common.clk,
	&pll5_d2.common.clk,
	&pll5_d3.common.clk,
	&pll5_d4.common.clk,
	&pll5_d5.common.clk,
	&pll5_d6.common.clk,
	&pll5_d7.common.clk,
	&pll5_d8.common.clk,
	&pll6_d1.common.clk,
	&pll6_d2.common.clk,
	&pll6_d3.common.clk,
	&pll6_d4.common.clk,
	&pll6_d5.common.clk,
	&pll6_d6.common.clk,
	&pll6_d7.common.clk,
	&pll6_d8.common.clk,
	&pll6_80.common.clk,
	&pll6_40.common.clk,
	&pll6_20.common.clk,
	&pll7_d1.common.clk,
	&pll7_d2.common.clk,
	&pll7_d3.common.clk,
	&pll7_d4.common.clk,
	&pll7_d5.common.clk,
	&pll7_d6.common.clk,
	&pll7_d7.common.clk,
	&pll7_d8.common.clk,
	&pll8_d1.common.clk,
	&pll8_d2.common.clk,
	&pll8_d3.common.clk,
	&pll8_d4.common.clk,
	&pll8_d5.common.clk,
	&pll8_d6.common.clk,
	&pll8_d7.common.clk,
	&pll8_d8.common.clk,
};

static const struct spacemit_ccu_data k3_ccu_pll_data = {
	.clks		= k3_ccu_pll_clks,
	.num		= ARRAY_SIZE(k3_ccu_pll_clks),
	.offset		= K3_PLL_ID,
};

static struct clk *k3_ccu_mpmu_clks[] = {
	&pll1_d8_307p2.common.clk,
	&pll1_d32_76p8.common.clk,
	&pll1_d40_61p44.common.clk,
	&pll1_d16_153p6.common.clk,
	&pll1_d24_102p4.common.clk,
	&pll1_d48_51p2.common.clk,
	&pll1_d48_51p2_ap.common.clk,
	&pll1_m3d128_57p6.common.clk,
	&pll1_d96_25p6.common.clk,
	&pll1_d192_12p8.common.clk,
	&pll1_d192_12p8_wdt.common.clk,
	&pll1_d384_6p4.common.clk,
	&pll1_d768_3p2.common.clk,
	&pll1_d1536_1p6.common.clk,
	&pll1_d3072_0p8.common.clk,
	&pll1_d6_409p6.common.clk,
	&pll1_d12_204p8.common.clk,
	&pll1_d5_491p52.common.clk,
	&pll1_d10_245p76.common.clk,
	&pll1_d4_614p4.common.clk,
	&pll1_d52_47p26.common.clk,
	&pll1_d78_31p5.common.clk,
	&pll1_d3_819p2.common.clk,
	&pll1_d2_1228p8.common.clk,
	&apb_clk.common.clk,
	&slow_uart.common.clk,
	&slow_uart1_14p74.common.clk,
	&slow_uart2_48.common.clk,
	&wdt_clk.common.clk,
	&wdt_bus_clk.common.clk,
	&r_ipc_clk.common.clk,
	&i2s_153p6.common.clk,
	&i2s_153p6_base.common.clk,
	&i2s_sysclk_src.common.clk,
	&i2s1_sysclk.common.clk,
	&i2s_bclk.common.clk,
	&i2s0_sysclk_sel.common.clk,
	&i2s2_sysclk_sel.common.clk,
	&i2s3_sysclk_sel.common.clk,
	&i2s4_sysclk_sel.common.clk,
	&i2s5_sysclk_sel.common.clk,
	&i2s0_sysclk_div.common.clk,
	&i2s2_sysclk_div.common.clk,
	&i2s3_sysclk_div.common.clk,
	&i2s4_sysclk_div.common.clk,
	&i2s5_sysclk_div.common.clk,
	&i2s0_sysclk.common.clk,
	&i2s2_sysclk.common.clk,
	&i2s3_sysclk.common.clk,
	&i2s4_sysclk.common.clk,
	&i2s5_sysclk.common.clk,
};

static const struct spacemit_ccu_data k3_ccu_mpmu_data = {
	.clks		= k3_ccu_mpmu_clks,
	.num		= ARRAY_SIZE(k3_ccu_mpmu_clks),
	.offset		= K3_MPMU_ID,
};

static struct clk *k3_ccu_apbc_clks[] = {
	&uart0_clk.common.clk,
	&uart2_clk.common.clk,
	&uart3_clk.common.clk,
	&uart4_clk.common.clk,
	&uart5_clk.common.clk,
	&uart6_clk.common.clk,
	&uart7_clk.common.clk,
	&uart8_clk.common.clk,
	&uart9_clk.common.clk,
	&uart10_clk.common.clk,
	&uart0_bus_clk.common.clk,
	&uart2_bus_clk.common.clk,
	&uart3_bus_clk.common.clk,
	&uart4_bus_clk.common.clk,
	&uart5_bus_clk.common.clk,
	&uart6_bus_clk.common.clk,
	&uart7_bus_clk.common.clk,
	&uart8_bus_clk.common.clk,
	&uart9_bus_clk.common.clk,
	&uart10_bus_clk.common.clk,
	&gpio_clk.common.clk,
	&gpio_bus_clk.common.clk,
	&pwm0_clk.common.clk,
	&pwm1_clk.common.clk,
	&pwm2_clk.common.clk,
	&pwm3_clk.common.clk,
	&pwm4_clk.common.clk,
	&pwm5_clk.common.clk,
	&pwm6_clk.common.clk,
	&pwm7_clk.common.clk,
	&pwm8_clk.common.clk,
	&pwm9_clk.common.clk,
	&pwm10_clk.common.clk,
	&pwm11_clk.common.clk,
	&pwm12_clk.common.clk,
	&pwm13_clk.common.clk,
	&pwm14_clk.common.clk,
	&pwm15_clk.common.clk,
	&pwm16_clk.common.clk,
	&pwm17_clk.common.clk,
	&pwm18_clk.common.clk,
	&pwm19_clk.common.clk,
	&pwm0_bus_clk.common.clk,
	&pwm1_bus_clk.common.clk,
	&pwm2_bus_clk.common.clk,
	&pwm3_bus_clk.common.clk,
	&pwm4_bus_clk.common.clk,
	&pwm5_bus_clk.common.clk,
	&pwm6_bus_clk.common.clk,
	&pwm7_bus_clk.common.clk,
	&pwm8_bus_clk.common.clk,
	&pwm9_bus_clk.common.clk,
	&pwm10_bus_clk.common.clk,
	&pwm11_bus_clk.common.clk,
	&pwm12_bus_clk.common.clk,
	&pwm13_bus_clk.common.clk,
	&pwm14_bus_clk.common.clk,
	&pwm15_bus_clk.common.clk,
	&pwm16_bus_clk.common.clk,
	&pwm17_bus_clk.common.clk,
	&pwm18_bus_clk.common.clk,
	&pwm19_bus_clk.common.clk,
	&spi0_i2s_bclk.common.clk,
	&spi1_i2s_bclk.common.clk,
	&spi3_i2s_bclk.common.clk,
	&spi0_clk.common.clk,
	&spi1_clk.common.clk,
	&spi3_clk.common.clk,
	&spi0_bus_clk.common.clk,
	&spi1_bus_clk.common.clk,
	&spi3_bus_clk.common.clk,
	&rtc_clk.common.clk,
	&rtc_bus_clk.common.clk,
	&twsi0_clk.common.clk,
	&twsi1_clk.common.clk,
	&twsi2_clk.common.clk,
	&twsi4_clk.common.clk,
	&twsi5_clk.common.clk,
	&twsi6_clk.common.clk,
	&twsi8_clk.common.clk,
	&twsi0_bus_clk.common.clk,
	&twsi1_bus_clk.common.clk,
	&twsi2_bus_clk.common.clk,
	&twsi4_bus_clk.common.clk,
	&twsi5_bus_clk.common.clk,
	&twsi6_bus_clk.common.clk,
	&twsi8_bus_clk.common.clk,
	&timers0_clk.common.clk,
	&timers1_clk.common.clk,
	&timers2_clk.common.clk,
	&timers3_clk.common.clk,
	&timers4_clk.common.clk,
	&timers5_clk.common.clk,
	&timers6_clk.common.clk,
	&timers7_clk.common.clk,
	&timers0_bus_clk.common.clk,
	&timers1_bus_clk.common.clk,
	&timers2_bus_clk.common.clk,
	&timers3_bus_clk.common.clk,
	&timers4_bus_clk.common.clk,
	&timers5_bus_clk.common.clk,
	&timers6_bus_clk.common.clk,
	&timers7_bus_clk.common.clk,
	&aib_clk.common.clk,
	&aib_bus_clk.common.clk,
	&onewire_clk.common.clk,
	&onewire_bus_clk.common.clk,
	&i2s0_i2s_bclk.common.clk,
	&i2s1_i2s_bclk.common.clk,
	&i2s2_i2s_bclk.common.clk,
	&i2s3_i2s_bclk.common.clk,
	&i2s4_i2s_bclk.common.clk,
	&i2s5_i2s_bclk.common.clk,
	&i2s0_clk.common.clk,
	&i2s1_clk.common.clk,
	&i2s2_clk.common.clk,
	&i2s3_clk.common.clk,
	&i2s4_clk.common.clk,
	&i2s5_clk.common.clk,
	&i2s0_bus_clk.common.clk,
	&i2s1_bus_clk.common.clk,
	&i2s2_bus_clk.common.clk,
	&i2s3_bus_clk.common.clk,
	&i2s4_bus_clk.common.clk,
	&i2s5_bus_clk.common.clk,
	&dro_clk.common.clk,
	&ir0_clk.common.clk,
	&ir1_clk.common.clk,
	&tsen_clk.common.clk,
	&tsen_bus_clk.common.clk,
	&ipc_ap2rcpu_clk.common.clk,
	&ipc_ap2rcpu_bus_clk.common.clk,
	&can0_clk.common.clk,
	&can1_clk.common.clk,
	&can2_clk.common.clk,
	&can3_clk.common.clk,
	&can4_clk.common.clk,
	&can0_bus_clk.common.clk,
	&can1_bus_clk.common.clk,
	&can2_bus_clk.common.clk,
	&can3_bus_clk.common.clk,
	&can4_bus_clk.common.clk,
};

static const struct spacemit_ccu_data k3_ccu_apbc_data = {
	.clks		= k3_ccu_apbc_clks,
	.num		= ARRAY_SIZE(k3_ccu_apbc_clks),
	.offset		= K3_APBC_ID,
};

static struct clk *k3_ccu_apmu_clks[] = {
	&axi_clk.common.clk,
	&cci550_clk.common.clk,
	&cpu_c0_core_clk.common.clk,
	&cpu_c1_core_clk.common.clk,
	&cpu_c2_core_clk.common.clk,
	&cpu_c3_core_clk.common.clk,
	&ccic2phy_clk.common.clk,
	&ccic3phy_clk.common.clk,
	&csi_clk.common.clk,
	&isp_bus_clk.common.clk,
	&d1p_1228p8.common.clk,
	&d1p_819p2.common.clk,
	&d1p_614p4.common.clk,
	&d1p_491p52.common.clk,
	&d1p_409p6.common.clk,
	&d1p_307p2.common.clk,
	&d1p_245p76.common.clk,
	&v2d_clk.common.clk,
	&dsi_esc_clk.common.clk,
	&lcd_hclk.common.clk,
	&lcd_dsc_clk.common.clk,
	&lcd_pxclk.common.clk,
	&lcd_mclk.common.clk,
	&ccic_4x_clk.common.clk,
	&ccic1phy_clk.common.clk,
	&sc2_hclk.common.clk,
	&sdh_axi_aclk.common.clk,
	&sdh0_clk.common.clk,
	&sdh1_clk.common.clk,
	&sdh2_clk.common.clk,
	&usb2_bus_clk.common.clk,
	&usb3_porta_bus_clk.common.clk,
	&usb3_portb_bus_clk.common.clk,
	&usb3_portc_bus_clk.common.clk,
	&usb3_portd_bus_clk.common.clk,
	&qspi_clk.common.clk,
	&qspi_bus_clk.common.clk,
	&dma_clk.common.clk,
	&aes_wtm_clk.common.clk,
	&vpu_clk.common.clk,
	&dtc_clk.common.clk,
	&gpu_clk.common.clk,
	&mc_ahb_clk.common.clk,
	&top_dclk.common.clk,
	&ucie_clk.common.clk,
	&ucie_sbclk.common.clk,
	&rcpu_clk.common.clk,
	&dsi4ln2_dsi_esc_clk.common.clk,
	&dsi4ln2_lcd_dsc_clk.common.clk,
	&dsi4ln2_lcd_pxclk.common.clk,
	&dsi4ln2_lcd_mclk.common.clk,
	&dsi4ln2_dpu_aclk.common.clk,
	&dpu_aclk.common.clk,
	&ufs_aclk.common.clk,
	&edp0_pxclk.common.clk,
	&edp1_pxclk.common.clk,
	&pciea_mstr_clk.common.clk,
	&pciea_slv_clk.common.clk,
	&pcieb_mstr_clk.common.clk,
	&pcieb_slv_clk.common.clk,
	&pciec_mstr_clk.common.clk,
	&pciec_slv_clk.common.clk,
	&pcied_mstr_clk.common.clk,
	&pcied_slv_clk.common.clk,
	&pciee_mstr_clk.common.clk,
	&pciee_slv_clk.common.clk,
	&emac0_bus_clk.common.clk,
	&emac0_ref_clk.common.clk,
	&emac0_1588_clk.common.clk,
	&emac0_rgmii_tx_clk.common.clk,
	&emac1_bus_clk.common.clk,
	&emac1_ref_clk.common.clk,
	&emac1_1588_clk.common.clk,
	&emac1_rgmii_tx_clk.common.clk,
	&emac2_bus_clk.common.clk,
	&emac2_ref_clk.common.clk,
	&emac2_1588_clk.common.clk,
	&emac2_rgmii_tx_clk.common.clk,
	&espi_sclk_src.common.clk,
	&espi_sclk.common.clk,
	&espi_mclk.common.clk,
	&cam_src1_clk.common.clk,
	&cam_src2_clk.common.clk,
	&cam_src3_clk.common.clk,
	&cam_src4_clk.common.clk,
	&isim_vclk_out0.common.clk,
	&isim_vclk_out1.common.clk,
	&isim_vclk_out2.common.clk,
	&isim_vclk_out3.common.clk,
};

static const struct spacemit_ccu_data k3_ccu_apmu_data = {
	.clks		= k3_ccu_apmu_clks,
	.num		= ARRAY_SIZE(k3_ccu_apmu_clks),
	.offset		= K3_APMU_ID,
};

/*
 * These clocks are marked as CLK_IS_CRITICAL, but unlike the
 * kernel, U-Boot has no mechanism to enable critical clocks.
 * Enable them explicitly here, otherwise Linux gets stuck
 * halfway through boot.
 */
static struct clk *const k3_apmu_boot_clks[] = {
	&cpu_c0_core_clk.common.clk,
	&cpu_c1_core_clk.common.clk,
	&cpu_c2_core_clk.common.clk,
	&cpu_c3_core_clk.common.clk,
};

static struct clk *k3_ccu_dciu_clks[] = {
	&hdma_clk.common.clk,
	&dma350_clk.common.clk,
	&c2_tcm_pipe_clk.common.clk,
	&c3_tcm_pipe_clk.common.clk,
};

static const struct spacemit_ccu_data k3_ccu_dciu_data = {
	.clks		= k3_ccu_dciu_clks,
	.num		= ARRAY_SIZE(k3_ccu_dciu_clks),
	.offset		= K3_DCIU_ID,
};

static int k3_clk_force_probe(const struct driver *drv)
{
	struct udevice *dev;

	return uclass_get_device_by_driver(UCLASS_CLK, drv, &dev);
}

static int k3_clk_register(struct udevice *dev, struct regmap *regmap,
			   struct regmap *lock_regmap,
			   const struct spacemit_ccu_data *data)
{
	int i, ret;

	for (i = 0; i < data->num; i++) {
		struct clk *clk = data->clks[i];
		struct ccu_common *common;

		if (!clk)
			continue;

		common = clk_to_ccu_common(clk);
		common->regmap = regmap;
		common->lock_regmap = lock_regmap;

		if (common->clk.id < K3_PLL_ID)
			clk->id = common->clk.id + data->offset;
		else
			clk->id = common->clk.id;

		ret = common->init(common);
		if (ret)
			return ret;
	}

	return 0;
}

static int k3_pll_clk_probe(struct udevice *dev)
{
	struct regmap *base_regmap, *lock_regmap = NULL;
	const struct spacemit_ccu_data *data;
	int ret;

	clk_register_fixed_rate(NULL, "clock-dummy", 0);

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	if (device_is_compatible(dev, "spacemit,k3-pll")) {
		struct ofnode_phandle_args mpmu_args;

		ret = dev_read_phandle_with_args(dev, "spacemit,mpmu", NULL, 0, 0,
						 &mpmu_args);
		if (ret)
			return ret;

		ret = regmap_init_mem(mpmu_args.node, &lock_regmap);
		if (ret)
			return ret;
	}

	data = (struct spacemit_ccu_data *)dev_get_driver_data(dev);

	ret = k3_clk_register(dev, base_regmap, lock_regmap, data);
	if (ret)
		return -EPROBE_DEFER;

	return 0;
}

static int k3_mpmu_clk_probe(struct udevice *dev)
{
	struct regmap *base_regmap;
	const struct spacemit_ccu_data *data;
	int ret;

	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_pll_clk));
	if (ret)
		return ret;

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	data = (struct spacemit_ccu_data *)dev_get_driver_data(dev);

	ret = k3_clk_register(dev, base_regmap, NULL, data);
	if (ret)
		return -EPROBE_DEFER;

	return 0;
}

static int k3_apmu_clk_probe(struct udevice *dev)
{
	struct regmap *base_regmap;
	const struct spacemit_ccu_data *data;
	int i, ret;

	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_pll_clk));
	if (ret)
		return ret;
	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_mpmu_clk));
	if (ret)
		return ret;

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	data = (struct spacemit_ccu_data *)dev_get_driver_data(dev);

	ret = k3_clk_register(dev, base_regmap, NULL, data);
	if (ret)
		return -EPROBE_DEFER;

	for (i = 0; i < ARRAY_SIZE(k3_apmu_boot_clks); i++) {
		ret = clk_enable(k3_apmu_boot_clks[i]);
		if (ret)
			return ret;
	}

	return 0;
}

static int k3_apbc_clk_probe(struct udevice *dev)
{
	struct regmap *base_regmap;
	const struct spacemit_ccu_data *data;
	int ret;

	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_pll_clk));
	if (ret)
		return ret;
	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_mpmu_clk));
	if (ret)
		return ret;
	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_apmu_clk));
	if (ret)
		return ret;

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	data = (struct spacemit_ccu_data *)dev_get_driver_data(dev);

	ret = k3_clk_register(dev, base_regmap, NULL, data);
	if (ret)
		return -EPROBE_DEFER;

	return 0;
}

static int k3_dciu_clk_probe(struct udevice *dev)
{
	struct regmap *base_regmap;
	const struct spacemit_ccu_data *data;
	int ret;

	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_pll_clk));
	if (ret)
		return ret;
	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_mpmu_clk));
	if (ret)
		return ret;
	ret = k3_clk_force_probe(DM_DRIVER_GET(k3_apmu_clk));
	if (ret)
		return ret;

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	data = (struct spacemit_ccu_data *)dev_get_driver_data(dev);

	ret = k3_clk_register(dev, base_regmap, NULL, data);
	if (ret)
		return -EPROBE_DEFER;

	return 0;
}

static int k3_pll_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K3_PLL_ID + args->args[0];
	else
		clk->id = K3_PLL_ID;

	return 0;
}

static int k3_mpmu_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K3_MPMU_ID + args->args[0];
	else
		clk->id = K3_MPMU_ID;

	return 0;
}

static int k3_apbc_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K3_APBC_ID + args->args[0];
	else
		clk->id = K3_APBC_ID;

	return 0;
}

static int k3_apmu_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K3_APMU_ID + args->args[0];
	else
		clk->id = K3_APMU_ID;

	return 0;
}

static int k3_dciu_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K3_DCIU_ID + args->args[0];
	else
		clk->id = K3_DCIU_ID;

	return 0;
}

#define K3_CLK_OPS(name)				\
static const struct clk_ops k3_##name##_clk_ops = {	\
		.set_rate = ccf_clk_set_rate,		\
		.get_rate = ccf_clk_get_rate,		\
		.enable = ccf_clk_enable,		\
		.disable = ccf_clk_disable,		\
		.set_parent = ccf_clk_set_parent,	\
		.of_xlate = k3_##name##_clk_of_xlate,	\
}

static const struct udevice_id k3_pll_clk_match[] = {
	{ .compatible = "spacemit,k3-pll",
	  .data = (ulong)&k3_ccu_pll_data },
	{ /* sentinel */ },
};

K3_CLK_OPS(pll);

U_BOOT_DRIVER(k3_pll_clk) = {
	.name		= "k3_pll_clk",
	.id		= UCLASS_CLK,
	.of_match	= k3_pll_clk_match,
	.probe		= k3_pll_clk_probe,
	.ops		= &k3_pll_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static int k3_mpmu_clk_bind(struct udevice *dev)
{
	return spacemit_k3_reset_bind(dev, SPACEMIT_K3_RESET_MPMU);
}

static int k3_apbc_clk_bind(struct udevice *dev)
{
	return spacemit_k3_reset_bind(dev, SPACEMIT_K3_RESET_APBC);
}

static int k3_apmu_clk_bind(struct udevice *dev)
{
	return spacemit_k3_reset_bind(dev, SPACEMIT_K3_RESET_APMU);
}

static int k3_dciu_clk_bind(struct udevice *dev)
{
	return spacemit_k3_reset_bind(dev, SPACEMIT_K3_RESET_DCIU);
}

static const struct udevice_id k3_mpmu_clk_match[] = {
	{ .compatible = "spacemit,k3-syscon-mpmu",
	  .data = (ulong)&k3_ccu_mpmu_data },
	{ /* sentinel */ },
};

K3_CLK_OPS(mpmu);

U_BOOT_DRIVER(k3_mpmu_clk) = {
	.name		= "k3_mpmu_clk",
	.id		= UCLASS_CLK,
	.of_match	= k3_mpmu_clk_match,
	.bind		= k3_mpmu_clk_bind,
	.probe		= k3_mpmu_clk_probe,
	.ops		= &k3_mpmu_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k3_apbc_clk_match[] = {
	{ .compatible = "spacemit,k3-syscon-apbc",
	  .data = (ulong)&k3_ccu_apbc_data },
	{ /* sentinel */ },
};

K3_CLK_OPS(apbc);

U_BOOT_DRIVER(k3_apbc_clk) = {
	.name		= "k3_apbc_clk",
	.id		= UCLASS_CLK,
	.of_match	= k3_apbc_clk_match,
	.bind		= k3_apbc_clk_bind,
	.probe		= k3_apbc_clk_probe,
	.ops		= &k3_apbc_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k3_apmu_clk_match[] = {
	{ .compatible = "spacemit,k3-syscon-apmu",
	  .data = (ulong)&k3_ccu_apmu_data },
	{ /* sentinel */ },
};

K3_CLK_OPS(apmu);

U_BOOT_DRIVER(k3_apmu_clk) = {
	.name		= "k3_apmu_clk",
	.id		= UCLASS_CLK,
	.of_match	= k3_apmu_clk_match,
	.bind		= k3_apmu_clk_bind,
	.probe		= k3_apmu_clk_probe,
	.ops		= &k3_apmu_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k3_dciu_clk_match[] = {
	{ .compatible = "spacemit,k3-syscon-dciu",
	  .data = (ulong)&k3_ccu_dciu_data },
	{ /* sentinel */ },
};

K3_CLK_OPS(dciu);

U_BOOT_DRIVER(k3_dciu_clk) = {
	.name		= "k3_dciu_clk",
	.id		= UCLASS_CLK,
	.of_match	= k3_dciu_clk_match,
	.bind		= k3_dciu_clk_bind,
	.probe		= k3_dciu_clk_probe,
	.ops		= &k3_dciu_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
