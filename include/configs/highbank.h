/*
 * Copyright 2010-2011 Calxeda, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_THUMB_BUILD

#define CONFIG_SYS_NO_FLASH

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_FIT
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SYS_BOOTMAPSZ		(16 << 20)

#define CONFIG_SYS_TIMER_RATE		(150000000/256)
#define CONFIG_SYS_TIMER_COUNTER	(0xFFF34000 + 0x4)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)

#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK		150000000
#define CONFIG_PL01x_PORTS		{ (void *)(0xFFF36000) }
#define CONFIG_CONS_INDEX		0

#define CONFIG_BAUDRATE			115200

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#define CONFIG_SYS_BOOTCOUNT_LE		/* Use little-endian accessors */
#define CONFIG_SYS_BOOTCOUNT_ADDR	0xfff3cf0c

#define CONFIG_MISC_INIT_R
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	5
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					CONFIG_SYS_SCSI_MAX_LUN)

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

#define CONFIG_CALXEDA_XGMAC

/* PXE support */
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_PXE_CLIENTARCH	0x100
#define CONFIG_BOOTP_VCI_STRING		"U-boot.armv7.highbank"

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_PXE
#define CONFIG_MENU

#define CONFIG_BOOTDELAY		2
#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_RESET_TO_RETRY
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT "Autobooting in %d seconds...\nPress <s> to stop or <d> to delay\n", bootdelay
#define CONFIG_AUTOBOOT_KEYED_CTRLC
/*
 * Miscellaneous configurable options
 */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_LONGHELP		/* undef to save memory		 */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of cmd args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PROMPT		"Highbank #"
#define CONFIG_SYS_HUSH_PARSER
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT)+16)

#define CONFIG_SYS_LOAD_ADDR		0x800000
#define CONFIG_SYS_64BIT_LBA


/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1_SIZE		(4089 << 20)
#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1_SIZE - 0x100000)

/* Environment data setup
*/
#define CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_SYS_NVRAM_BASE_ADDR	0xfff88000	/* NVRAM base address */
#define CONFIG_SYS_NVRAM_SIZE		0x8000		/* NVRAM size */
#define CONFIG_ENV_SIZE			0x2000		/* Size of Environ */
#define CONFIG_ENV_ADDR			CONFIG_SYS_NVRAM_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_TEXT_BASE		0x00008000
#define CONFIG_SYS_INIT_SP_ADDR		0x01000000
#define CONFIG_SKIP_LOWLEVEL_INIT

#endif
