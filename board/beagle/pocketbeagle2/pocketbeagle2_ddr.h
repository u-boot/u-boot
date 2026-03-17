/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * DDR override logic for AM625 PocketBeagle 2
 * https://www.beagleboard.org/boards/pocketbeagle-2
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __POCKETBEAGLE2_DDR_H
#define __POCKETBEAGLE2_DDR_H

#include "../../phytec/common/k3/k3_ddrss_patch.h"

typedef enum {
	EEPROM_RAM_SIZE_512MB = 0,
	EEPROM_RAM_SIZE_1GB = 1
} eeprom_ram_size;

struct ddr_reg ddr_1gb_ctl_regs[] = {
	{ 317, 0x00000101 },
	{ 318, 0x1FFF0000 },
};

struct ddr_reg ddr_1gb_pi_regs[] = {
	{ 77, 0x04010100 },
};

struct ddrss pocketbeagle2_ddrss_data[] = {
	// default configuration
	[EEPROM_RAM_SIZE_512MB] = {
		.ctl_regs = NULL,
		.ctl_regs_num = 0,
		.pi_regs = NULL,
		.pi_regs_num = 0,
		.phy_regs = NULL,
		.phy_regs_num = 0,
	},

	// industrial configuration
	[EEPROM_RAM_SIZE_1GB] = {
		.ctl_regs = &ddr_1gb_ctl_regs[0],
		.ctl_regs_num = ARRAY_SIZE(ddr_1gb_ctl_regs),
		.pi_regs = &ddr_1gb_pi_regs[0],
		.pi_regs_num = ARRAY_SIZE(ddr_1gb_pi_regs),
		.phy_regs = NULL,
		.phy_regs_num = 0,
	},
};

#endif /* __POCKETBEAGLE2_DDR_H */
