/*
 * MCF5282 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
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

#ifndef __IMMAP_5282__
#define __IMMAP_5282__


/* Fast ethernet controller registers
 */
typedef struct fec {
	uint	fec_ecntrl;		/* ethernet control register		*/
	uint	fec_ievent;		/* interrupt event register		*/
	uint	fec_imask;		/* interrupt mask register		*/
	uint	fec_ivec;		/* interrupt level and vector status	*/
	uint	fec_r_des_active;	/* Rx ring updated flag			*/
	uint	fec_x_des_active;	/* Tx ring updated flag			*/
	uint	res3[10];		/* reserved				*/
	uint	fec_mii_data;		/* MII data register			*/
	uint	fec_mii_speed;		/* MII speed control register		*/
	uint	res4[17];		/* reserved				*/
	uint	fec_r_bound;		/* end of RAM (read-only)		*/
	uint	fec_r_fstart;		/* Rx FIFO start address		*/
	uint	res5[6];		/* reserved				*/
	uint	fec_x_fstart;		/* Tx FIFO start address		*/
	uint	res7[21];		/* reserved				*/
	uint	fec_r_cntrl;		/* Rx control register			*/
	uint	fec_r_hash;		/* Rx hash register			*/
	uint	res8[14];		/* reserved				*/
	uint	fec_x_cntrl;		/* Tx control register			*/
	uint	res9[0x9e];		/* reserved				*/
	uint	fec_addr_low;		/* lower 32 bits of station address	*/
	uint	fec_addr_high;		/* upper 16 bits of station address	*/
	uint	fec_hash_table_high;	/* upper 32-bits of hash table		*/
	uint	fec_hash_table_low;	/* lower 32-bits of hash table		*/
	uint	fec_r_des_start;	/* beginning of Rx descriptor ring	*/
	uint	fec_x_des_start;	/* beginning of Tx descriptor ring	*/
	uint	fec_r_buff_size;	/* Rx buffer size			*/
	uint	res2[9];		/* reserved				*/
	uchar	fec_fifo[960];		/* fifo RAM				*/
} fec_t;

#endif /* __IMMAP_5282__ */
