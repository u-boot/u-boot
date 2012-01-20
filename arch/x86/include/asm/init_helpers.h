/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _INIT_HELPERS_H_
#define _INIT_HELPERS_H_

int display_banner(void);
int display_dram_config(void);
int init_baudrate_f(void);
int calculate_relocation_address(void);

int copy_gd_to_ram_f_r(void);
int init_cache_f_r(void);

int set_reloc_flag_r(void);
int mem_malloc_init_r(void);
int init_bd_struct_r(void);
int flash_init_r(void);
int init_ip_address_r(void);
int status_led_set_r(void);
int set_bootfile_r(void);
int set_load_addr_r(void);

#endif	/* !_INIT_HELPERS_H_ */
