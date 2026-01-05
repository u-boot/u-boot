// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 Toradex - https://www.toradex.com/
 * This contains a diff against the 32GB register settings created from the 8GB tool output.

 * The 8GB dtsi file was generated with the following tool revisions:
 *     - SysConfig: Revision 1.23.1+4034
 *     - Jacinto7_DDRSS_RegConfigTool: Revision 0.11.0
 * It was generated on Thu May 15 2025 10:35:37 GMT+0200 (Central European Summer Time)
 */

#include <asm/u-boot.h>
#include <linux/kernel.h>
#include "ddrs_patch.h"

#define DDRSS_PLL_FHS_CNT 3

#define DDRSS_CTL_60_DATA 0x00001008
#define DDRSS_CTL_62_DATA 0x00000256
#define DDRSS_CTL_64_DATA 0x00000256
#define DDRSS_CTL_67_DATA 0x00040000
#define DDRSS_CTL_68_DATA 0x00950005
#define DDRSS_CTL_69_DATA 0x00950200
#define DDRSS_CTL_79_DATA 0x00100010
#define DDRSS_CTL_80_DATA 0x02660266
#define DDRSS_CTL_81_DATA 0x02660266
#define DDRSS_CTL_267_DATA 0x01010101
#define DDRSS_CTL_268_DATA 0x01010000
#define DDRSS_CTL_270_DATA 0x00000FFF
#define DDRSS_CTL_271_DATA 0xFFFFFFFF
#define DDRSS_CTL_272_DATA 0x01FF0000
#define DDRSS_CTL_273_DATA 0x000001FF
#define DDRSS_CTL_278_DATA 0x00010000
#define DDRSS_CTL_287_DATA 0x00000100

#define DDRSS_PI_14_DATA 0x08000005
#define DDRSS_PI_29_DATA 0x05000000
#define DDRSS_PI_45_DATA 0x00050500
#define DDRSS_PI_55_DATA 0x05000000
#define DDRSS_PI_67_DATA 0x00020205
#define DDRSS_PI_73_DATA 0x00080100
#define DDRSS_PI_169_DATA 0x00105012
#define DDRSS_PI_171_DATA 0x00000256
#define DDRSS_PI_173_DATA 0x00000256
#define DDRSS_PI_235_DATA 0x02660010
#define DDRSS_PI_236_DATA 0x03030266
#define DDRSS_PI_241_DATA 0x00000010
#define DDRSS_PI_246_DATA 0x00000266
#define DDRSS_PI_251_DATA 0x01000266

static struct ddr_reg_patch ctl_patch[] = {
	{ 60, DDRSS_CTL_60_DATA},
	{ 62, DDRSS_CTL_62_DATA},
	{ 64, DDRSS_CTL_64_DATA},
	{ 67, DDRSS_CTL_67_DATA},
	{ 68, DDRSS_CTL_68_DATA},
	{ 69, DDRSS_CTL_69_DATA},
	{ 79, DDRSS_CTL_79_DATA},
	{ 80, DDRSS_CTL_80_DATA},
	{ 81, DDRSS_CTL_81_DATA},
	{ 267, DDRSS_CTL_267_DATA},
	{ 268, DDRSS_CTL_268_DATA},
	{ 270, DDRSS_CTL_270_DATA},
	{ 271, DDRSS_CTL_271_DATA},
	{ 272, DDRSS_CTL_272_DATA},
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
	{ 73, DDRSS_PI_73_DATA},
	{ 169, DDRSS_PI_169_DATA},
	{ 171, DDRSS_PI_171_DATA},
	{ 173, DDRSS_PI_173_DATA},
	{ 235, DDRSS_PI_235_DATA},
	{ 236, DDRSS_PI_236_DATA},
	{ 241, DDRSS_PI_241_DATA},
	{ 246, DDRSS_PI_246_DATA},
	{ 251, DDRSS_PI_251_DATA}
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

struct ddrss_patch *aquila_am69_ddrss_patch_8GB[4] = {
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch,
	&ddrss_ctrl_patch
};
