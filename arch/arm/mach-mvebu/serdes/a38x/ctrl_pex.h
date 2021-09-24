/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _CTRL_PEX_H
#define _CTRL_PEX_H

#include <pci.h>
#include "high_speed_env_spec.h"

/* Direct access to PEX0 Root Port's PCIe Capability structure */
#define PEX0_RP_PCIE_CFG_OFFSET		(0x00080000 + 0x60)

/* SOC_CONTROL_REG1 fields */
#define PCIE0_ENABLE_OFFS		0
#define PCIE0_ENABLE_MASK		(0x1 << PCIE0_ENABLE_OFFS)
#define PCIE1_ENABLE_OFFS		1
#define PCIE1_ENABLE_MASK		(0x1 << PCIE1_ENABLE_OFFS)
#define PCIE2_ENABLE_OFFS		2
#define PCIE2_ENABLE_MASK		(0x1 << PCIE2_ENABLE_OFFS)
#define PCIE3_ENABLE_OFFS		3
#define PCIE4_ENABLE_MASK		(0x1 << PCIE3_ENABLE_OFFS)

int hws_pex_config(const struct serdes_map *serdes_map, u8 count);
void board_pex_config(void);

#endif
