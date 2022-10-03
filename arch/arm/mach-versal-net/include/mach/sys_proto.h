/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.
 */

#include <linux/build_bug.h>

void mem_map_fill(void);

static inline int zynqmp_mmio_write(const u32 address, const u32 mask,
				    const u32 value)
{
	BUILD_BUG();
	return -EINVAL;
}
