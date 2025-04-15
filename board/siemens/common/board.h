/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common board functions for siemens based boards
 * (C) Copyright 2022 Siemens Schweiz AG
 */

#ifndef __COMMON_BOARD_H
#define __COMMON_BOARD_H

/*
 * Chip data
 * Offset in EEPROM: 0x120 - 0x14F
 *
 * -----------------------------------------------------------------------------------
 * | Address range |                          Content                                |
 * -----------------------------------------------------------------------------------
 * | 0x120 - 0x123 |  Magic Number - 0x43484950 (4 byte)                             |
 * -----------------------------------------------------------------------------------
 * | 0x124 - 0x133 |  Device Nomenclature (15 + 1 byte)                              |
 * -----------------------------------------------------------------------------------
 * | 0x134 - 0x13A |  HW Version of the form "v00.00" (6 + 1 byte)                 |
 * |               |   - First 2 digits: Layout revision (starting from 1)           |
 * |               |   - Last 2 digits: Assembly variant revision (starting from 1)  |
 * -----------------------------------------------------------------------------------
 * | 0x13B - 0x13F |  Flash Size in Gibit (4 + 1 byte)                               |
 * -----------------------------------------------------------------------------------
 * | 0x140 - 0x144 |  Ram Size in Gibit (4 + 1 byte)                                 |
 * -----------------------------------------------------------------------------------
 * | 0x145 - 0x14F |  Sequence number, equals DMC-code (10 + 1 byte) [OBSOLETE]      |
 * -----------------------------------------------------------------------------------
 */

#define MAGIC_CHIP		0x50494843
#define EEPROM_CHIP_OFFSET	0x120

struct chip_data {
	unsigned int magic;
	char sdevname[16];
	char shwver[7];
	char flash_size[5];
	char ram_size[5];
};

#endif /* __COMMON_BOARD_H */
