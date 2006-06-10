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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __IMMAP_5282__
#define __IMMAP_5282__

struct sys_ctrl {
	uint ipsbar;
	char res1[4];
	uint rambar;
	char res2[4];
	uchar crsr;
	uchar cwcr;
	uchar lpicr;
	uchar cwsr;
	uint dmareqc;
	char res3[4];
	uint mpark;

    /* TODO: finish these */
};

/* Fast ethernet controller registers
 */
typedef struct fec {
	uint	res1;		/* reserved			1000*/
	uint	fec_ievent;	/* interrupt event register	1004*/	/* EIR */
	uint	fec_imask;	/* interrupt mask register	1008*/	/* EIMR */
	uint	res2;		/* reserved			100c*/
	uint	fec_r_des_active;    /* Rx ring updated flag	1010*/	/* RDAR */
	uint	fec_x_des_active;    /* Tx ring updated flag	1014*/	/* XDAR */
	uint	res3[3];	/* reserved			1018*/
	uint	fec_ecntrl;	/* ethernet control register	1024*/	/* ECR */
	uint	res4[6];	/* reserved			1028*/
	uint	fec_mii_data;	/* MII data register		1040*/	/* MDATA */
	uint	fec_mii_speed;	/* MII speed control register	1044*/	/* MSCR */
				      /*1044*/
	uint	res5[7];	/* reserved			1048*/
	uint	fec_mibc;	/* MIB Control/Status register	1064*/ /* MIBC */
	uint	res6[7];	/* reserved			1068*/
	uint	fec_r_cntrl;	/* Rx control register		1084*/	/* RCR */
	uint	res7[15];	/* reserved			1088*/
	uint	fec_x_cntrl;	/* Tx control register		10C4*/	/* TCR */
	uint	res8[7];	/* reserved			10C8*/
	uint	fec_addr_low;	/* lower 32 bits of station address */	/* PALR */
	uint	fec_addr_high;	/* upper 16 bits of station address  */ /* PAUR */
	uint	fec_opd;	/* opcode + pause duration	10EC*/	/* OPD */
	uint	res9[10];	/* reserved			10F0*/
	uint	fec_ihash_table_high;	/* upper 32-bits of individual hash */ /* IAUR */
	uint	fec_ihash_table_low;	/* lower 32-bits of individual hash */ /* IALR */
	uint	fec_ghash_table_high;	/* upper 32-bits of group hash	*/ /* GAUR */
	uint	fec_ghash_table_low;	/* lower 32-bits of group hash	*/ /* GALR */
	uint	res10[7];	/* reserved			1128*/
	uint	fec_tfwr;	/* Transmit FIFO watermark	1144*/	/* TFWR */
	uint	res11;		/* reserved			1148*/
	uint	fec_r_bound;	/* FIFO Receive Bound Register = end of */ /* FRBR */
	uint	fec_r_fstart;	/* FIFO Receive FIfo Start Registers =	*/ /* FRSR */
	uint	res12[11];	/* reserved			1154*/
	uint	fec_r_des_start;/* beginning of Rx descriptor ring    1180*/ /* ERDSR */
	uint	fec_x_des_start;/* beginning of Tx descriptor ring    1184*/ /* ETDSR */
	uint	fec_r_buff_size;/* Rx buffer size		1188*/	/* EMRBR */
} fec_t;

#endif /* __IMMAP_5282__ */
