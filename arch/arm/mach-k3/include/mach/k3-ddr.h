/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024, Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef _K3_DDR_H_
#define _K3_DDR_H_

#include <spl.h>

/* We need 3 extra entries for:
 *   SoC peripherals, flash and the sentinel value.
 */
#define K3_MEM_MAP_LEN			((CONFIG_NR_DRAM_BANKS) + 3)
#define K3_MEM_MAP_FIRST_BANK_IDX	2

int dram_init(void);
int dram_init_banksize(void);

void fixup_ddr_driver_for_ecc(struct spl_image_info *spl_image);
void fixup_memory_node(struct spl_image_info *spl_image);

#endif /* _K3_DDR_H_ */
