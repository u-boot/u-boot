/*
 * Freescale non-CPM SPI Controller
 *
 * Copyright 2008 Qstreams Networks, Inc.
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifndef _ASM_MPC8XXX_SPI_H_
#define _ASM_MPC8XXX_SPI_H_

#include <asm/types.h>

#if defined(CONFIG_MPC834X) || \
	defined(CONFIG_MPC8313) || \
	defined(CONFIG_MPC8315) || \
	defined(CONFIG_MPC837X)

typedef struct spi8xxx {
	u8 res0[0x20];	/* 0x0-0x01f reserved */
	u32 mode;	/* mode register  */
	u32 event;	/* event register */
	u32 mask;	/* mask register  */
	u32 com;	/* command register */
	u32 tx;		/* transmit register */
	u32 rx;		/* receive register */
	u8 res1[0xFC8];	/* fill up to 0x1000 */
} spi8xxx_t;

#endif

#endif	/* _ASM_MPC8XXX_SPI_H_ */
