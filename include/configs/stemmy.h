/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#ifndef __CONFIGS_STEMMY_H
#define __CONFIGS_STEMMY_H

#include <linux/sizes.h>

#define CONFIG_SKIP_LOWLEVEL_INIT	/* Loaded by another bootloader */
#define CONFIG_SYS_MALLOC_LEN		SZ_2M

/* Physical Memory Map */
#define PHYS_SDRAM_1			0x00000000	/* DDR-SDRAM Bank #1 */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE		SZ_1G
#define CONFIG_SYS_INIT_RAM_SIZE	0x00100000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_GBL_DATA_OFFSET

/* FIXME: This should be loaded from device tree... */
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE		0xa0412000

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#endif
