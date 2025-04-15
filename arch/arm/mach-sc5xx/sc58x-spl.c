// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2024 - Analog Devices, Inc.
 */

#include <asm/arch-adi/sc5xx/spl.h>

// Table 53-13 in SC58x HRM
const struct adi_boot_args adi_rom_boot_args[] = {
	// JTAG/no boot
	[0] = {0, 0, 0},
	// SPI master, used for qspi as well
	[1] = {0x60020000, 0x00040000, 0x00010207},
	// SPI slave
	[2] = {0, 0, 0x00000212},
	// reserved, no boot
	[3] = {0, 0, 0},
	// reserved, no boot
	[4] = {0, 0, 0},
	// reserved, also no boot
	[5] = {0, 0, 0},
	// Linkport slave
	[6] = {0, 0, 0x00000014},
	// UART slave
	[7] = {0, 0, 0x00000013},
};
