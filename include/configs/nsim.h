/*
 * Copyright (C) 2013-2016 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_NSIM_H_
#define _CONFIG_NSIM_H_

#include <linux/sizes.h>

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_256M

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		SZ_2M
#define CONFIG_SYS_BOOTM_LEN		SZ_32M
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * UART configuration
 *
 */
#define CONFIG_ARC_SERIAL
#define CONFIG_ARC_UART_BASE		0xC0FC1000

/*
 * Command line configuration
 */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS		16

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			SZ_512
#define CONFIG_ENV_OFFSET		0

/*
 * Environment configuration
 */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTARGS			"console=ttyARC0,115200n8"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_CBSIZE		SZ_256
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

#endif /* _CONFIG_NSIM_H_ */
