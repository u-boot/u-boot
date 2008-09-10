/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * Configuation settings for the SAMSUNG SMDK6400(mDirac-III) board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_S3C6400		1	/* in a SAMSUNG S3C6400 SoC     */
#define CONFIG_S3C64XX		1	/* in a SAMSUNG S3C64XX Family  */
#define CONFIG_SMDK6400		1	/* on a SAMSUNG SMDK6400 Board  */

#define CFG_SDRAM_BASE	0x50000000

/* input clock of PLL: SMDK6400 has 12MHz input clock */
#define CONFIG_SYS_CLK_FREQ	12000000

#if !defined(CONFIG_NAND_SPL) && (TEXT_BASE >= 0xc0000000)
#define CONFIG_ENABLE_MMU
#endif

#define CONFIG_MEMORY_UPPER_CODE

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG

/*
 * Architecture magic and machine type
 */
#define MACH_TYPE		1270

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#undef CONFIG_SKIP_RELOCATE_UBOOT

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CONFIG_ENV_SIZE + 1024 * 1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board	*/
#define CS8900_BASE	  	0x18800300
#define CS8900_BUS16		1 	/* follow the Linux driver	*/

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SMDK6400	*/

#define CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_CMDLINE_EDITING

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

/***********************************************************
 * Command definition
 ***********************************************************/
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_ENV
#define CONFIG_CMD_NAND
#if defined(CONFIG_BOOT_ONENAND)
#define CONFIG_CMD_ONENAND
#endif
#define CONFIG_CMD_PING
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2

#define CONFIG_BOOTDELAY	3

#define CONFIG_ZERO_BOOTDELAY_CHECK

#if (CONFIG_COMMANDS & CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1	/* which serial port to use	 */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory	      */
#define CFG_PROMPT		"SMDK6400 # "	/* Monitor Command Prompt     */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size    */
#define CFG_PBSIZE		384		/* Print Buffer Size          */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size  */

#define CFG_MEMTEST_START	CFG_SDRAM_BASE	/* memtest works on	      */
#define CFG_MEMTEST_END		(CFG_SDRAM_BASE + 0x7e00000) /* 126MB in DRAM */

#define CFG_LOAD_ADDR		CFG_SDRAM_BASE	/* default load address	*/

#define CFG_HZ			1000

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	0x40000		/* regular stack 256KB */

/**********************************
 Support Clock Settings
 **********************************
 Setting	SYNC	ASYNC
 ----------------------------------
 667_133_66	 X	  O
 533_133_66	 O	  O
 400_133_66	 X	  O
 400_100_50	 O	  O
 **********************************/

/*#define CONFIG_CLK_667_133_66*/
#define CONFIG_CLK_533_133_66
/*
#define CONFIG_CLK_400_100_50
#define CONFIG_CLK_400_133_66
#define CONFIG_SYNC_MODE
*/

/* SMDK6400 has 2 banks of DRAM, but we use only one in U-Boot */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CFG_SDRAM_BASE	/* SDRAM Bank #1	*/
#define PHYS_SDRAM_1_SIZE	0x08000000	/* 128 MB in Bank #1	*/

#define CFG_FLASH_BASE		0x10000000
#define CFG_MONITOR_BASE	0x00000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks	*/
/* AM29LV160B has 35 sectors, AM29LV800B - 19 */
#define CFG_MAX_FLASH_SECT	40

#define CONFIG_AMD_LV800
#define CFG_FLASH_CFI		1	/* Use CFI parameters (needed?) */
/* Use drivers/cfi_flash.c, even though the flash is not CFI-compliant	*/
#define CONFIG_FLASH_CFI_DRIVER	1
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_FLASH_CFI_LEGACY
#define CFG_FLASH_LEGACY_512Kx16

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5 * CFG_HZ) /* Timeout for Flash Erase	*/
#define CFG_FLASH_WRITE_TOUT	(5 * CFG_HZ) /* Timeout for Flash Write	*/

#define CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */

/*
 * SMDK6400 board specific data
 */

#define CONFIG_IDENT_STRING	" for SMDK6400"

/* base address for uboot */
#define CFG_PHY_UBOOT_BASE	(CFG_SDRAM_BASE + 0x07e00000)
/* total memory available to uboot */
#define CFG_UBOOT_SIZE		(1024 * 1024)

#ifdef CONFIG_ENABLE_MMU
#define CFG_MAPPED_RAM_BASE	0xc0000000
#define CONFIG_BOOTCOMMAND	"nand read 0xc0018000 0x60000 0x1c0000;" \
				"bootm 0xc0018000"
#else
#define CFG_MAPPED_RAM_BASE	CFG_SDRAM_BASE
#define CONFIG_BOOTCOMMAND	"nand read 0x50018000 0x60000 0x1c0000;" \
				"bootm 0x50018000"
#endif

/* NAND U-Boot load and start address */
#define CFG_UBOOT_BASE		(CFG_MAPPED_RAM_BASE + 0x07e00000)

#define CONFIG_ENV_OFFSET		0x0040000

/* NAND configuration */
#define CFG_MAX_NAND_DEVICE	1
#define CFG_NAND_BASE		0x70200010
#define NAND_MAX_CHIPS		1
#define CFG_S3C_NAND_HWECC

#define CFG_NAND_SKIP_BAD_DOT_I	1  /* ".i" read skips bad blocks	      */
#define CFG_NAND_WP		1
#define CFG_NAND_YAFFS_WRITE	1  /* support yaffs write		      */
#define CFG_NAND_BBT_2NDPAGE	1  /* bad-block markers in 1st and 2nd pages  */

#define CFG_NAND_U_BOOT_DST	CFG_PHY_UBOOT_BASE	/* NUB load-addr      */
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST	/* NUB start-addr     */

#define CFG_NAND_U_BOOT_OFFS	(4 * 1024)	/* Offset to RAM U-Boot image */
#define CFG_NAND_U_BOOT_SIZE	(252 * 1024)	/* Size of RAM U-Boot image   */

/* NAND chip page size		*/
#define CFG_NAND_PAGE_SIZE	2048
/* NAND chip block size		*/
#define CFG_NAND_BLOCK_SIZE	(128 * 1024)
/* NAND chip page per block count  */
#define CFG_NAND_PAGE_COUNT	64
/* Location of the bad-block label */
#define CFG_NAND_BAD_BLOCK_POS	0
/* Extra address cycle for > 128MiB */
#define CFG_NAND_5_ADDR_CYCLE

/* Size of the block protected by one OOB (Spare Area in Samsung terminology) */
#define CFG_NAND_ECCSIZE	CFG_NAND_PAGE_SIZE
/* Number of ECC bytes per OOB - S3C6400 calculates 4 bytes ECC in 1-bit mode */
#define CFG_NAND_ECCBYTES	4
/* Number of ECC-blocks per NAND page */
#define CFG_NAND_ECCSTEPS	(CFG_NAND_PAGE_SIZE / CFG_NAND_ECCSIZE)
/* Size of a single OOB region */
#define CFG_NAND_OOBSIZE	64
/* Number of ECC bytes per page */
#define CFG_NAND_ECCTOTAL	(CFG_NAND_ECCBYTES * CFG_NAND_ECCSTEPS)
/* ECC byte positions */
#define CFG_NAND_ECCPOS		{40, 41, 42, 43, 44, 45, 46, 47, \
				 48, 49, 50, 51, 52, 53, 54, 55, \
				 56, 57, 58, 59, 60, 61, 62, 63}

/* Boot configuration (define only one of next 3) */
#define CONFIG_BOOT_NAND
/* None of these are currently implemented. Left from the original Samsung
 * version for reference
#define CONFIG_BOOT_NOR
#define CONFIG_BOOT_MOVINAND
#define CONFIG_BOOT_ONENAND
*/

#define CONFIG_NAND
#define CONFIG_NAND_S3C64XX
/* Unimplemented or unsupported. See comment above.
#define CONFIG_ONENAND
#define CONFIG_MOVINAND
*/

/* Settings as above boot configuration */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_BOOTARGS		"console=ttySAC,115200"

#if !defined(CONFIG_ENABLE_MMU)
#define CONFIG_CMD_USB			1
#define CONFIG_USB_OHCI_NEW		1
#define CFG_USB_OHCI_REGS_BASE		0x74300000
#define CFG_USB_OHCI_SLOT_NAME		"s3c6400"
#define CFG_USB_OHCI_MAX_ROOT_PORTS	3
#define CFG_USB_OHCI_CPU_INIT		1
#define LITTLEENDIAN			1	/* used by usb_ohci.c	*/

#define CONFIG_USB_STORAGE	1
#endif
#define CONFIG_DOS_PARTITION	1

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_ENABLE_MMU)
# error "usb_ohci.c is currently broken with MMU enabled."
#endif

#endif	/* __CONFIG_H */
