/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#ifndef __SOC_ROCKCHIP_RK3588_GRF_H__
#define __SOC_ROCKCHIP_RK3588_GRF_H__

struct rk3588_pmu1grf {
	unsigned int soc_con[12];
	unsigned int reserved0[(0x0050 - 0x002c) / 4 - 1];
	unsigned int biu_con;
	unsigned int biu_sts;
	unsigned int reserved1[(0x0060 - 0x0054) / 4 - 1];
	unsigned int soc_sts;
	unsigned int reserved2[(0x0080 - 0x0060) / 4 - 1];
	unsigned int mem_con[4];
	unsigned int reserved3[(0x0200 - 0x008c) / 4 - 1];
	unsigned int os_reg[8];
	unsigned int reserved4[(0x0230 - 0x021c) / 4 - 1];
	unsigned int rst_sts;
	unsigned int rst_clr;
	unsigned int reserved5[(0x0380 - 0x0234) / 4 - 1];
	unsigned int sd_detect_con;
	unsigned int reserved6[(0x0390 - 0x0380) / 4 - 1];
	unsigned int sd_detect_sts;
	unsigned int reserved7[(0x03a0 - 0x0390) / 4 - 1];
	unsigned int sd_detect_clr;
	unsigned int reserved8[(0x03b0 - 0x03a0) / 4 - 1];
	unsigned int sd_detect_cnt;
};

check_member(rk3588_pmu1grf, sd_detect_cnt, 0x03b0);

#define SYS_GRF_BASE	0xfd58c000

struct rk3588_sysgrf {
	unsigned int wdt_con0;
	unsigned int reserved0[(0x0010 - 0x0000) / 4 - 1];
	unsigned int uart_con[2];
	unsigned int reserved1[(0x00c0 - 0x0014) / 4 - 1];
	unsigned int gic_con0;
	unsigned int reserved2[(0x0200 - 0x00c0) / 4 - 1];
	unsigned int memcfg_con[32];
	unsigned int reserved3[(0x0300 - 0x027c) / 4 - 1];
	/* soc_con0 is reserved */
	unsigned int soc_con[14];
	unsigned int reserved4[(0x0380 - 0x0334) / 4 - 1];
	unsigned int soc_status[4];
	unsigned int reserved5[(0x0500 - 0x038c) / 4 - 1];
	unsigned int otp_key08;
	unsigned int otp_key0d;
	unsigned int otp_key0e;
	unsigned int reserved6[(0x0600 - 0x0508) / 4 - 1];
	unsigned int chip_id;
};

check_member(rk3588_sysgrf, chip_id, 0x0600);
#endif /*__SOC_ROCKCHIP_RK3588_GRF_H__ */
