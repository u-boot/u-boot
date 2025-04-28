/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Helpers that are commonly used with DW memory controller.
 *
 * (C) Copyright 2025 Jernej Skrabec <jernej.skrabec@gmail.com>
 *
 */

#ifndef _DRAM_DW_HELPERS_H
#define _DRAM_DW_HELPERS_H

#include <asm/arch/dram.h>

bool mctl_core_init(const struct dram_para *para,
		    const struct dram_config *config);
void mctl_auto_detect_rank_width(const struct dram_para *para,
				 struct dram_config *config);
void mctl_auto_detect_dram_size(const struct dram_para *para,
				struct dram_config *config);
unsigned long mctl_calc_size(const struct dram_config *config);

#endif
