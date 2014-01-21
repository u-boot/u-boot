/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef FTRACE
#define CONFIG_TRACE
#define CONFIG_CMD_TRACE
#define CONFIG_TRACE_BUFFER_SIZE	(16 << 20)
#define CONFIG_TRACE_EARLY_SIZE		(8 << 20)
#define CONFIG_TRACE_EARLY
#define CONFIG_TRACE_EARLY_ADDR		0x00100000

#endif

#define CONFIG_SYS_TIMER_RATE		1000000

#define CONFIG_BOOTSTAGE
#define CONFIG_BOOTSTAGE_REPORT

/* Number of bits in a C 'long' on this architecture */
#define CONFIG_SANDBOX_BITS_PER_LONG	64

#define CONFIG_OF_CONTROL
#define CONFIG_OF_HOSTFILE
#define CONFIG_OF_LIBFDT
#define CONFIG_LMB
#define CONFIG_FIT
#define CONFIG_FIT_SIGNATURE
#define CONFIG_RSA
#define CONFIG_CMD_FDT

#define CONFIG_FS_FAT
#define CONFIG_FS_EXT4
#define CONFIG_EXT4_WRITE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_PART
#define CONFIG_DOS_PARTITION
#define CONFIG_HOST_MAX_DEVICES 4

#define CONFIG_SYS_VSNPRINTF

#define CONFIG_CMD_GPIO
#define CONFIG_SANDBOX_GPIO
#define CONFIG_SANDBOX_GPIO_COUNT	20

#define CONFIG_CMD_GPT
#define CONFIG_PARTITION_UUIDS
#define CONFIG_EFI_PARTITION

/*
 * Size of malloc() pool, although we don't actually use this yet.
 */
#define CONFIG_SYS_MALLOC_LEN		(4 << 20)	/* 4MB  */

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_LONGHELP			/* #undef to save memory */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16

/* turn on command-line edit/c/auto */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_COMMAND_HISTORY
#define CONFIG_AUTO_COMPLETE

#define CONFIG_ENV_SIZE		8192
#define CONFIG_ENV_IS_NOWHERE

/* SPI */
#define CONFIG_SANDBOX_SPI
#define CONFIG_CMD_SF
#define CONFIG_CMD_SF_TEST
#define CONFIG_CMD_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SANDBOX
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_WINBOND

/* Memory things - we don't really want a memory test */
#define CONFIG_SYS_LOAD_ADDR		0x00000000
#define CONFIG_SYS_MEMTEST_START	0x00100000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x1000)
#define CONFIG_SYS_FDT_LOAD_ADDR	0x1000000

/* Size of our emulated memory */
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		(128 << 20)
#define CONFIG_SYS_TEXT_BASE		0
#define CONFIG_SYS_MONITOR_BASE	0
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_SANDBOX_SERIAL

#define CONFIG_SYS_NO_FLASH

/* include default commands */
#include <config_cmd_default.h>

/* We don't have networking support yet */
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#define CONFIG_CMD_HASH
#define CONFIG_HASH_VERIFY
#define CONFIG_SHA1
#define CONFIG_SHA256

#define CONFIG_CMD_SANDBOX

#define CONFIG_BOOTARGS ""

#define CONFIG_EXTRA_ENV_SETTINGS	"stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#define CONFIG_GZIP_COMPRESSED
#define CONFIG_BZIP2
#define CONFIG_LZO
#define CONFIG_LZMA

#define CONFIG_TPM_TIS_SANDBOX

#endif
