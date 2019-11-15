/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: Architecture common definitions
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <asm/armv7_mpu.h>

#define AM654	2
#define J721E	4

#define REV_PG1_0	0
#define REV_PG2_0	1

void setup_k3_mpu_regions(void);
int early_console_init(void);
