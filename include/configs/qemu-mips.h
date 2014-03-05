/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This file contains the configuration parameters for qemu-mips target.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_QEMU_MIPS
#define CONFIG_MISC_INIT_R

#define CONFIG_BOOTDELAY	10	/* autoboot after 10 seconds */

#define CONFIG_BAUDRATE		115200

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */
#undef CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"panic=1\0"						\
	"bootfile=/tftpboot/vmlinux\0"				\
	"load=tftp 80500000 ${u-boot}\0"				\
	""

#define CONFIG_BOOTCOMMAND	"bootp;bootelf"

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_DHCP

#define CONFIG_DRIVER_NE2000
#define CONFIG_DRIVER_NE2000_BASE	0xb4000300

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		115200
#define CONFIG_SYS_NS16550_COM1		0xb40003f8
#define CONFIG_CONS_INDEX		1

#define CONFIG_CMD_IDE
#define CONFIG_DOS_PARTITION

#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x1f0
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x170
#define CONFIG_SYS_ATA_DATA_OFFSET	0
#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_BASE_ADDR	0xb4000000

#define CONFIG_SYS_IDE_MAXDEVICE	4

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */

/* Monitor Command Prompt */
#if defined(CONFIG_SYS_LITTLE_ENDIAN)
#define CONFIG_SYS_PROMPT		"qemu-mipsel # "
#else
#define CONFIG_SYS_PROMPT		"qemu-mips # "
#endif

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_HUSH_PARSER

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		256
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS		16

#define CONFIG_SYS_MALLOC_LEN		128*1024

#define CONFIG_SYS_BOOTPARAMS_LEN	128*1024

#define CONFIG_SYS_MHZ			132

#define CONFIG_SYS_MIPS_TIMER_FREQ	(CONFIG_SYS_MHZ * 1000000)

/* Cached addr */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

/* default load address */
#define CONFIG_SYS_LOAD_ADDR		0x81000000

#define CONFIG_SYS_MEMTEST_START	0x80100000
#define CONFIG_SYS_MEMTEST_END		0x80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/* The following #defines are needed to get flash environment right */
#define CONFIG_SYS_TEXT_BASE		0xbfc00000 /* Rom version */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 << 10)

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

/* We boot from this flash, selected with dip switch */
#define CONFIG_SYS_FLASH_BASE		0xbfc00000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	128
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)

/* Address and size of Primary Environment Sector */
#define CONFIG_ENV_SIZE		0x8000

#define CONFIG_ENV_OVERWRITE	1

#define MEM_SIZE		128

#define CONFIG_LZMA

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

#endif /* __CONFIG_H */
