/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <asm/arch/AT91RM9200.h>
#include <at91rm9200_net.h>
#include <dm9161.h>
#include <net.h>

int board_late_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* Fix Ethernet Initialization Bug when starting Linux from U-Boot */
	eth_init(gd->bd);
	return 0;
}


/* checks if addr is in RAM */
int addr2ram(ulong addr)
{
	int result = 0;

	if((addr >= PHYS_SDRAM) && (addr < (PHYS_SDRAM + PHYS_SDRAM_SIZE)))
		result = 1;

	return result;
}
