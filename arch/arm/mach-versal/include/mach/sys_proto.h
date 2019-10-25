/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

enum {
	TCM_LOCK,
	TCM_SPLIT,
};

#define PAYLOAD_ARG_CNT	4U

void tcm_init(u8 mode);
void mem_map_fill(void);
