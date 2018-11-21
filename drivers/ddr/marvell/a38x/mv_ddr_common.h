/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR_COMMON_H
#define _MV_DDR_COMMON_H

extern const char mv_ddr_build_message[];
extern const char mv_ddr_version_string[];

#define MV_DDR_NUM_BITS_IN_BYTE	8
#define MV_DDR_MEGA_BITS	(1024 * 1024)
#define MV_DDR_32_BITS_MASK	0xffffffff

unsigned int ceil_div(unsigned int x, unsigned int y);
unsigned int time_to_nclk(unsigned int t, unsigned int tclk);
int round_div(unsigned int dividend, unsigned int divisor, unsigned int *quotient);

#endif /* _MV_DDR_COMMON_H */
