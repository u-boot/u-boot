/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/salvator-x.h
 *     This file is Salvator-X board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 */

#ifndef __SALVATOR_X_H
#define __SALVATOR_X_H

#include "rcar-gen3-common.h"

/* Environment in eMMC, at the end of 2nd "boot sector" */

#define CONFIG_FLASH_SHOW_PROGRESS	45
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH_BANKS_LIST	{ 0x08000000 }
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_WRITE_SWAPPED_DATA

#endif /* __SALVATOR_X_H */
