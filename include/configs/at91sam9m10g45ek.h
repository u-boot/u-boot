/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9M10G45EK board(and AT91SAM9G45EKES).
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/hardware.h>

#define CONFIG_SYS_TEXT_BASE		0x73f00000

#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

#define CONFIG_AT91SAM9M10G45EK

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define	CONFIG_USART_ID			ATMEL_ID_SYS

/* LCD */
#define CONFIG_LCD
#define LCD_BPP				LCD_COLOR8
#define CONFIG_LCD_LOGO
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_ATMEL_LCD
#define CONFIG_ATMEL_LCD_RGB565
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
/* board specific(not enough SRAM) */
#define CONFIG_AT91SAM9G45_LCD_BASE		0x73E00000

/* LED */
#define CONFIG_AT91_LED
#define	CONFIG_RED_LED		AT91_PIN_PD31	/* this is the user1 led */
#define	CONFIG_GREEN_LED	AT91_PIN_PD0	/* this is the user2 led */


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

/* No NOR flash */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_CMD_NAND

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE           ATMEL_BASE_CS6
#define CONFIG_SYS_SDRAM_SIZE		0x08000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC8

#endif

/* MMC */

#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#endif

#if defined(CONFIG_CMD_USB) || defined(CONFIG_CMD_MMC)
#define CONFIG_DOS_PARTITION
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R
#define CONFIG_AT91_WANTS_COMMON_PHY

/* USB */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_ATMEL
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x23e00000

#ifdef CONFIG_SYS_USE_NANDFLASH
/* bootstrap + u-boot + env in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0xc0000
#define CONFIG_ENV_OFFSET_REDUND	0x100000
#define CONFIG_ENV_SIZE			0x20000

#define CONFIG_BOOTCOMMAND						\
	"nand read 0x70000000 0x200000 0x300000;"			\
	"bootm 0x70000000"
#define CONFIG_BOOTARGS							\
	"console=ttyS0,115200 earlyprintk "				\
	"mtdparts=atmel_nand:256k(bootstrap)ro,512k(uboot)ro,"		\
	"256k(env),256k(env_redundant),256k(spare),"			\
	"512k(dtb),6M(kernel)ro,-(rootfs) "				\
	"root=/dev/mtdblock7 rw rootfstype=jffs2"
#elif CONFIG_SYS_USE_MMC
/* bootstrap + u-boot + env + linux in mmc */
#define FAT_ENV_INTERFACE	"mmc"
/*
 * We don't specify the part number, if device 0 has partition table, it means
 * the first partition; it no partition table, then take whole device as a
 * FAT file system.
 */
#define FAT_ENV_DEVICE_AND_PART	"0"
#define FAT_ENV_FILE		"uboot.env"
#define CONFIG_ENV_IS_IN_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_ENV_SIZE		0x4000

#define CONFIG_BOOTARGS		"console=ttyS0,115200 " \
				"mtdparts=atmel_nand:" \
				"8M(bootstrap/uboot/kernel)ro,-(rootfs) " \
				"root=/dev/mmcblk0p2 rw rootwait"
#define CONFIG_BOOTCOMMAND	"fatload mmc 0:1 0x71000000 dtb; " \
				"fatload mmc 0:1 0x72000000 zImage; " \
				"bootz 0x72000000 - 0x71000000"
#endif

#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x300000
#define CONFIG_SPL_MAX_SIZE		0x010000
#define CONFIG_SPL_STACK		0x310000

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT

#define CONFIG_SYS_MONITOR_LEN		0x80000

#ifdef CONFIG_SYS_USE_MMC

#define CONFIG_SPL_BSS_START_ADDR	0x70000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00080000
#define CONFIG_SYS_SPL_MALLOC_START	0x70080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00080000

#define CONFIG_SPL_LDSCRIPT		arch/arm/mach-at91/arm926ejs/u-boot-spl.lds
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x200
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT

#elif CONFIG_SYS_USE_NANDFLASH
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_NAND_SOFTECC
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x80000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE

#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x20000
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }
#endif

#define CONFIG_SPL_ATMEL_SIZE
#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20c73f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#endif
