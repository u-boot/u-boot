/*
 * (C) Copyright 2014 Xilinx, Inc.
 *
 * Configuration settings for the Xilinx Zynq OCM.
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_OCM_H
#define __CONFIG_ZYNQ_OCM_H

#define CONFIG_SYS_NO_FLASH
#define _CONFIG_CMD_DEFAULT_H
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_ZYNQ_OCM
#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_QSPI
#define CONFIG_ZYNQ_EEPROM

#include <configs/zynq-common.h>

/* Undef unneeded configs */
#undef CONFIG_SYS_SDRAM_BASE
#undef CONFIG_OF_LIBFDT
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOARD_LATE_INIT
#undef CONFIG_FIT
#undef CONFIG_FIT_VERBOSE
#undef CONFIG_CMD_GO
#undef CONFIG_CMD_BOOTM
#undef CONFIG_CMD_BOOTZ
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_SYS_HUSH_PARSER
#undef CONFIG_SYS_PROMPT_HUSH_PS2
#undef CONFIG_BOOTDELAY
#undef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_ENV_SIZE
#undef CONFIG_CMDLINE_EDITING
#undef CONFIG_AUTO_COMPLETE
#undef CONFIG_ZLIB
#undef CONFIG_GZIP
#undef CONFIG_CMD_SPL
#undef CONFIG_SUPPORT_VFAT
#undef CONFIG_CMD_EXT2
#undef CONFIG_FAT_WRITE
#undef CONFIG_CMD_EXT4
#undef CONFIG_CMD_EXT4_WRITE
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SECT_SIZE

/* Define needed configs */
#define CONFIG_CMD_MEMORY
#define CONFIG_BOOTDELAY	-1 /* -1 to Disable autoboot */
#define CONFIG_SYS_MALLOC_LEN	0x4000
#define CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		0x0
#define CONFIG_ENV_SECT_SIZE		255
#define CONFIG_ENV_SIZE			1000

#define CONFIG_SYS_SDRAM_SIZE		((256 * 1024) - 32)
#define CONFIG_SYS_SDRAM_BASE		0xFFFC0000

#define FAT_BUFF_PTR_OCM		0xFFFC5000
/* Define the cluster size fat filesystem */
#define CONFIG_FS_FAT_MAX_CLUSTSIZE		32768

#endif /* __CONFIG_ZYNQ_OCM_H */
