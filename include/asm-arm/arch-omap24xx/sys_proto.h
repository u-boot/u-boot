/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
  */
#ifndef _OMAP24XX_SYS_PROTO_H_
#define _OMAP24XX_SYS_PROTO_H_

void prcm_init(void);
void memif_init(void);
void sdrc_init(void);
void do_sdrc_init(u32,u32);
void gpmc_init(void);

void ether_init(void);
void watchdog_init(void);
void set_muxconf_regs(void);
void peripheral_enable(void);

u32 get_cpu_type(void);
u32 get_cpu_rev(void);
u32 get_mem_type(void);
u32 get_sysboot_value(void);
u32 get_gpmc0_base(void);
u32 is_gpmc_muxed(void);
u32 get_gpmc0_type(void);
u32 get_gpmc0_width(void);
u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound);
u32 get_board_type(void);
void display_board_info(u32);
void update_mux(u32,u32);
u32 get_sdr_cs_size(u32 offset);

u32 running_in_sdram(void);
u32 running_in_sram(void);
u32 running_in_flash(void);
u32 running_from_internal_boot(void);
u32 get_device_type(void);
#endif
