// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Toradex - https://www.toradex.com/
 * This contains a diff against the 32GB register settings created from
 * the 16GB dual rank tool output.

 * The 16GB dtsi file was generated with the following tool revisions:
 *     - SysConfig: Revision 1.26.2+4477
 *     - Jacinto7_DDRSS_RegConfigTool: Revision 0.12.0
 * This file was generated on Fri Mar 06 2026 10:39:50 GMT+0100 (Central European Standard Time)
 */

#include <asm/u-boot.h>
#include <linux/kernel.h>
#include "ddrs_patch.h"

#define DDRSS_PLL_FHS_CNT 5

#define DDRSS_CTL_268_DATA 0x01010000
#define DDRSS_CTL_270_DATA 0x00000FFF
#define DDRSS_CTL_271_DATA 0x1FFF1000
#define DDRSS_CTL_272_DATA 0x01FF0000
#define DDRSS_CTL_273_DATA 0x000101FF

#define DDRSS_PI_73_DATA 0x00080100

static struct ddr_reg_patch ctl_patch[] = {
	{ 268, DDRSS_CTL_268_DATA},
	{ 270, DDRSS_CTL_270_DATA},
	{ 271, DDRSS_CTL_271_DATA},
	{ 272, DDRSS_CTL_272_DATA},
	{ 273, DDRSS_CTL_273_DATA}
};

static struct ddr_reg_patch pi_patch[] = {
	{ 73, DDRSS_PI_73_DATA},
};

static struct ddrss_patch ddrss_ctrl_patch = {
	.ddr_fhs_cnt = DDRSS_PLL_FHS_CNT,
	.ctl_patch = ctl_patch,
	.ctl_patch_num = ARRAY_SIZE(ctl_patch),
	.pi_patch = pi_patch,
	.pi_patch_num = ARRAY_SIZE(pi_patch),
	.phy_patch = NULL,
	.phy_patch_num = 0
};

struct ddrss_patch *aquila_am69_ddrss_patch_16GB_rank_2[4] = {
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch
};
