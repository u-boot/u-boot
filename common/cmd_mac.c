/*
 * Copyright 2006 Freescale Semiconductor
 * York Sun (yorksun@freescale.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

extern int do_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

U_BOOT_CMD(
	mac, 3, 1,  do_mac,
	"display and program the system ID and MAC addresses in EEPROM",
	"[read|save|id|num|errata|date|ports|0|1|2|3|4|5|6|7]\n"
	"mac read\n"
	"    - read EEPROM content into memory\n"
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
	"mac X\n"
	"    - program the MAC address for port X [X=0...7]"
);
