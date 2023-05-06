// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#include <common.h>
#include <asm/io.h>
#include <cpu_func.h>
#include <linux/bitops.h>

#define JH7110_L2_PREFETCHER_BASE_ADDR		0x2030000
#define JH7110_L2_PREFETCHER_HART_OFFSET	0x2000

/* enable U74-mc hart1~hart4 prefetcher */
static void enable_prefetcher(void)
{
	u8 hart;
	u32 *reg;

	/* JH7110 use U74MC CORE IP, it include five cores(one S7 and four U7),
	 * but only U7 cores support prefetcher configuration
	 */
	for (hart = 1; hart < 5; hart++) {
		reg = (void *)(u64)(JH7110_L2_PREFETCHER_BASE_ADDR
					+ hart * JH7110_L2_PREFETCHER_HART_OFFSET);

		mb(); /* memory barrier */
		setbits_le32(reg, 0x1);
		mb(); /* memory barrier */
	}
}

int board_init(void)
{
	enable_caches();
	enable_prefetcher();

	return 0;
}
