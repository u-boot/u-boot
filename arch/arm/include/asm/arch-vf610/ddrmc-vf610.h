/*
 * Copyright (C) 2015
 * Toradex, Inc.
 *
 * Authors: Stefan Agner
 *          Sanchayan Maity
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_VF610_DDRMC_H
#define __ASM_ARCH_VF610_DDRMC_H

struct ddrmc_lvl_info {
	u16 wrlvl_reg_en;
	u16 wrlvl_dl_0;
	u16 wrlvl_dl_1;
	u16 rdlvl_gt_reg_en;
	u16 rdlvl_gt_dl_0;
	u16 rdlvl_gt_dl_1;
	u16 rdlvl_reg_en;
	u16 rdlvl_dl_0;
	u16 rdlvl_dl_1;
};

struct ddr3_jedec_timings {
	u8 tinit;
	u32 trst_pwron;
	u32 cke_inactive;
	u8 wrlat;
	u8 caslat_lin;
	u8 trc;
	u8 trrd;
	u8 tccd;
	u8 tfaw;
	u8 trp;
	u8 twtr;
	u8 tras_min;
	u8 tmrd;
	u8 trtp;
	u32 tras_max;
	u8 tmod;
	u8 tckesr;
	u8 tcke;
	u8 trcd_int;
	u8 tdal;
	u16 tdll;
	u8 trp_ab;
	u16 tref;
	u8 trfc;
	u8 tpdex;
	u8 txpdll;
	u8 txsnr;
	u16 txsr;
	u8 cksrx;
	u8 cksre;
	u16 zqcl;
	u16 zqinit;
	u8 zqcs;
	u8 ref_per_zq;
	u8 aprebit;
	u8 wlmrd;
	u8 wldqsen;
};

void ddrmc_setup_iomux(void);
void ddrmc_phy_init(void);
void ddrmc_ctrl_init_ddr3(struct ddr3_jedec_timings const *timings,
						  struct ddrmc_lvl_info *lvl,
						  int col_diff, int row_diff);

#endif
