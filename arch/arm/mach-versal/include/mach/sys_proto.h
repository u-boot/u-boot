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

int versal_pm_request(u32 api_id, u32 arg0, u32 arg1, u32 arg2,
		      u32 arg3, u32 *ret_payload);
