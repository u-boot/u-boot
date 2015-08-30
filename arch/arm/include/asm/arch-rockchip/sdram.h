/*
 * Copyright (c) 2015 Google, Inc
 *
 * Copyright 2014 Rockchip Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ASM_ARCH_RK3288_SDRAM_H__
#define _ASM_ARCH_RK3288_SDRAM_H__

enum {
	DDR3 = 3,
	LPDDR3 = 6,
	UNUSED = 0xFF,
};

struct rk3288_sdram_channel {
	u8 rank;
	u8 col;
	u8 bk;
	u8 bw;
	u8 dbw;
	u8 row_3_4;
	u8 cs0_row;
	u8 cs1_row;
};

struct rk3288_sdram_pctl_timing {
	u32 togcnt1u;
	u32 tinit;
	u32 trsth;
	u32 togcnt100n;
	u32 trefi;
	u32 tmrd;
	u32 trfc;
	u32 trp;
	u32 trtw;
	u32 tal;
	u32 tcl;
	u32 tcwl;
	u32 tras;
	u32 trc;
	u32 trcd;
	u32 trrd;
	u32 trtp;
	u32 twr;
	u32 twtr;
	u32 texsr;
	u32 txp;
	u32 txpdll;
	u32 tzqcs;
	u32 tzqcsi;
	u32 tdqs;
	u32 tcksre;
	u32 tcksrx;
	u32 tcke;
	u32 tmod;
	u32 trstl;
	u32 tzqcl;
	u32 tmrr;
	u32 tckesr;
	u32 tdpd;
};
check_member(rk3288_sdram_pctl_timing, tdpd, 0x144 - 0xc0);

struct rk3288_sdram_phy_timing {
	u32 dtpr0;
	u32 dtpr1;
	u32 dtpr2;
	u32 mr[4];
};

struct rk3288_base_params {
	u32 noc_timing;
	u32 noc_activate;
	u32 ddrconfig;
	u32 ddr_freq;
	u32 dramtype;
	u32 stride;
	u32 odt;
};

struct rk3288_sdram_params {
	struct rk3288_sdram_channel ch[2];
	struct rk3288_sdram_pctl_timing pctl_timing;
	struct rk3288_sdram_phy_timing phy_timing;
	struct rk3288_base_params base;
	int num_channels;
};

#endif
