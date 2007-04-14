/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * Ethernet initialization for the AVR32 Network Gateway
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
#include <common.h>

#include <asm/arch/memory-map.h>

extern int macb_eth_initialize(int id, void *regs, unsigned int phy_addr);

#ifdef CONFIG_CMD_NET
void atngw100_eth_initialize(bd_t *bi)
{
	macb_eth_initialize(0, (void *)MACB0_BASE, bi->bi_phy_id[0]);
	macb_eth_initialize(1, (void *)MACB1_BASE, bi->bi_phy_id[1]);
}
#endif
