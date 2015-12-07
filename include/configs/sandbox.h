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

#define CONFIG_IO_TRACE
#define CONFIG_CMD_IOTRACE

#define CONFIG_SYS_TIMER_RATE		1000000

#define CONFIG_SYS_STDIO_DEREGISTER

/* Number of bits in a C 'long' on this architecture */
#define CONFIG_SANDBOX_BITS_PER_LONG	64

#define CONFIG_OF_LIBFDT
#define CONFIG_LMB
#define CONFIG_CMD_FDT
#define CONFIG_ANDROID_BOOT_IMAGE

#define CONFIG_CMD_PCI
#define CONFIG_PCI_PNP
#define CONFIG_CMD_IO

#define CONFIG_FS_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_FS_EXT4
#define CONFIG_EXT4_WRITE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_PART
#define CONFIG_DOS_PARTITION
#define CONFIG_HOST_MAX_DEVICES 4
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_MD5SUM

#define CONFIG_CMD_GPIO

#define CONFIG_CMD_GPT
#define CONFIG_PARTITION_UUIDS
#define CONFIG_EFI_PARTITION
#define CONFIG_DOS_PARTITION

/*
 * Size of malloc() pool, before and after relocation
 */
#define CONFIG_MALLOC_F_ADDR		0x0010000
#define CONFIG_SYS_MALLOC_LEN		(32 << 20)	/* 32MB  */

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

/* SPI - enable all SPI flash types for testing purposes */
#define CONFIG_CMD_SF
#define CONFIG_CMD_SF_TEST
#define CONFIG_CMD_SPI
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SPI_FLASH_EON
#define CONFIG_SPI_FLASH_GIGADEVICE
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_WINBOND

#define CONFIG_CMD_I2C
#define CONFIG_I2C_EDID
#define CONFIG_I2C_EEPROM

/* Memory things - we don't really want a memory test */
#define CONFIG_SYS_LOAD_ADDR		0x00000000
#define CONFIG_SYS_MEMTEST_START	0x00100000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x1000)
#define CONFIG_SYS_FDT_LOAD_ADDR	        0x100

#define CONFIG_PHYSMEM

/* Size of our emulated memory */
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		(128 << 20)
#define CONFIG_SYS_TEXT_BASE		0
#define CONFIG_SYS_MONITOR_BASE	0
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

#define CONFIG_SYS_NO_FLASH

/* include default commands */
#include <config_distro_defaults.h>

#define BOOT_TARGET_DEVICES(func) \
	func(HOST, host, 1) \
	func(HOST, host, 0)

#define CONFIG_BOOTCOMMAND ""

#include <config_distro_bootcmd.h>

#define CONFIG_KEEP_SERVERADDR
#define CONFIG_UDP_CHECKSUM
#define CONFIG_CMD_LINK_LOCAL
#define CONFIG_CMD_CDP
#define CONFIG_CMD_DNS
#define CONFIG_CMD_SNTP
#define CONFIG_TIMESTAMP
#define CONFIG_CMD_RARP
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_SERVERIP
#define CONFIG_IP_DEFRAG

/* Can't boot elf images */
#undef CONFIG_CMD_ELF

#define CONFIG_CMD_HASH
#define CONFIG_HASH_VERIFY
#define CONFIG_SHA1
#define CONFIG_SHA256

#define CONFIG_CMD_SANDBOX

#define CONFIG_CMD_ENV_FLAGS
#define CONFIG_CMD_ENV_CALLBACK
#define CONFIG_CMD_GREPENV
#define CONFIG_CMD_ASKENV

#define CONFIG_BOOTARGS ""

#define CONFIG_BOARD_LATE_INIT

#ifndef SANDBOX_NO_SDL
#define CONFIG_SANDBOX_SDL
#endif

/* LCD and keyboard require SDL support */
#ifdef CONFIG_SANDBOX_SDL
#define CONFIG_LCD
#define CONFIG_VIDEO_SANDBOX_SDL
#define CONFIG_CMD_BMP
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define LCD_BPP			LCD_COLOR16
#define CONFIG_LCD_BMP_RLE8

#define CONFIG_KEYBOARD

#define SANDBOX_SERIAL_SETTINGS		"stdin=serial,cros-ec-keyb\0" \
					"stdout=serial,lcd\0" \
					"stderr=serial,lcd\0"
#else
#define SANDBOX_SERIAL_SETTINGS		"stdin=serial\0" \
					"stdout=serial,lcd\0" \
					"stderr=serial,lcd\0"
#endif

#define SANDBOX_ETH_SETTINGS		"ethaddr=00:00:11:22:33:44\0" \
					"eth1addr=00:00:11:22:33:45\0" \
					"eth3addr=00:00:11:22:33:46\0" \
					"eth5addr=00:00:11:22:33:47\0" \
					"ipaddr=1.2.3.4\0"

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"kernel_addr_r=0x1000000\0" \
	"fdt_addr_r=0xc00000\0" \
	"ramdisk_addr_r=0x2000000\0" \
	"scriptaddr=0x1000\0" \
	"pxefile_addr_r=0x2000\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	SANDBOX_SERIAL_SETTINGS \
	SANDBOX_ETH_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS

#define CONFIG_GZIP_COMPRESSED
#define CONFIG_BZIP2
#define CONFIG_LZO
#define CONFIG_LZMA

#define CONFIG_CMD_LZMADEC
#define CONFIG_CMD_USB
#define CONFIG_CMD_DATE

#endif
