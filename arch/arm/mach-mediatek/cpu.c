// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <cpu_func.h>
#include <dm.h>
#include <init.h>
#include <wdt.h>
#include <dm/uclass-internal.h>
#include <linux/arm-smccc.h>
#include <linux/types.h>

#define MTK_SIP_PLAT_BINFO ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_64, \
					      ARM_SMCCC_OWNER_SIP, 0x529)

int arch_cpu_init(void)
{
	icache_enable();

	return 0;
}

void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}

/**
 * mediatek_sip_part_name - get the part name
 *
 * Retrieve the part name of platform description.
 *
 * This only applicable to SoCs that support SIP plat binfo SMC call.
 *
 * Returns: the part name or 0 if error or no part name
 */
u32 mediatek_sip_part_name(void)
{
	if (CONFIG_IS_ENABLED(TARGET_MT8188) || CONFIG_IS_ENABLED(TARGET_MT8189) ||
	    CONFIG_IS_ENABLED(TARGET_MT8195) || CONFIG_IS_ENABLED(TARGET_MT8365)) {
		struct arm_smccc_res res __maybe_unused;

		arm_smccc_smc(MTK_SIP_PLAT_BINFO, 0, 0, 0, 0, 0, 0, 0, &res);
		if (res.a0)
			return 0;

		return res.a1;
	}

	return 0;
}

/**
 * mediatek_sip_segment_name - get the segment name
 *
 * Retrieve the segment name of platform description.
 *
 * This only applicable to SoCs that support SIP plat binfo SMC call.
 *
 * Returns: the segment name or 0 if error or no segment name
 */
u32 mediatek_sip_segment_name(void)
{
	if (CONFIG_IS_ENABLED(TARGET_MT8188) || CONFIG_IS_ENABLED(TARGET_MT8189) ||
	    CONFIG_IS_ENABLED(TARGET_MT8195) || CONFIG_IS_ENABLED(TARGET_MT8365)) {
		struct arm_smccc_res res __maybe_unused;

		arm_smccc_smc(MTK_SIP_PLAT_BINFO, 1, 0, 0, 0, 0, 0, 0, &res);
		if (res.a0)
			return 0;

		return res.a1;
	}

	return 0;
}
