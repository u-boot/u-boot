/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Timer
 */
#ifndef __FTTMR010_H
#define __FTTMR010_H

struct fttmr010 {
	unsigned int	timer1_counter;		/* 0x00 */
	unsigned int	timer1_load;		/* 0x04 */
	unsigned int	timer1_match1;		/* 0x08 */
	unsigned int	timer1_match2;		/* 0x0c */
	unsigned int	timer2_counter;		/* 0x10 */
	unsigned int	timer2_load;		/* 0x14 */
	unsigned int	timer2_match1;		/* 0x18 */
	unsigned int	timer2_match2;		/* 0x1c */
	unsigned int	timer3_counter;		/* 0x20 */
	unsigned int	timer3_load;		/* 0x24 */
	unsigned int	timer3_match1;		/* 0x28 */
	unsigned int	timer3_match2;		/* 0x2c */
	unsigned int	cr;			/* 0x30 */
	unsigned int	interrupt_state;	/* 0x34 */
	unsigned int	interrupt_mask;		/* 0x38 */
};

/*
 * Timer Control Register
 */
#define FTTMR010_TM3_UPDOWN	(1 << 11)
#define FTTMR010_TM2_UPDOWN	(1 << 10)
#define FTTMR010_TM1_UPDOWN	(1 << 9)
#define FTTMR010_TM3_OFENABLE	(1 << 8)
#define FTTMR010_TM3_CLOCK	(1 << 7)
#define FTTMR010_TM3_ENABLE	(1 << 6)
#define FTTMR010_TM2_OFENABLE	(1 << 5)
#define FTTMR010_TM2_CLOCK	(1 << 4)
#define FTTMR010_TM2_ENABLE	(1 << 3)
#define FTTMR010_TM1_OFENABLE	(1 << 2)
#define FTTMR010_TM1_CLOCK	(1 << 1)
#define FTTMR010_TM1_ENABLE	(1 << 0)

/*
 * Timer Interrupt State & Mask Registers
 */
#define FTTMR010_TM3_OVERFLOW	(1 << 8)
#define FTTMR010_TM3_MATCH2	(1 << 7)
#define FTTMR010_TM3_MATCH1	(1 << 6)
#define FTTMR010_TM2_OVERFLOW	(1 << 5)
#define FTTMR010_TM2_MATCH2	(1 << 4)
#define FTTMR010_TM2_MATCH1	(1 << 3)
#define FTTMR010_TM1_OVERFLOW	(1 << 2)
#define FTTMR010_TM1_MATCH2	(1 << 1)
#define FTTMR010_TM1_MATCH1	(1 << 0)

#endif	/* __FTTMR010_H */
