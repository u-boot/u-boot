/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016-2017 Rockchip Electronics Co., Ltd
 */

#ifndef _ASM_ARCH_SDRAM_RK3399_H
#define _ASM_ARCH_SDRAM_RK3399_H

struct rk3399_ddr_pctl_regs {
	u32 denali_ctl[332];
};

struct rk3399_ddr_publ_regs {
	u32 denali_phy[959];
};

struct rk3399_ddr_pi_regs {
	u32 denali_pi[200];
};

struct rk3399_msch_regs {
	u32 coreid;
	u32 revisionid;
	u32 ddrconf;
	u32 ddrsize;
	u32 ddrtiminga0;
	u32 ddrtimingb0;
	u32 ddrtimingc0;
	u32 devtodev0;
	u32 reserved0[(0x110 - 0x20) / 4];
	u32 ddrmode;
	u32 reserved1[(0x1000 - 0x114) / 4];
	u32 agingx0;
};

struct rk3399_msch_timings {
	u32 ddrtiminga0;
	u32 ddrtimingb0;
	u32 ddrtimingc0;
	u32 devtodev0;
	u32 ddrmode;
	u32 agingx0;
};

struct rk3399_ddr_cic_regs {
	u32 cic_ctrl0;
	u32 cic_ctrl1;
	u32 cic_idle_th;
	u32 cic_cg_wait_th;
	u32 cic_status0;
	u32 cic_status1;
	u32 cic_ctrl2;
	u32 cic_ctrl3;
	u32 cic_ctrl4;
};

/* DENALI_CTL_00 */
#define START		1

/* DENALI_CTL_68 */
#define PWRUP_SREFRESH_EXIT	(1 << 16)

/* DENALI_CTL_274 */
#define MEM_RST_VALID	1

struct rk3399_sdram_channel {
	struct sdram_cap_info cap_info;
	struct rk3399_msch_timings noc_timings;
};

struct rk3399_sdram_params {
	struct rk3399_sdram_channel ch[2];
	struct sdram_base_params base;
	struct rk3399_ddr_pctl_regs pctl_regs;
	struct rk3399_ddr_pi_regs pi_regs;
	struct rk3399_ddr_publ_regs phy_regs;
};

#define PI_CA_TRAINING		(1 << 0)
#define PI_WRITE_LEVELING	(1 << 1)
#define PI_READ_GATE_TRAINING	(1 << 2)
#define PI_READ_LEVELING	(1 << 3)
#define PI_WDQ_LEVELING		(1 << 4)
#define PI_FULL_TRAINING	0xff

#endif
