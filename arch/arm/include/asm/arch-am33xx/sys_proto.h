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

/*
 * AM335x parts define a system EEPROM that defines certain sub-fields.
 * We use these fields to in turn see what board we are on, and what
 * that might require us to set or not set.
 */
#define HDR_NO_OF_MAC_ADDR	3
#define HDR_ETH_ALEN		6
#define HDR_NAME_LEN		8

struct am335x_baseboard_id {
	unsigned int  magic;
	char name[HDR_NAME_LEN];
	char version[4];
	char serial[12];
	char config[32];
	char mac_addr[HDR_NO_OF_MAC_ADDR][HDR_ETH_ALEN];
};

#define BOARD_REV_ID	0x0

u32 get_cpu_rev(void);
u32 get_sysboot_value(void);

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void);
#endif

extern struct ctrl_stat *cstat;
u32 get_device_type(void);
void setup_clocks_for_console(void);
void ddr_pll_config(unsigned int ddrpll_M);

/*
 * We have three pin mux functions that must exist.  We must be able to enable
 * uart0, for initial output and i2c0 to read the main EEPROM.  We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_i2c0_pin_mux(void);
void enable_board_pin_mux(struct am335x_baseboard_id *header);
#endif
