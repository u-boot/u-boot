/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 * Copyright 2016 IBM Corporation
 * (C) Copyright 2016 Google, Inc
 */

#ifndef _ASPEED_COMMON_CONFIG_H
#define _ASPEED_COMMON_CONFIG_H

#include <asm/arch/platform.h>

/* Misc CPU related */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SYS_SDRAM_BASE		ASPEED_DRAM_BASE

#ifdef CONFIG_PRE_CON_BUF_SZ
#define CONFIG_SYS_INIT_RAM_ADDR	(ASPEED_SRAM_BASE + CONFIG_PRE_CON_BUF_SZ)
#define CONFIG_SYS_INIT_RAM_SIZE	(ASPEED_SRAM_SIZE - CONFIG_PRE_CON_BUF_SZ)
#else
#define CONFIG_SYS_INIT_RAM_ADDR	(ASPEED_SRAM_BASE)
#define CONFIG_SYS_INIT_RAM_SIZE	(ASPEED_SRAM_SIZE)
#endif

#define SYS_INIT_RAM_END		(CONFIG_SYS_INIT_RAM_ADDR \
					 + CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(SYS_INIT_RAM_END \
					 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		(32 << 20)

/*
 * NS16550 Configuration
 */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Miscellaneous configurable options
 */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0"	\
	"spi_dma=yes\0" \
	""

#endif	/* __AST_COMMON_CONFIG_H */
