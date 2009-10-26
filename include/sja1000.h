/*
 * Copyright 2009, Matthias Fuchs <matthias.fuchs@esd.eu>
 *
 * SJA1000 register layout for basic CAN mode
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

#ifndef _SJA1000_H_
#define _SJA1000_H_

/*
 * SJA1000 register layout in basic can mode
 */
struct sja1000_basic_s {
	u8 cr;
	u8 cmr;
	u8 sr;
	u8 ir;
	u8 ac;
	u8 am;
	u8 btr0;
	u8 btr1;
	u8 oc;
	u8 txb[10];
	u8 rxb[10];
	u8 unused;
	u8 cdr;
};

/* control register */
#define CR_RR		0x01

/* output control register */
#define OC_MODE0	0x01
#define OC_MODE1	0x02
#define OC_POL0		0x04
#define OC_TN0		0x08
#define OC_TP0		0x10
#define OC_POL1		0x20
#define OC_TN1		0x40
#define OC_TP1		0x80

#endif
