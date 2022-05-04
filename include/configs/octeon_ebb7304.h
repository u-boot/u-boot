/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-2020
 * Marvell <www.marvell.com>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "octeon_common.h"

/*
 * CFI flash
 */
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

#endif /* __CONFIG_H__ */
