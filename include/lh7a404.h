/*
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

/*
 * lh7a404 SoC interface
 */

#ifndef __LH7A404_H__
#define __LH7A404_H__

#include "lh7a40x.h"

/* Interrupt Controller (userguide 8.2.1) */
typedef struct {
	volatile u32  irqstatus;
	volatile u32  fiqstatus;
	volatile u32  rawintr;
	volatile u32  intsel;
	volatile u32  inten;
	volatile u32  intenclr;
	volatile u32  softint;
	volatile u32  softintclr;
	volatile u32  protect;
	volatile u32  unused1;
	volatile u32  unused2;
	volatile u32  vectaddr;
	volatile u32  nvaddr;
	volatile u32  unused3[32];
	volatile u32  vad[16];
	volatile u32  unused4[44];
	volatile u32  vectcntl[16];
	volatile u32  unused5[44];
	volatile u32  itcr;
	volatile u32  itip1;
	volatile u32  itip2;
	volatile u32  itop1;
	volatile u32  itop2;
	volatile u32  unused6[333];
	volatile u32  periphid[4];
	volatile u32  pcellid[4];
} /*__attribute__((__packed__))*/ lh7a404_vic_t;
#define LH7A404_VIC_BASE	(0x80008000)
#define LH7A400_VIC_PTR(x)  ((lh7a404_vic_t*)(LH7A400_VIC_BASE + (x*0x2000)))


typedef struct {
	lh7a40x_dmachan_t  m2p0_tx;
	lh7a40x_dmachan_t  m2p1_rx;
	lh7a40x_dmachan_t  m2p2_tx;
	lh7a40x_dmachan_t  m2p3_rx;
	lh7a40x_dmachan_t  m2m0;
	lh7a40x_dmachan_t  m2m1;
	lh7a40x_dmachan_t  unused1;
	lh7a40x_dmachan_t  unused2;
	lh7a40x_dmachan_t  m2p5_rx;
	lh7a40x_dmachan_t  m2p4_tx;
	lh7a40x_dmachan_t  m2p7_rx;
	lh7a40x_dmachan_t  m2p6_tx;
	lh7a40x_dmachan_t  m2p9_rx;
	lh7a40x_dmachan_t  m2p8_tx;
	volatile u32       chanarb;
	volatile u32       glblint;
} /*__attribute__((__packed__))*/ lh7a400_dma_t;


#endif  /* __LH7A404_H__ */
