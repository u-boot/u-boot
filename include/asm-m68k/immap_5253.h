/*
 * MCF5253 Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef __IMMAP_5249__
#define __IMMAP_5249__

#define MMAP_INTC		(CFG_MBAR + 0x00000040)
#define MMAP_DTMR0		(CFG_MBAR + 0x00000140)
#define MMAP_DTMR1		(CFG_MBAR + 0x00000180)
#define MMAP_UART0		(CFG_MBAR + 0x000001C0)
#define MMAP_UART1		(CFG_MBAR + 0x00000200)
#define MMAP_I2C0		(CFG_MBAR + 0x00000280)
#define MMAP_QSPI		(CFG_MBAR + 0x00000400)
#define MMAP_CAN0		(CFG_MBAR + 0x00010000)
#define MMAP_CAN1		(CFG_MBAR + 0x00011000)

#define MMAP_I2C1		(CFG_MBAR2 + 0x00000440)
#define MMAP_UART2		(CFG_MBAR2 + 0x00000C00)

/*********************************************************************
* ATA Module (ATAC)
*********************************************************************/

/* Register read/write struct */
typedef struct atac {
	/* PIO */
	u8 toff;		/* 0x00 */
	u8 ton;			/* 0x01 */
	u8 t1;			/* 0x02 */
	u8 t2w;			/* 0x03 */
	u8 t2r;			/* 0x04 */
	u8 ta;			/* 0x05 */
	u8 trd;			/* 0x06 */
	u8 t4;			/* 0x07 */
	u8 t9;			/* 0x08 */

	/* DMA */
	u8 tm;			/* 0x09 */
	u8 tn;			/* 0x0A */
	u8 td;			/* 0x0B */
	u8 tk;			/* 0x0C */
	u8 tack;		/* 0x0D */
	u8 tenv;		/* 0x0E */
	u8 trp;			/* 0x0F */
	u8 tzah;		/* 0x10 */
	u8 tmli;		/* 0x11 */
	u8 tdvh;		/* 0x12 */
	u8 tdzfs;		/* 0x13 */
	u8 tdvs;		/* 0x14 */
	u8 tcvh;		/* 0x15 */
	u8 tss;			/* 0x16 */
	u8 tcyc;		/* 0x17 */

	/* FIFO */
	u32 fifo32;		/* 0x18 */
	u16 fifo16;		/* 0x1C */
	u8 rsvd0[2];
	u8 ffill;		/* 0x20 */
	u8 rsvd1[3];

	/* ATA */
	u8 cr;			/* 0x24 */
	u8 rsvd2[3];
	u8 isr;			/* 0x28 */
	u8 rsvd3[3];
	u8 ier;			/* 0x2C */
	u8 rsvd4[3];
	u8 icr;			/* 0x30 */
	u8 rsvd5[3];
	u8 falarm;		/* 0x34 */
} atac_t;

#endif				/* __IMMAP_5249__ */
