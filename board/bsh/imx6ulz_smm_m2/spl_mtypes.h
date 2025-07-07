/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 BSH Hausgeraete GmbH
 *
 * Written by: Simon Holesch <simon.holesch@bshg.com>
 */

#ifndef SPL_MTYPES_H
#define SPL_MTYPES_H

#include <spl.h>

struct dram_cfg_param {
	unsigned int reg;
	unsigned int val;
};

struct dram_timing_info {
	const struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
	size_t dram_size;
};

extern struct dram_timing_info bsh_dram_timing_128mb;
extern struct dram_timing_info bsh_dram_timing_256mb;
extern struct dram_timing_info bsh_dram_timing_512mb;

#endif /* SPL_MTYPES_H */
