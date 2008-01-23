/*
 * Copyright 2006 Freescale Semiconductor
 * York Sun (yorksun@freescale.com)
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
#include <command.h>

#ifdef CFG_ID_EEPROM

extern int do_mac(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

U_BOOT_CMD(
	mac, 3, 1,  do_mac,
	"mac     - display and program the system ID and MAC addresses in EEPROM\n",
	"[read|save|id|num|errata|date|ports|0|1|2|3|4|5|6|7]\n"
	"read\n"
	"    - show content of EEPROM\n"
	"mac save\n"
	"    - save to the EEPROM\n"
	"mac id\n"
	"    - program system id\n"
	"mac num\n"
	"    - program system serial number\n"
	"mac errata\n"
	"    - program errata data\n"
	"mac date\n"
	"    - program date\n"
	"mac ports\n"
	"    - program the number of ports\n"
	"mac 0\n"
	"    - program the MAC address for port 0\n"
	"mac 1\n"
	"    - program the MAC address for port 1\n"
	"mac 2\n"
	"    - program the MAC address for port 2\n"
	"mac 3\n"
	"    - program the MAC address for port 3\n"
	"mac 4\n"
	"    - program the MAC address for port 4\n"
	"mac 5\n"
	"    - program the MAC address for port 5\n"
	"mac 6\n"
	"    - program the MAC address for port 6\n"
	"mac 7\n"
	"    - program the MAC address for port 7\n"
);
#endif /* CFG_ID_EEPROM */
