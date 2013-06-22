/*
 * sys_proto.h
 *
 * System information header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#define BOARD_REV_ID	0x0

u32 get_cpu_rev(void);
u32 get_sysboot_value(void);

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void);
#endif

extern struct ctrl_stat *cstat;
u32 get_device_type(void);
void save_omap_boot_params(void);
void setup_clocks_for_console(void);
void mpu_pll_config_val(int mpull_m);
void ddr_pll_config(unsigned int ddrpll_M);

void sdelay(unsigned long);

struct gpmc_cs;
void gpmc_init(void);
void enable_gpmc_cs_config(const u32 *gpmc_config, struct gpmc_cs *cs, u32 base,
			u32 size);
void omap_nand_switch_ecc(uint32_t, uint32_t);

void rtc32k_enable(void);
void uart_soft_reset(void);
#endif
