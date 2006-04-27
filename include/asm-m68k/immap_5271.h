/*
 * MCF5272 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 *               2006 Zachary P. Landau <zachary.landau@labxtechnologies.com>
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

#ifndef __IMMAP_5271__
#define __IMMAP_5271__

/* Interrupt module registers
*/
typedef struct int_ctrl {
	uint	int_icr1;
	uint	int_icr2;
	uint	int_icr3;
	uint	int_icr4;
	uint	int_isr;
	uint	int_pitr;
	uint	int_piwr;
	uchar	res1[3];
	uchar	int_pivr;
} intctrl_t;

/* Timer module registers
 */
typedef struct timer_ctrl {
	ushort	timer_tmr;
	ushort	res1;
	ushort	timer_trr;
	ushort	res2;
	ushort	timer_tcap;
	ushort	res3;
	ushort	timer_tcn;
	ushort	res4;
	ushort	timer_ter;
	uchar	res5[14];
} timer_t;

 /* Fast ethernet controller registers
  */
typedef struct fec {
	uint    res1;
	uint    fec_ievent;
	uint    fec_imask;
	uint    res2;
	uint    fec_r_des_active;
	uint    fec_x_des_active;
	uint    res3[3];
	uint    fec_ecntrl;
	uint    res4[6];
	uint    fec_mii_data;
	uint    fec_mii_speed;
	uint    res5[7];
	uint    fec_mibc;
	uint    res6[7];
	uint    fec_r_cntrl;
	uint    res7[15];
	uint    fec_x_cntrl;
	uint    res8[7];
	uint    fec_addr_low;
	uint    fec_addr_high;
	uint    fec_opd;
	uint    res9[10];
	uint    fec_ihash_table_high;
	uint    fec_ihash_table_low;
	uint    fec_ghash_table_high;
	uint    fec_ghash_table_low;
	uint    res10[7];
	uint    fec_tfwr;
	uint    res11;
	uint    fec_r_bound;
	uint    fec_r_fstart;
	uint    res12[11];
	uint    fec_r_des_start;
	uint    fec_x_des_start;
	uint    fec_r_buff_size;
} fec_t;

#endif /* __IMMAP_5271__ */
