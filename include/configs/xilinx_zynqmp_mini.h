/*
 * Configuration for Xilinx ZynqMP Flash utility
 *
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * Siva Durga Prasad Paladugu <sivadur@xilinx.com>
 *
 * Based on Configuration for Versatile Express
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_MINI_H
#define __CONFIG_ZYNQMP_MINI_H

#define CONFIG_SYS_NO_FLASH
#define CONFIG_ZYNQ_DCC
#define _CONFIG_CMD_DEFAULT_H
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF

#include <configs/xilinx_zynqmp.h>

/* Undef unneeded configs */
#undef CONFIG_OF_LIBFDT
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOARD_LATE_INIT
#undef CONFIG_CMD_BOOTZ
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_SYS_HUSH_PARSER
#undef CONFIG_SYS_PROMPT_HUSH_PS2
#undef CONFIG_BOOTDELAY
#undef CONFIG_PREBOOT
#undef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_ENV_SIZE
#undef CONFIG_CMDLINE_EDITING
#undef CONFIG_AUTO_COMPLETE
#undef CONFIG_ZLIB
#undef CONFIG_GZIP
#undef CONFIG_CMD_SPL
#undef CONFIG_CMD_ENV
#undef CONFIG_CMD_EXT2
#undef CONFIG_CMD_EXT4
#undef CONFIG_CMD_FAT
#undef CONFIG_CMD_FS_GENERIC
#undef CONFIG_CMD_MEMORY
#undef CONFIG_DOS_PARTITION
#undef CONFIG_CMD_ELF
#undef CONFIG_MP
#undef CONFIG_SYS_MEMTEST_START
#undef CONFIG_SYS_MEMTEST_END
#undef CONFIG_SYS_CACHELINE_SIZE
#undef CONFIG_SYS_INIT_SP_ADDR

#undef CONFIG_CMD_MII

/* BOOTP options */
#undef CONFIG_BOOTP_BOOTFILESIZE
#undef CONFIG_BOOTP_BOOTPATH
#undef CONFIG_BOOTP_GATEWAY
#undef CONFIG_BOOTP_HOSTNAME
#undef CONFIG_BOOTP_MAY_FAIL
#undef CONFIG_BOOTP_SERVERIP
#undef CONFIG_CMD_BOOTI
#undef CONFIG_CMD_UNZIP

/* Define needed configs */
#define CONFIG_CMD_MEMORY
#define CONFIG_BOOTDELAY	-1 /* -1 to Disable autoboot */
#define CONFIG_SYS_MALLOC_LEN	0x2000

#define CONFIG_NR_DRAM_BANKS	1

#if defined(CONFIG_MINI_QSPI)
# define CONFIG_SYS_SDRAM_SIZE		(256 * 1024)
# define CONFIG_SYS_SDRAM_BASE		0xFFFC0000
# define CONFIG_ENV_SIZE		1400
# define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x20000)

#elif defined(CONFIG_MINI_NAND)
# define CONFIG_SYS_SDRAM_SIZE		(4 * 1024 * 1024)
# define CONFIG_SYS_SDRAM_BASE		0
# define CONFIG_ENV_SIZE		0x10000
# define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x40000)

#endif

#endif /* __CONFIG_ZYNQMP_MINI_H */
