/*
 * Configuration settings for the SAMA5D4 Xplained ultra board.
 *
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/hardware.h>

#define CONFIG_SYS_TEXT_BASE		0x26f00000

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

#define CONFIG_ARCH_CPU_INIT

#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_CMD_BOOTZ
#define CONFIG_OF_LIBFDT		/* Device Tree support */

#define CONFIG_SYS_GENERIC_BOARD

/* general purpose I/O */
#define CONFIG_AT91_GPIO

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_USART3
#define CONFIG_USART_ID			ATMEL_ID_USART3

#define CONFIG_BOOTDELAY		3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* No NOR flash */
#define CONFIG_SYS_NO_FLASH

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SETEXPR

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE           ATMEL_BASE_DDRCS
#define CONFIG_SYS_SDRAM_SIZE		0x20000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x210000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

/* SerialFlash */
#define CONFIG_CMD_SF

#ifdef CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI
#define CONFIG_ATMEL_SPI0
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		30000000
#endif

/* NAND flash */
#define CONFIG_CMD_NAND

#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ONFI_DETECTION
/* PMECC & PMERRLOC */
#define CONFIG_ATMEL_NAND_HWECC
#define CONFIG_ATMEL_NAND_HW_PMECC
#endif

/* MMC */
#define CONFIG_CMD_MMC

#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#define ATMEL_BASE_MMCI			ATMEL_BASE_MCI1
#endif

/* USB */
#define CONFIG_CMD_USB

#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_ATMEL
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	3
#define CONFIG_USB_STORAGE
#endif

/* USB device */
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_ATMEL_USBA
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETH_RNDIS
#define CONFIG_USBNET_MANUFACTURER      "Atmel SAMA5D4EK"

#if defined(CONFIG_CMD_USB) || defined(CONFIG_CMD_MMC)
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

/* Ethernet Hardware */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_MACB_SEARCH_PHY

/* LCD */
/* #define CONFIG_LCD */
#ifdef CONFIG_LCD
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP                  24
#define CONFIG_LCD_LOGO
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_ATMEL_HLCD
#define CONFIG_ATMEL_LCD_RGB565
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#endif

#ifdef CONFIG_SYS_USE_SERIALFLASH
/* bootstrap + u-boot + env + linux in serial flash */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_BUS	CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS	CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_OFFSET       0x10000
#define CONFIG_ENV_SIZE         0x10000
#define CONFIG_ENV_SECT_SIZE    0x1000
#define CONFIG_BOOTCOMMAND      "sf probe 0; " \
				"sf read 0x21000000 0xa0000 0x60000; " \
				"sf read 0x22000000 0x100000 0x300000; " \
				"bootz 0x22000000 - 0x21000000"
#elif CONFIG_SYS_USE_NANDFLASH
/* bootstrap + u-boot + env in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0xc0000
#define CONFIG_ENV_OFFSET_REDUND	0x100000
#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_BOOTCOMMAND	"nand read 0x21000000 0x180000 0x80000;" \
				"nand read 0x22000000 0x200000 0x600000;" \
				"bootz 0x22000000 - 0x21000000"
#elif CONFIG_SYS_USE_MMC
/* bootstrap + u-boot + env in sd card */
#define CONFIG_ENV_IS_IN_FAT
#define CONFIG_FAT_WRITE
#define FAT_ENV_INTERFACE	"mmc"
/*
 * We don't specify the part number, if device 0 has partition table, it means
 * the first partition; it no partition table, then take whole device as a
 * FAT file system.
 */
#define FAT_ENV_DEVICE_AND_PART	"0"
#define FAT_ENV_FILE		"uboot.env"
#define CONFIG_ENV_SIZE		0x4000
#define CONFIG_BOOTCOMMAND	"fatload mmc 0:1 0x21000000 at91-sama5d4_xplained.dtb; " \
				"fatload mmc 0:1 0x22000000 zImage; " \
				"bootz 0x22000000 - 0x21000000"
#endif

#ifdef CONFIG_SYS_USE_MMC
#define CONFIG_BOOTARGS							\
	"console=ttyS0,115200 earlyprintk "				\
	"root=/dev/mmcblk0p2 rw rootwait"
#else
#define CONFIG_BOOTARGS							\
	"console=ttyS0,115200 earlyprintk "				\
	"mtdparts=atmel_nand:256k(bootstrap)ro,512k(uboot)ro,"		\
	"256K(env),256k(evn_redundent),256k(spare),"			\
	"512k(dtb),6M(kernel)ro,-(rootfs) "				\
	"rootfstype=ubifs ubi.mtd=7 root=ubi0:rootfs"
#endif

#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)


/* SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x200000
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#ifdef CONFIG_SYS_USE_MMC
#define CONFIG_SPL_LDSCRIPT		arch/arm/cpu/at91-common/u-boot-spl.lds
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
#define CONFIG_PMECC_CAP		8
#define CONFIG_PMECC_SECTOR_SIZE	512
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE	0x1000
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_OOBSIZE		224
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x40000
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0x0
#define CONFIG_SPL_GENERATE_ATMEL_PMECC_HEADER

#elif CONFIG_SYS_USE_SERIALFLASH
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

#endif
#endif
