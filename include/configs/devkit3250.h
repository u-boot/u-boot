/*
 * Embest/Timll DevKit3250 board configuration file
 *
 * Copyright (C) 2011 Vladimir Zapolskiy <vz@mleia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_DEVKIT3250_H__
#define __CONFIG_DEVKIT3250_H__

/* SoC and board defines */
#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * Define DevKit3250 machine type by hand until it lands in mach-types
 */
#define MACH_TYPE_DEVKIT3250		3697
#define CONFIG_MACH_TYPE		MACH_TYPE_DEVKIT3250

#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_MALLOC_LEN		SZ_1M
#define CONFIG_SYS_SDRAM_BASE		EMC_DYCS0_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_64M
#define CONFIG_SYS_TEXT_BASE		0x83FA0000
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + SZ_32K)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_TEXT_BASE - SZ_1M)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_32K)

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_4K \
					 - GENERATED_GBL_DATA_SIZE)

/*
 * Serial Driver
 */
#define CONFIG_SYS_LPC32XX_UART		2   /* UART2 */
#define CONFIG_BAUDRATE			115200

/*
 * NOR Flash
 */
#define CONFIG_CMD_FLASH
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	71
#define CONFIG_SYS_FLASH_BASE		EMC_CS0_BASE
#define CONFIG_SYS_FLASH_SIZE		SZ_4M
#define CONFIG_SYS_FLASH_CFI

/*
 * U-Boot General Configurations
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_PBSIZE		\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_VERSION_VARIABLE
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DOS_PARTITION

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			SZ_128K

/*
 * U-Boot Commands
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_CACHE

/*
 * Boot Linux
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTDELAY		3

#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTARGS			"console=ttyS2,115200n8"
#define CONFIG_LOADADDR			0x80008000

/*
 * Include SoC specific configuration
 */
#include <asm/arch/config.h>

#endif  /* __CONFIG_DEVKIT3250_H__*/
