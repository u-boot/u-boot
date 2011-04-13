/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _PANTHEON_CPU_H
#define _PANTHEON_CPU_H

#include <asm/io.h>
#include <asm/system.h>

/*
 * Main Power Management (MPMU) Registers
 * Refer Register Datasheet 9.1
 */
struct panthmpmu_registers {
	u8 pad0[0x0024];
	u32 ccgr;	/*0x0024*/
	u8 pad1[0x0200 - 0x024 - 4];
	u32 wdtpcr;	/*0x0200*/
	u8 pad2[0x1020 - 0x200 - 4];
	u32 aprr;	/*0x1020*/
	u32 acgr;	/*0x1024*/
};

/*
 * APB Clock Reset/Control Registers
 * Refer Register Datasheet 6.14
 */
struct panthapb_registers {
	u32 uart0;	/*0x000*/
	u32 uart1;	/*0x004*/
	u32 gpio;	/*0x008*/
	u8 pad0[0x02c - 0x08 - 4];
	u32 twsi;	/*0x02c*/
	u8 pad1[0x034 - 0x2c - 4];
	u32 timers;	/*0x034*/
};

/*
 * CPU Interface Registers
 * Refer Register Datasheet 4.3
 */
struct panthcpu_registers {
	u32 chip_id;		/* Chip Id Reg */
	u32 pad;
	u32 cpu_conf;		/* CPU Conf Reg */
	u32 pad1;
	u32 cpu_sram_spd;	/* CPU SRAM Speed Reg */
	u32 pad2;
	u32 cpu_l2c_spd;	/* CPU L2cache Speed Conf */
	u32 mcb_conf;		/* MCB Conf Reg */
	u32 sys_boot_ctl;	/* Sytem Boot Control */
};

/*
 * Functions
 */
u32 panth_sdram_base(int);
u32 panth_sdram_size(int);

#endif /* _PANTHEON_CPU_H */
