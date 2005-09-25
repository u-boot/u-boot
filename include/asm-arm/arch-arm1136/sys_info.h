/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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

#ifndef _OMAP24XX_SYS_INFO_H_
#define _OMAP24XX_SYS_INFO_H_

typedef struct  h4_system_data {
	/* base board info */
	u32 base_b_rev;		/* rev from base board i2c */
	/* cpu board info */
	u32 cpu_b_rev;		/* rev from cpu board i2c */
	u32 cpu_b_mux;		/* mux type on daughter board */
	u32 cpu_b_ddr_type;	/* mem type */
	u32 cpu_b_ddr_speed;	/* ddr speed rating */
	u32 cpu_b_switches;	/* boot ctrl switch settings */
	/* cpu info */
	u32 cpu_type;		/* type of cpu; 2420, 2422, 2430,...*/
	u32 cpu_rev;		/* rev of given cpu; ES1, ES2,...*/
} h4_sys_data;

#define XDR_POP           5      /* package on package part */
#define SDR_DISCRETE      4      /* 128M memory SDR module*/
#define DDR_STACKED       3      /* stacked part on 2422 */
#define DDR_COMBO         2      /* combo part on cpu daughter card (menalaeus) */
#define DDR_DISCRETE      1      /* 2x16 parts on daughter card */

#define DDR_100           100    /* type found on most mem d-boards */
#define DDR_111           111    /* some combo parts */
#define DDR_133           133    /* most combo, some mem d-boards */
#define DDR_165           165    /* future parts */

#define CPU_2420          0x2420
#define CPU_2422          0x2422 /* 2420 + 64M stacked */
#define CPU_2423          0x2423 /* 2420 + 96M stacked */

#define CPU_2422_ES1      1
#define CPU_2422_ES2      2
#define CPU_2420_ES1      1
#define CPU_2420_ES2      2
#define CPU_2420_2422_ES1 1

#define CPU_2420_CHIPID   0x0B5D9000
#define CPU_24XX_ID_MASK  0x0FFFF000
#define CPU_242X_REV_MASK 0xF0000000
#define CPU_242X_PID_MASK 0x000F0000

#define BOARD_H4_MENELAUS 1
#define BOARD_H4_SDP      2

#define GPMC_MUXED        1
#define GPMC_NONMUXED     0

#define TYPE_NAND         0x800   /* bit pos for nand in gpmc reg */
#define TYPE_NOR          0x000

#define WIDTH_8BIT        0x0000
#define WIDTH_16BIT       0x1000  /* bit pos for 16 bit in gpmc */

#define I2C_MENELAUS 0x72	/* i2c id for companion chip */

#endif
