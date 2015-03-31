/*
 * (C) Copyright 2013 Atmel Corporation.
 * Josh Wu <josh.wu@atmel.com>
 *
 * Configuation settings for the AT91SAM9N12-EK boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AT91SAM9N12_CONFIG_H_
#define __AT91SAM9N12_CONFIG_H_

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

#define CONFIG_SYS_TEXT_BASE		0x26f00000

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	16000000	/* main clock xtal */

/* Misc CPU related */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_OF_LIBFDT
#define CONFIG_SYS_GENERIC_BOARD

/* general purpose I/O */
#define CONFIG_AT91_GPIO

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200

/* LCD */
#define CONFIG_LCD
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP			24
#define CONFIG_LCD_LOGO
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_ATMEL_HLCD
#define CONFIG_ATMEL_LCD_RGB565
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_BOOTDELAY		3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_FPGA

#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NAND
#define CONFIG_CMD_SF
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB

#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x08000000

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
# define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM + 0x1000 - GENERATED_GBL_DATA_SIZE)

/* DataFlash */
#ifdef CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SF_DEFAULT_SPEED		30000000
#define CONFIG_ENV_SPI_MODE		SPI_MODE_3
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#endif

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	GPIO_PIN_PD(4)
#define CONFIG_SYS_NAND_READY_PIN	GPIO_PIN_PD(5)

/* PMECC & PMERRLOC */
#define CONFIG_ATMEL_NAND_HWECC
#define CONFIG_ATMEL_NAND_HW_PMECC
#define CONFIG_PMECC_CAP		2
#define CONFIG_PMECC_SECTOR_SIZE	512
#define CONFIG_PMECC_INDEX_TABLE_OFFSET	0x8000

#define CONFIG_CMD_NAND_TRIMFFS

#endif

#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT			"nand0=atmel_nand"
#define MTDPARTS_DEFAULT						\
	"mtdparts=atmel_nand:256k(bootstrap)ro,512k(uboot)ro,"		\
	"256k(env),256k(env_redundant),256k(spare),"			\
	"512k(dtb),6M(kernel)ro,-(rootfs)"

#define CONFIG_EXTRA_ENV_SETTINGS                                       \
	"console=console=ttyS0,115200\0"                                \
	"mtdparts="MTDPARTS_DEFAULT"\0"					\
	"bootargs_nand=rootfstype=ubifs ubi.mtd=7 root=ubi0:rootfs rw\0"\
	"bootargs_mmc=root=/dev/mmcblk0p2 rw rootfstype=ext4 rootwait\0"

/* MMC */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#endif

/* FAT */
#ifdef CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

/* Ethernet */
#define CONFIG_KS8851_MLL
#define CONFIG_KS8851_MLL_BASEADDR	0x30000000 /* use NCS2 */

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x26e00000

/* USB host */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_BASE_OHCI
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9n12"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	1
#define CONFIG_USB_STORAGE
#endif

#ifdef CONFIG_SYS_USE_SPIFLASH

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		0x5000
#define CONFIG_ENV_SIZE			0x3000
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_nand};"	\
	"sf probe 0; sf read 0x22000000 0x100000 0x300000; "		\
	"bootm 0x22000000"

#elif defined(CONFIG_SYS_USE_NANDFLASH)

/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0xc0000
#define CONFIG_ENV_OFFSET_REDUND	0x100000
#define CONFIG_ENV_SIZE			0x20000		/* 1 sector = 128 kB */
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_nand};"	\
	"nand read 0x21000000 0x180000 0x080000;"			\
	"nand read 0x22000000 0x200000 0x400000;"			\
	"bootm 0x22000000 - 0x21000000"

#else /* CONFIG_SYS_USE_MMC */

/* bootstrap + u-boot + env + linux in mmc */

#ifdef CONFIG_ENV_IS_IN_MMC
/* Use raw reserved sectors to save environment */
#define CONFIG_ENV_OFFSET		0x2000
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_SYS_MMC_ENV_DEV		0
#else
/* Use file in FAT file to save environment */
#define CONFIG_ENV_IS_IN_FAT
#define CONFIG_FAT_WRITE
#define FAT_ENV_INTERFACE		"mmc"
#define FAT_ENV_FILE			"uboot.env"
#define FAT_ENV_DEVICE_AND_PART		"0"
#define CONFIG_ENV_SIZE			0x4000
#endif

#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_mmc};"	\
	"fatload mmc 0:1 0x21000000 dtb;"				\
	"fatload mmc 0:1 0x22000000 uImage;"				\
	"bootm 0x22000000 - 0x21000000"

#endif

#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) \
					+ 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(4 * 1024 * 1024)

/* SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x300000
#define CONFIG_SPL_MAX_SIZE		0x6000
#define CONFIG_SPL_STACK		0x308000

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

#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20953f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#define ATMEL_BASE_MPDDRC		ATMEL_BASE_DDRSDRC

#ifdef CONFIG_SYS_USE_MMC
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
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x20000
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0x0
#define CONFIG_SPL_GENERATE_ATMEL_PMECC_HEADER

#elif CONFIG_SYS_USE_SPIFLASH
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8400

#endif

#endif
