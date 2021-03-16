/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

#include <linux/build_bug.h>

enum {
	TCM_LOCK,
	TCM_SPLIT,
};

void tcm_init(u8 mode);
void mem_map_fill(void);

static inline int zynqmp_mmio_write(const u32 address, const u32 mask, const u32 value)
{
	BUILD_BUG();
	return -EINVAL;
}
