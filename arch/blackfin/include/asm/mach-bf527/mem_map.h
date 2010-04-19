/*
 * Common Blackfin memory map
 *
 * Copyright 2004-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#ifndef __BF52X_MEM_MAP_H__
#define __BF52X_MEM_MAP_H__

#define L1_DATA_A_SRAM      (0xFF800000)
#define L1_DATA_A_SRAM_SIZE (0x4000)
#define L1_DATA_A_SRAM_END  (L1_DATA_A_SRAM + L1_DATA_A_SRAM_SIZE)
#define L1_DATA_B_SRAM      (0xFF900000)
#define L1_DATA_B_SRAM_SIZE (0x4000)
#define L1_DATA_B_SRAM_END  (L1_DATA_B_SRAM + L1_DATA_B_SRAM_SIZE)
#define L1_INST_SRAM        (0xFFA00000)
#define L1_INST_SRAM_SIZE   (0xC000)
#define L1_INST_SRAM_END    (L1_INST_SRAM + L1_INST_SRAM_SIZE)

#endif
