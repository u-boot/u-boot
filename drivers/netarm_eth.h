/*
 * Copyright (C) 2003 IMMS gGmbH <www.imms.de>
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
 *
 * author(s): Thomas Elste, <info@elste.org>
 */

#include <asm/types.h>
#include <config.h>

#ifdef CONFIG_DRIVER_NETARMETH

#define SET_EADDR(ad,val) *(volatile unsigned int*)(ad + NETARM_ETH_MODULE_BASE) = val
#define GET_EADDR(ad) (*(volatile unsigned int*)(ad + NETARM_ETH_MODULE_BASE))

#define NA_MII_POLL_BUSY_DELAY 900

/* MII negotiation timeout value
   500 jiffies = 5 seconds */
#define NA_MII_NEGOTIATE_DELAY 30

/* Registers in the physical layer chip */
#define MII_PHY_CONTROL		0
#define MII_PHY_STATUS		1
#define MII_PHY_ID              2
#define MII_PHY_AUTONEGADV	4

#endif /* CONFIG_DRIVER_NETARMETH */
