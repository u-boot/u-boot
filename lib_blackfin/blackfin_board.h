/*
 * U-boot - blackfin_board.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef __BLACKFIN_BOARD_H__
#define __BLACKFIN_BOARD_H__

extern void timer_init(void);
extern void init_IRQ(void);
extern void rtc_init(void);

extern ulong uboot_end_data;
extern ulong uboot_end;

ulong monitor_flash_len;


#define VERSION_STRING_SIZE  150 /* including 40 bytes buffer to change any string */
#define VERSION_STRING_FORMAT "%s (%s - %s)\n"
#define VERSION_STRING		U_BOOT_VERSION, __DATE__, __TIME__

char version_string[VERSION_STRING_SIZE];

int *g_addr;
static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;
extern char _sram_in_sdram_start[];
extern char _sram_inst_size[];
#ifdef DEBUG
static void display_global_data(void);
#endif

/* definitions used to check the SMC card availability */
#define SMC_BASE_ADDRESS CONFIG_SMC91111_BASE
#define UPPER_BYTE_MASK	0xFF00
#define SMC_IDENT	0x3300

#endif
