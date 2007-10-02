/*
 * Copyright 2007 Freescale Semiconductor.
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

#include <common.h>
#include "bcsr.h"

void enable_8568mds_duart()
{
	volatile uint* duart_mux	= (uint *)(CFG_CCSRBAR + 0xe0060);
	volatile uint* devices		= (uint *)(CFG_CCSRBAR + 0xe0070);
	volatile u8 *bcsr		= (u8 *)(CFG_BCSR);

	*duart_mux = 0x80000000;	/* Set the mux to Duart on PMUXCR */
	*devices  = 0;			/* Enable all peripheral devices */
	bcsr[5] |= 0x01;		/* Enable Duart in BCSR*/
}

void enable_8568mds_flash_write()
{
	volatile u8 *bcsr = (u8 *)(CFG_BCSR);

	bcsr[9] |= 0x01;
}

void disable_8568mds_flash_write()
{
	volatile u8 *bcsr = (u8 *)(CFG_BCSR);

	bcsr[9] &= ~(0x01);
}

void enable_8568mds_qe_mdio()
{
	u8 *bcsr = (u8 *)(CFG_BCSR);

	bcsr[7] |= 0x01;
}
