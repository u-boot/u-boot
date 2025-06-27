/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024, Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef _K3_DDR_H_
#define _K3_DDR_H_

#include <spl.h>

int dram_init(void);
int dram_init_banksize(void);

void fixup_ddr_driver_for_ecc(struct spl_image_info *spl_image);
void fixup_memory_node(struct spl_image_info *spl_image);

#endif /* _K3_DDR_H_ */
