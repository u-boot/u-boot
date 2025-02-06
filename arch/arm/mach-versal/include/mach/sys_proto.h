/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

#include <linux/build_bug.h>

enum tcm_mode {
	TCM_LOCK = 0,
	TCM_SPLIT = 1,
};

void initialize_tcm(enum tcm_mode mode);
void tcm_init(enum tcm_mode mode);
void mem_map_fill(void);
