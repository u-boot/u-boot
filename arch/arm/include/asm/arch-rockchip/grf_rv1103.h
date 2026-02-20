/* SPDX-License-Identifier:     GPL-2.0+ */

/*
 * (C) Copyright 2024 Rockchip Electronics Co., Ltd.
 */

#ifndef _ASM_ARCH_GRF_RV1103B_H
#define _ASM_ARCH_GRF_RV1103B_H

#define VEPU_GRF	0x20100000
#define NPU_GRF		0x20110000
#define VI_GRF		0x20120000
#define CPU_GRF		0x20130000
#define DDR_GRF		0x20140000
#define SYS_GRF		0x20150000
#define PMU_GRF		0x20160000

struct rv1103_grf {
	u32 reserved0[(SYS_GRF + 0xA0 - VEPU_GRF) / 4];
	u32 gmac_con0;				/* address offset: 0x00a0 */
	u32 gmac_clk_con;			/* address offset: 0x00a4 */
	u32 gmac_st;				/* address offset: 0x00a8 */
	u32 reserved00ac;			/* address offset: 0x00ac */
	u32 macphy_con0;			/* address offset: 0x00b0 */
	u32 macphy_con1;			/* address offset: 0x00b4 */
	u32 reserved1[(PMU_GRF + 0x10000 - (SYS_GRF + 0xB4)) / 4];
};

check_member(rv1103_grf, macphy_con1, SYS_GRF + 0xB4 - VEPU_GRF);

#endif /*  _ASM_ARCH_GRF_RV1103B_H  */
