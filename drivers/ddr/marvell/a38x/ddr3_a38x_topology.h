/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_A38X_TOPOLOGY_H
#define _DDR3_A38X_TOPOLOGY_H

#include "ddr_topology_def.h"

/* Bus mask variants */
#define BUS_MASK_32BIT			0xf
#define BUS_MASK_32BIT_ECC		0x1f
#define BUS_MASK_16BIT			0x3
#define BUS_MASK_16BIT_ECC		0x13
#define BUS_MASK_16BIT_ECC_PUP3		0xb

#define DYNAMIC_CS_SIZE_CONFIG
#define DISABLE_L2_FILTERING_DURING_DDR_TRAINING

#endif /* _DDR3_A38X_TOPOLOGY_H */
