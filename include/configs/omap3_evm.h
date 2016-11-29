/*
 * Configuration settings for the TI OMAP3 EVM board.
 *
 * Copyright (C) 2006-2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Author :
 *	Manikandan Pillai <mani.pillai@ti.com>
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * Manikandan Pillai <mani.pillai@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __OMAP3EVM_CONFIG_H
#define __OMAP3EVM_CONFIG_H

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/* ----------------------------------------------------------------------------
 * Supported U-Boot commands
 * ----------------------------------------------------------------------------
 */

#define CONFIG_CMD_JFFS2

#define CONFIG_CMD_NAND

/* ----------------------------------------------------------------------------
 * Supported U-Boot features
 * ----------------------------------------------------------------------------
 */
#define CONFIG_SYS_LONGHELP

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Add auto-completion support */
#define CONFIG_AUTO_COMPLETE

/* ----------------------------------------------------------------------------
 * Supported hardware
 * ----------------------------------------------------------------------------
 */

/* MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC

/* SPL */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"

/* Partition tables */
#define CONFIG_EFI_PARTITION
#define CONFIG_DOS_PARTITION

/* USB
 *
 * Enable CONFIG_USB_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_USB_MUSB_UDD for Device functionalities.
 */
#define CONFIG_USB_OMAP3
#define CONFIG_USB_MUSB_HCD
/* #define CONFIG_USB_MUSB_UDC */

/* NAND SPL */
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_START   CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

/*
 * High level configuration options
 */
#define CONFIG_OMAP			/* This is TI OMAP core */
#define CONFIG_OMAP_GPIO
/* Common ARM Erratas */
#define CONFIG_ARM_ERRATA_454179
#define CONFIG_ARM_ERRATA_430973
#define CONFIG_ARM_ERRATA_621766

#define CONFIG_SDRC			/* The chip has SDRC controller */

#define CONFIG_OMAP3_EVM		/* This is a OMAP3 EVM */
#define CONFIG_TWL4030_POWER		/* with TWL4030 PMIC */

/*
 * Clock related definitions
 */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/* Size of environment - 128KB */
#define CONFIG_ENV_SIZE			(128 << 10)

/* Size of malloc pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10))

/*
 * Physical Memory Map
 * Note 1: CS1 may or may not be populated
 * Note 2: SDRAM size is expected to be at least 32MB
 */
#define CONFIG_NR_DRAM_BANKS		2
#define PHYS_SDRAM_1			OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2			OMAP34XX_SDRC_CS1

/* Limits for memtest */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
						0x01F00000) /* 31MB */

/* Default load address */
#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)

/* -----------------------------------------------------------------------------
 * Hardware drivers
 * -----------------------------------------------------------------------------
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SERIAL1			1	/* UART1 on OMAP3 EVM */
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_OMAP24_I2C_SPEED	100000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_SYS_I2C_OMAP34XX

/*
 * PISMO support
 */
/* Monitor at start of flash - Reserve 2 sectors */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)

/* Start location & size of environment */
#define ONENAND_ENV_OFFSET		0x260000
#define SMNAND_ENV_OFFSET		0x260000

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */

/*
 * NAND
 */
/* Physical address to access NAND */
#define CONFIG_SYS_NAND_ADDR		NAND_BASE

/* Physical address to access NAND at CS0 */
#define CONFIG_SYS_NAND_BASE		NAND_BASE

/* Max number of NAND devices */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT
/* Timeout values (in ticks) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(100 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT	(100 * CONFIG_SYS_HZ)

/* Flash banks JFFS2 should use */
#define CONFIG_SYS_MAX_MTD_BANKS	(CONFIG_SYS_MAX_FLASH_BANKS + \
						CONFIG_SYS_MAX_NAND_DEVICE)

#define CONFIG_SYS_JFFS2_MEM_NAND
#define CONFIG_SYS_JFFS2_FIRST_BANK	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* Start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
/* Size of jffs2 partition */
#define CONFIG_JFFS2_PART_SIZE		0xf980000

/*
 * USB
 */
#ifdef CONFIG_USB_OMAP3

#ifdef CONFIG_USB_MUSB_HCD

#define CONGIG_CMD_STORAGE

#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_SYS_USB_EVENT_POLL
#define CONFIG_PREBOOT			"usb start"
#endif /* CONFIG_USB_KEYBOARD */

#endif /* CONFIG_USB_MUSB_HCD */

#ifdef CONFIG_USB_MUSB_UDC
/* USB device configuration */
#define CONFIG_USB_DEVICE
#define CONFIG_USB_TTY

/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x0451
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#define CONFIG_USBD_PRODUCT_NAME	"EVM"
#endif /* CONFIG_USB_MUSB_UDC */

#endif /* CONFIG_USB_OMAP3 */

/* ----------------------------------------------------------------------------
 * U-Boot features
 * ----------------------------------------------------------------------------
 */
#define CONFIG_SYS_MAXARGS		16	/* max args for a command */

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Size of Console IO buffer */
#define CONFIG_SYS_CBSIZE		512

/* Size of print buffer */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

/* Size of bootarg buffer */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_BOOTFILE			"uImage"

/*
 * NAND / OneNAND
 */
#if defined(CONFIG_CMD_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE

#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#elif defined(CONFIG_CMD_ONENAND)
#define CONFIG_SYS_FLASH_BASE		ONENAND_MAP
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP
#endif

#if !defined(CONFIG_ENV_IS_NOWHERE)
#if defined(CONFIG_CMD_NAND)
#define CONFIG_ENV_IS_IN_NAND
#elif defined(CONFIG_CMD_ONENAND)
#define CONFIG_ENV_IS_IN_ONENAND
#define CONFIG_ENV_OFFSET		ONENAND_ENV_OFFSET
#endif
#endif /* CONFIG_ENV_IS_NOWHERE */

#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET

#if defined(CONFIG_CMD_NET)

/* Ethernet (SMSC9115 from SMSC9118 family) */
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE		0x2C000000

/* BOOTP fields */
#define CONFIG_BOOTP_SUBNETMASK		0x00000001
#define CONFIG_BOOTP_GATEWAY		0x00000002
#define CONFIG_BOOTP_HOSTNAME		0x00000004
#define CONFIG_BOOTP_BOOTPATH		0x00000010

#endif /* CONFIG_CMD_NET */

/* Support for relocation */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/* -----------------------------------------------------------------------------
 * Board specific
 * -----------------------------------------------------------------------------
 */
#define CONFIG_SYS_NO_FLASH

/* Uncomment to define the board revision statically */
/* #define CONFIG_STATIC_BOARD_REV	OMAP3EVM_BOARD_GEN_2 */

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x40200800
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)

#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_OMAP3_ID_NAND
#define CONFIG_SPL_LDSCRIPT		"arch/arm/mach-omap2/u-boot-spl.lds"

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

/* -----------------------------------------------------------------------------
 * Default environment
 * -----------------------------------------------------------------------------
 */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"usbtty=cdc_acm\0" \
	"mmcdev=0\0" \
	"console=ttyO0,115200n8\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=/dev/mmcblk0p2 rw " \
		"rootfstype=ext3 rootwait\0" \
	"nandargs=setenv bootargs console=${console} " \
		"root=/dev/mtdblock4 rw " \
		"rootfstype=jffs2\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"onenand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run nandboot; " \
			"fi; " \
		"fi; " \
	"else run nandboot; fi"

#endif /* __OMAP3EVM_CONFIG_H */
