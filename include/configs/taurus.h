/*
 * Common board functions for Siemens TAURUS (AT91SAM9G20) based boards
 * (C) Copyright 2013 Siemens AG
 *
 * Based on:
 * U-Boot file: include/configs/at91sam9260ek.h
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

#define CONFIG_SYS_GENERIC_BOARD

#if defined(CONFIG_SPL_BUILD)
#define CONFIG_SYS_THUMB_BUILD
#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_DCACHE_OFF
#endif
/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */


#define CONFIG_SYS_TEXT_BASE		0x21000000

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* main clock xtal */

/* Misc CPU related */
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_CMD_BOOTZ
#define CONFIG_OF_LIBFDT

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200

#define CONFIG_BOOTDELAY	3

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_SOURCE

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NAND

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		(128 * 1024 * 1024)

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
# define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM1 + 0x1000 - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13
#endif

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH			1

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_AT91_WANTS_COMMON_PHY

#define CONFIG_AT91SAM9_WATCHDOG
#if !defined(CONFIG_SPL_BUILD)
/* Enable the watchdog */
#define CONFIG_HW_WATCHDOG
#endif

/* USB */
#if defined(CONFIG_BOARD_TAURUS)
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE
#endif

/* SPI EEPROM */
#define CONFIG_SPI
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_ATMEL_SPI
#define CONFIG_SPI_FLASH_STMICRO
#define TAURUS_SPI_MASK (1 << 4)
#define TAURUS_SPI_CS_PIN	AT91_PIN_PA3

#if defined(CONFIG_SPL_BUILD)
/* SPL related */
#undef CONFIG_SPL_OS_BOOT		/* Not supported by existing map */
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

#define CONFIG_SF_DEFAULT_BUS 0
#define CONFIG_SF_DEFAULT_SPEED 10000000
#define CONFIG_SF_DEFAULT_MODE SPI_MODE_0
#endif

/* load address */
#define CONFIG_SYS_LOAD_ADDR			0x22000000

/* bootstrap in spi flash , u-boot + env + linux in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x100000
#define CONFIG_ENV_OFFSET_REDUND	0x180000
#define CONFIG_ENV_SIZE		0x20000		/* 1 sector = 128 kB */
#define CONFIG_BOOTCOMMAND	"nand read 0x22000000 0x200000 0x300000; bootm"
#define CONFIG_BOOTARGS							\
	"console=ttyS0,115200 earlyprintk "				\
	"mtdparts=atmel_nand:256k(bootstrap)ro,512k(uboot)ro,"		\
	"256k(env),256k(env_redundant),256k(spare),"			\
	"512k(dtb),6M(kernel)ro,-(rootfs) "				\
	"root=/dev/mtdblock7 rw rootfstype=jffs2"

#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE \
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN \
	ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x0
#define CONFIG_SPL_MAX_SIZE		(14 * 1024)
#define CONFIG_SPL_STACK		(16 * 1024)
#define CONFIG_SYS_SPL_MALLOC_START     (CONFIG_SYS_TEXT_BASE - \
					CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_SPL_MALLOC_SIZE      CONFIG_SYS_MALLOC_LEN

#define CONFIG_SPL_BSS_START_ADDR	CONFIG_SPL_MAX_SIZE
#define CONFIG_SPL_BSS_MAX_SIZE		(3 * 1024)

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SYS_NAND_ENABLE_PIN_SPL	(2*32 + 14)
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SYS_USE_NANDFLASH	1
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_NAND_RAW_ONLY
#define CONFIG_SPL_NAND_SOFTECC
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x80000
#define	CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_5_ADDR_CYCLE

#define CONFIG_SYS_NAND_SIZE		(256*1024*1024)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }


#define CONFIG_SPL_ATMEL_SIZE
#define CONFIG_SYS_MASTER_CLOCK		132096000
#define AT91_PLL_LOCK_TIMEOUT		1000000
#define CONFIG_SYS_AT91_PLLA		0x202A3F01
#define CONFIG_SYS_MCKR			0x1300
#define CONFIG_SYS_MCKR_CSS		(0x02 | CONFIG_SYS_MCKR)
#define CONFIG_SYS_AT91_PLLB		0x10193F05
#endif
