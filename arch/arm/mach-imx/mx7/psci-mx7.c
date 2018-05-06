// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <asm/arch/imx-regs.h>
#include <common.h>
#include <fsl_wdog.h>

#define GPC_CPU_PGC_SW_PDN_REQ	0xfc
#define GPC_CPU_PGC_SW_PUP_REQ	0xf0
#define GPC_PGC_C1		0x840

#define BM_CPU_PGC_SW_PDN_PUP_REQ_CORE1_A7	0x2

/* below is for i.MX7D */
#define SRC_GPR1_MX7D		0x074
#define SRC_A7RCR0		0x004
#define SRC_A7RCR1		0x008

#define BP_SRC_A7RCR0_A7_CORE_RESET0	0
#define BP_SRC_A7RCR1_A7_CORE1_ENABLE	1

#define SNVS_LPCR		0x38
#define BP_SNVS_LPCR_DP_EN	0x20
#define BP_SNVS_LPCR_TOP	0x40

#define CCM_CCGR_SNVS		0x4250

#define CCM_ROOT_WDOG		0xbb80
#define CCM_CCGR_WDOG1		0x49c0

static inline void imx_gpcv2_set_m_core_pgc(bool enable, u32 offset)
{
	writel(enable, GPC_IPS_BASE_ADDR + offset);
}

__secure void imx_gpcv2_set_core1_power(bool pdn)
{
	u32 reg = pdn ? GPC_CPU_PGC_SW_PUP_REQ : GPC_CPU_PGC_SW_PDN_REQ;
	u32 val;

	imx_gpcv2_set_m_core_pgc(true, GPC_PGC_C1);

	val = readl(GPC_IPS_BASE_ADDR + reg);
	val |= BM_CPU_PGC_SW_PDN_PUP_REQ_CORE1_A7;
	writel(val, GPC_IPS_BASE_ADDR + reg);

	while ((readl(GPC_IPS_BASE_ADDR + reg) &
	       BM_CPU_PGC_SW_PDN_PUP_REQ_CORE1_A7) != 0)
		;

	imx_gpcv2_set_m_core_pgc(false, GPC_PGC_C1);
}

__secure void imx_enable_cpu_ca7(int cpu, bool enable)
{
	u32 mask, val;

	mask = 1 << (BP_SRC_A7RCR1_A7_CORE1_ENABLE + cpu - 1);
	val = readl(SRC_BASE_ADDR + SRC_A7RCR1);
	val = enable ? val | mask : val & ~mask;
	writel(val, SRC_BASE_ADDR + SRC_A7RCR1);
}

__secure int imx_cpu_on(int fn, int cpu, int pc)
{
	writel(pc, SRC_BASE_ADDR + cpu * 8 + SRC_GPR1_MX7D);
	imx_gpcv2_set_core1_power(true);
	imx_enable_cpu_ca7(cpu, true);
	return 0;
}

__secure int imx_cpu_off(int cpu)
{
	imx_enable_cpu_ca7(cpu, false);
	imx_gpcv2_set_core1_power(false);
	writel(0, SRC_BASE_ADDR + cpu * 8 + SRC_GPR1_MX7D + 4);
	return 0;
}

__secure void imx_system_reset(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	/* make sure WDOG1 clock is enabled */
	writel(0x1 << 28, CCM_BASE_ADDR + CCM_ROOT_WDOG);
	writel(0x3, CCM_BASE_ADDR + CCM_CCGR_WDOG1);
	writew(WCR_WDE, &wdog->wcr);
}

__secure void imx_system_off(void)
{
	u32 val;

	/* make sure SNVS clock is enabled */
	writel(0x3, CCM_BASE_ADDR + CCM_CCGR_SNVS);

	val = readl(SNVS_BASE_ADDR + SNVS_LPCR);
	val |= BP_SNVS_LPCR_DP_EN | BP_SNVS_LPCR_TOP;
	writel(val, SNVS_BASE_ADDR + SNVS_LPCR);
}
