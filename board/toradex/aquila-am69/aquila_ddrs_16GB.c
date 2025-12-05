// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 Toradex - https://www.toradex.com/
 * This contains a diff against the 32GB register settings created from the 16GB tool output.

 * The 16GB dtsi file was generated with the following tool revisions:
 *     - SysConfig: Revision 1.23.1+4034
 *     - Jacinto7_DDRSS_RegConfigTool: Revision 0.11.0
 * It was generated on Fri May 16 2025 17:28:31 GMT+0200 (Central European Summer Time)
 */

#include <asm/u-boot.h>
#include <linux/kernel.h>
#include "ddrs_patch.h"

#define DDRSS_PLL_FHS_CNT 3

#define DDRSS_CTL_267_DATA 0x01010101
#define DDRSS_CTL_271_DATA 0xFFFFFFFF
#define DDRSS_CTL_273_DATA 0x000003FF
#define DDRSS_CTL_278_DATA 0x00010000
#define DDRSS_CTL_287_DATA 0x00000100

#define DDRSS_PI_14_DATA 0x08000005
#define DDRSS_PI_29_DATA 0x05000000
#define DDRSS_PI_45_DATA 0x00050500
#define DDRSS_PI_55_DATA 0x05000000
#define DDRSS_PI_67_DATA 0x00020205

static struct ddr_reg_patch ctl_patch[] = {
	{ 267, DDRSS_CTL_267_DATA},
	{ 271, DDRSS_CTL_271_DATA},
	{ 273, DDRSS_CTL_273_DATA},
	{ 278, DDRSS_CTL_278_DATA},
	{ 287, DDRSS_CTL_287_DATA}
};

static struct ddr_reg_patch pi_patch[] = {
	{ 14, DDRSS_PI_14_DATA},
	{ 29, DDRSS_PI_29_DATA},
	{ 45, DDRSS_PI_45_DATA},
	{ 55, DDRSS_PI_55_DATA},
	{ 67, DDRSS_PI_67_DATA},
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

struct ddrss_patch *aquila_am69_ddrss_patch_16GB[4] = {
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch
};
