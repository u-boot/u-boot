/*
 * Copyright (C) 2016-2017 Intel Corporation
 *
 * SPDX-License-Identifier:    GPL-2.0
 */

#ifndef _MISC_H_
#define _MISC_H_

void dwmac_deassert_reset(const unsigned int of_reset_id, const u32 phymode);

struct bsel {
	const char	*mode;
	const char	*name;
};

extern struct bsel bsel_str[];

#ifdef CONFIG_FPGA
void socfpga_fpga_add(void);
#else
static inline void socfpga_fpga_add(void) {}
#endif

#endif /* _MISC_H_ */
