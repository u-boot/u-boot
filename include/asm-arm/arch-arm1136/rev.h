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

#ifndef _OMAP24XX_REV_H_
#define _OMAP24XX_REV_H_

typedef	struct	h4_system_data {
    /* base board info */
    u32	   base_b_rev;         /* rev from base board i2c */
    /* cpu board info */
    u32	   cpu_b_rev;          /* rev from cpu board i2c */
    u32    cpu_b_mux;          /* mux type on daughter board */
    u32    cpu_b_ddr_type;     /* mem type */
    u32    cpu_b_ddr_speed;    /* ddr speed rating */
    u32    cpu_b_switches;     /* boot ctrl switch settings */
    /* cpu info */
    u32	   cpu_type;           /* type of cpu; 2420, 2422, 2430,...*/
    u32    cpu_rev;            /* rev of given cpu; ES1, ES2,...*/
} h4_sys_data;

#define CDB_DDR_COMBO                   /* combo part on cpu daughter card */
#define CDB_DDR_IPDB                    /* 2x16 parts on daughter card */

#define DDR_100         100             /* type found on most mem d-boards */
#define DDR_111         111             /* some combo parts */
#define DDR_133         133             /* most combo, some mem d-boards */
#define DDR_165         165             /* future parts */

#define CPU_2420        0x2420
#define CPU_2422        0x2422

#define CPU_2422_ES1    1
#define CPU_2422_ES2    2
#define CPU_2420_ES1    1
#define CPU_2420_ES2    2

#endif
