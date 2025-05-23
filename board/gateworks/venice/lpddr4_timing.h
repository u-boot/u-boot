/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Gateworks Corporation
 */

#ifndef __LPDDR4_TIMING_H__
#define __LPDDR4_TIMING_H__

extern struct dram_timing_info *spl_dram_init(const char *model, int sizemb);

#endif /* __LPDDR4_TIMING_H__ */
