// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <asm/arch/imx-regs.h>
#include <linux/bitops.h>
#include <common.h>
#include <fsl_wdog.h>

#define GPC_CPU_PGC_SW_PDN_REQ	0xfc
#define GPC_CPU_PGC_SW_PUP_REQ	0xf0
#define GPC_PGC_C0		0x800
#define GPC_PGC_C1		0x840

#define BM_CPU_PGC_SW_PDN_PUP_REQ_CORE0_A7	0x1
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

#define MPIDR_AFF0		GENMASK(7, 0)

#define IMX7D_PSCI_NR_CPUS	2
#if IMX7D_PSCI_NR_CPUS > CONFIG_ARMV7_PSCI_NR_CPUS
#error "invalid value for CONFIG_ARMV7_PSCI_NR_CPUS"
#endif

u8 psci_state[IMX7D_PSCI_NR_CPUS] __secure_data = {
	 PSCI_AFFINITY_LEVEL_ON,
	 PSCI_AFFINITY_LEVEL_OFF};

static inline void psci_set_state(int cpu, u8 state)
{
	psci_state[cpu] = state;
	dsb();
	isb();
}

static inline void imx_gpcv2_set_m_core_pgc(bool enable, u32 offset)
{
	writel(enable, GPC_IPS_BASE_ADDR + offset);
}

__secure void imx_gpcv2_set_core_power(int cpu, bool pdn)
{
	u32 reg = pdn ? GPC_CPU_PGC_SW_PUP_REQ : GPC_CPU_PGC_SW_PDN_REQ;
	u32 pgc = cpu ? GPC_PGC_C1 : GPC_PGC_C0;
	u32 pdn_pup_req = cpu ? BM_CPU_PGC_SW_PDN_PUP_REQ_CORE1_A7 :
				BM_CPU_PGC_SW_PDN_PUP_REQ_CORE0_A7;
	u32 val;

	imx_gpcv2_set_m_core_pgc(true, pgc);

	val = readl(GPC_IPS_BASE_ADDR + reg);
	val |= pdn_pup_req;
	writel(val, GPC_IPS_BASE_ADDR + reg);

	while ((readl(GPC_IPS_BASE_ADDR + reg) & pdn_pup_req) != 0)
		;

	imx_gpcv2_set_m_core_pgc(false, pgc);
}

__secure void imx_enable_cpu_ca7(int cpu, bool enable)
{
	u32 mask, val;

	mask = 1 << (BP_SRC_A7RCR1_A7_CORE1_ENABLE + cpu - 1);
	val = readl(SRC_BASE_ADDR + SRC_A7RCR1);
	val = enable ? val | mask : val & ~mask;
	writel(val, SRC_BASE_ADDR + SRC_A7RCR1);
}

__secure void psci_arch_cpu_entry(void)
{
	u32 cpu = psci_get_cpu_id();

	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_ON);
}

__secure s32 psci_cpu_on(u32 __always_unused function_id, u32 mpidr, u32 ep,
			 u32 context_id)
{
	u32 cpu = mpidr & MPIDR_AFF0;

	if (mpidr & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= IMX7D_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	if (psci_state[cpu] == PSCI_AFFINITY_LEVEL_ON)
		return ARM_PSCI_RET_ALREADY_ON;

	if (psci_state[cpu] == PSCI_AFFINITY_LEVEL_ON_PENDING)
		return ARM_PSCI_RET_ON_PENDING;

	psci_save(cpu, ep, context_id);

	writel((u32)psci_cpu_entry, SRC_BASE_ADDR + cpu * 8 + SRC_GPR1_MX7D);

	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_ON_PENDING);

	imx_gpcv2_set_core_power(cpu, true);
	imx_enable_cpu_ca7(cpu, true);

	return ARM_PSCI_RET_SUCCESS;
}

__secure s32 psci_cpu_off(void)
{
	int cpu;

	cpu = psci_get_cpu_id();

	psci_cpu_off_common();
	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_OFF);

	imx_enable_cpu_ca7(cpu, false);
	imx_gpcv2_set_core_power(cpu, false);
	writel(0, SRC_BASE_ADDR + cpu * 8 + SRC_GPR1_MX7D + 4);

	while (1)
		wfi();
}

__secure void psci_system_reset(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	/* make sure WDOG1 clock is enabled */
	writel(0x1 << 28, CCM_BASE_ADDR + CCM_ROOT_WDOG);
	writel(0x3, CCM_BASE_ADDR + CCM_CCGR_WDOG1);
	writew(WCR_WDE, &wdog->wcr);

	while (1)
		wfi();
}

__secure void psci_system_off(void)
{
	u32 val;

	/* make sure SNVS clock is enabled */
	writel(0x3, CCM_BASE_ADDR + CCM_CCGR_SNVS);

	val = readl(SNVS_BASE_ADDR + SNVS_LPCR);
	val |= BP_SNVS_LPCR_DP_EN | BP_SNVS_LPCR_TOP;
	writel(val, SNVS_BASE_ADDR + SNVS_LPCR);

	while (1)
		wfi();
}

__secure u32 psci_version(void)
{
	return ARM_PSCI_VER_1_0;
}

__secure s32 psci_cpu_suspend(u32 __always_unused function_id, u32 power_state,
			      u32 entry_point_address,
			      u32 context_id)
{
	return ARM_PSCI_RET_INVAL;
}

__secure s32 psci_affinity_info(u32 __always_unused function_id,
				u32 target_affinity,
				u32 lowest_affinity_level)
{
	u32 cpu = target_affinity & MPIDR_AFF0;

	if (lowest_affinity_level > 0)
		return ARM_PSCI_RET_INVAL;

	if (target_affinity & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= IMX7D_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	return psci_state[cpu];
}

__secure s32 psci_migrate_info_type(u32 function_id)
{
	/* Trusted OS is either not present or does not require migration */
	return 2;
}

__secure s32 psci_features(u32 __always_unused function_id, u32 psci_fid)
{
	switch (psci_fid) {
	case ARM_PSCI_0_2_FN_PSCI_VERSION:
	case ARM_PSCI_0_2_FN_CPU_OFF:
	case ARM_PSCI_0_2_FN_CPU_ON:
	case ARM_PSCI_0_2_FN_AFFINITY_INFO:
	case ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE:
	case ARM_PSCI_0_2_FN_SYSTEM_OFF:
	case ARM_PSCI_0_2_FN_SYSTEM_RESET:
	case ARM_PSCI_1_0_FN_PSCI_FEATURES:
		return 0x0;
	}
	return ARM_PSCI_RET_NI;
}
