/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) Toradex - https://www.toradex.com/
 */
#ifndef __AQUILA_DDRS_H
#define __AQUILA_DDRS_H

#define MULTI_DDR_CFG_INTRLV_SIZE_8GB 9
#define MULTI_DDR_CFG_INTRLV_SIZE_16GB 11

extern struct ddrss_patch *aquila_am69_ddrss_patch_8GB[4];
extern struct ddrss_patch *aquila_am69_ddrss_patch_16GB[4];
extern struct ddrss_patch *aquila_am69_ddrss_patch_16GB_rank_2[4];

#endif // __AQUILA_DDRS_H
