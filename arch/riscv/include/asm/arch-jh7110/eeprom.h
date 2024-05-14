/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#ifndef _ASM_RISCV_EEPROM_H
#define _ASM_RISCV_EEPROM_H

#include <linux/types.h>

u8 get_pcb_revision_from_eeprom(void);
u32 get_ddr_size_from_eeprom(void);

/**
 * get_mmc_size_from_eeprom() - read eMMC size from EEPROM
 *
 * @return: size in GiB or 0 on error.
 */
u32 get_mmc_size_from_eeprom(void);

/**
 * get_product_id_from_eeprom - get product ID string
 *
 * A string like "VF7110A1-2228-D008E000-00000001" is returned.
 *
 * Return:	product ID string
 */
const char *get_product_id_from_eeprom(void);

#endif /* _ASM_RISCV_EEPROM_H */
