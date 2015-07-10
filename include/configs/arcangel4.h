/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_ARCANGEL4_H_
#define _CONFIG_ARCANGEL4_H_

/*
 *  CPU configuration
 */
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_CLK_FREQ

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		0x10000000	/* 256 Mb */

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		0x200000	/* 2 MB */
#define CONFIG_SYS_BOOTM_LEN		0x2000000	/* 32 MB */
#define CONFIG_SYS_LOAD_ADDR		0x82000000

#define CONFIG_SYS_NO_FLASH

/*
 * UART configuration
 *
 */
#define CONFIG_ARC_SERIAL
#define CONFIG_ARC_UART_BASE		0xC0FC1000
#define CONFIG_BAUDRATE			115200

/*
 * Command line configuration
 */
#define CONFIG_CMD_ELF

#define CONFIG_OF_LIBFDT

#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS		16

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			0x00200		/* 512 bytes */
#define CONFIG_ENV_OFFSET		0

/*
 * Environment configuration
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTARGS			"console=ttyARC0,115200n8"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"arcangel4# "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

#endif /* _CONFIG_ARCANGEL4_H_ */
