/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <asm/system.h>

#define SMC_SIP_INVOKE_MCE	0x82FFFF00
#define MCE_SMC_ROC_FLUSH_CACHE	11

int __asm_flush_l3_cache(void)
{
	struct pt_regs regs = {0};

	isb();

	regs.regs[0] = SMC_SIP_INVOKE_MCE | MCE_SMC_ROC_FLUSH_CACHE;
	smc_call(&regs);

	return 0;
}
