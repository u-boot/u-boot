// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <asm/cpu_common.h>
#include <asm/msr.h>

void cpu_flush_l1d_to_l2(void)
{
	struct msr_t msr;

	msr = msr_read(MSR_POWER_MISC);
	msr.lo |= FLUSH_DL1_L2;
	msr_write(MSR_POWER_MISC, msr);
}
