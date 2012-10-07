/*
 * Copyright 2011 Freescale Semiconductor
 * Author: Shengzhou Liu <Shengzhou.Liu@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This file provides support for the QIXIS of some Freescale reference boards.
 */

#ifndef __QIXIS_H_
#define __QIXIS_H_

struct qixis {
	u8 id;      /* ID value uniquely identifying each QDS board type */
	u8 arch;    /* Board version information */
	u8 scver;   /* QIXIS Version Register */
	u8 model;   /* Information of software programming model version */
	u8 tagdata;
	u8 ctl_sys;
	u8 aux;         /* Auxiliary Register,0x06 */
	u8 clk_spd;
	u8 stat_dut;
	u8 stat_sys;
	u8 stat_alrm;
	u8 present;
	u8 present2;    /* Presence Status Register 2,0x0c */
	u8 rcw_ctl;
	u8 ctl_led;
	u8 i2cblk;
	u8 rcfg_ctl;    /* Reconfig Control Register,0x10 */
	u8 rcfg_st;
	u8 dcm_ad;
	u8 dcm_da;
	u8 dcmd;
	u8 dmsg;
	u8 gdc;
	u8 gdd;         /* DCM Debug Data Register,0x17 */
	u8 dmack;
	u8 res1[6];
	u8 watch;       /* Watchdog Register,0x1F */
	u8 pwr_ctl[2];  /* Power Control Register,0x20 */
	u8 res2[2];
	u8 pwr_stat[4]; /* Power Status Register,0x24 */
	u8 res3[8];
	u8 clk_spd2[2];  /* SYSCLK clock Speed Register,0x30 */
	u8 res4[2];
	u8 sclk[3];  /* Clock Configuration Registers,0x34 */
	u8 res5;
	u8 dclk[3];
	u8 res6;
	u8 clk_dspd[3];
	u8 res7;
	u8 rst_ctl;     /* Reset Control Register,0x40 */
	u8 rst_stat;    /* Reset Status Register */
	u8 rst_rsn;     /* Reset Reason Register */
	u8 rst_frc[2];  /* Reset Force Registers,0x43 */
	u8 res8[11];
	u8 brdcfg[16];  /* Board Configuration Register,0x50 */
	u8 dutcfg[16];
	u8 rcw_ad[2];   /* RCW SRAM Address Registers,0x70 */
	u8 rcw_data;
	u8 res9[5];
	u8 post_ctl;
	u8 post_stat;
	u8 post_dat[2];
	u8 pi_d[4];
	u8 gpio_io[4];
	u8 gpio_dir[4];
	u8 res10[20];
	u8 rjtag_ctl;
	u8 rjtag_dat;
	u8 res11[2];
	u8 trig_src[4];
	u8 trig_dst[4];
	u8 trig_stat;
	u8 res12[3];
	u8 trig_ctr[4];
	u8 res13[48];
	u8 aux2[4];	/* Auxiliary Registers,0xE0 */
	u8 res14[10];
	u8 aux_ad;
	u8 aux_da;
	u8 res15[16];
};

u8 qixis_read(unsigned int reg);
void qixis_write(unsigned int reg, u8 value);

#define QIXIS_READ(reg) qixis_read(offsetof(struct qixis, reg))
#define QIXIS_WRITE(reg, value) qixis_write(offsetof(struct qixis, reg), value)

#endif
