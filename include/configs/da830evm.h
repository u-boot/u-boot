/*
 * Copyright (C) 2008 Texas Instruments, Inc <www.ti.com>
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Board
 */
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_USE_SPIFLASH

/*
 * SoC Configuration
 */
#define CONFIG_MACH_DAVINCI_DA830_EVM
#define CONFIG_SOC_DA8XX		/* TI DA8xx SoC */
#define CONFIG_SOC_DA830		/* TI DA830 SoC */
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE		0xc1080000

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
#define PHYS_SDRAM_1			0xc0000000 /* SDRAM Start */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + 0x2000000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
						(32 << 20))
#define CONFIG_NR_DRAM_BANKS	1 /* we have 1 bank of DRAM */

/*
 * Serial Driver info
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size */
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART2_BASE /* Base address of UART2 */
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */

/*
 * I2C Configuration
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DAVINCI
#define CONFIG_SYS_DAVINCI_I2C_SPEED     25000 /* 100Kbps won't work, H/W bug */
#define CONFIG_SYS_DAVINCI_I2C_SLAVE     10 /* Bogus, master-only in U-Boot */

/*
 * I2C EEPROM definitions for catalyst 24W256 EEPROM chip
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20

/*
 * Network & Ethernet Configuration
 */
#ifdef CONFIG_DRIVER_TI_EMAC
#define CONFIG_MII
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#endif

/*
 * Flash & Environment
 */
#ifdef CONFIG_USE_NAND
#undef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_NAND_DAVINCI
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_IN_NAND		/* U-Boot env in NAND Flash  */
#define CONFIG_ENV_OFFSET		(512 << 10)
#define CONFIG_ENV_SIZE			(10 << 10) /* 10KB */
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_NAND_CS		3
#define CONFIG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE3_BASE
#define CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_NAND_MASK_CLE		0x10
#define CONFIG_SYS_NAND_MASK_ALE		0x8
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */
#endif

#ifdef CONFIG_USE_NOR
#define CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* max number of flash banks */
#define CONFIG_SYS_FLASH_SECT_SZ	(64 << 10) /* 64KB */
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_FLASH_SECT_SZ*3)
#define CONFIG_SYS_FLASH_BASE		DAVINCI_ASYNC_EMIF_DATA_CE2_BASE
#define PHYS_FLASH_SIZE			(32 << 20) /* Flash size 32MB */
#define CONFIG_SYS_MAX_FLASH_SECT (PHYS_FLASH_SIZE/CONFIG_SYS_FLASH_SECT_SZ)
#define CONFIG_ENV_SECT_SIZE		CONFIG_SYS_FLASH_SECT_SZ
#define CONFIG_SYS_FLASH_SPL_ACCESS
#endif

#ifdef CONFIG_USE_SPIFLASH
#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE			(16 << 10)
#define CONFIG_ENV_OFFSET		(256 << 10)
#define CONFIG_ENV_SECT_SIZE		4096
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_DAVINCI_SPI
#define CONFIG_SYS_SPI_BASE		DAVINCI_SPI0_BASE
#define CONFIG_SYS_SPI_CLK		clk_get(DAVINCI_SPI0_CLKID)
#define CONFIG_SF_DEFAULT_SPEED		30000000
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#endif

/*
 * USB configuration
 */
#define CONFIG_USB_DA8XX	/* Platform hookup to MUSB controller */
#define CONFIG_MUSB_HCD

/*
 * U-Boot general configuration
 */
#undef CONFIG_MISC_INIT_R
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_PROMPT	"U-Boot > " /* Command Prompt */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16 /* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(PHYS_SDRAM_1 + 0x700000)
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE	/* Won't work with hush so far, may be later */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS		"mem=32M console=ttyS2,115200n8 root=/dev/mtdblock/2 rw noinitrd ip=dhcp"
#define CONFIG_BOOTCOMMAND	""
#define CONFIG_BOOTDELAY	3

/*
 * U-Boot commands
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_MEMORY
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#ifndef CONFIG_DRIVER_TI_EMAC
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_MII
#undef CONFIG_CMD_PING
#endif

#ifdef CONFIG_USE_NAND
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#endif

#ifdef CONFIG_USE_SPIFLASH
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_FLASH
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_CMD_SAVEENV
#endif

/* SD/MMC configuration */
#ifndef CONFIG_USE_NAND
#define CONFIG_MMC
#define CONFIG_DAVINCI_MMC_SD1
#define CONFIG_GENERIC_MMC
#define CONFIG_DAVINCI_MMC
#endif

/*
 * Enable MMC commands only when
 * MMC support is present
 */
#if defined(CONFIG_MMC) || defined(CONFIG_USB_DA8XX)
#define CONFIG_DOS_PARTITION	/* include support for FAT/storage */
#define CONFIG_CMD_FAT		/* include support for FAT cmd */
#endif

#ifdef CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_CMD_EXT2
#endif

#if !defined(CONFIG_USE_NAND) && \
	!defined(CONFIG_USE_NOR) && \
	!defined(CONFIG_USE_SPIFLASH)
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_SIZE		(16 << 10)
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_ENV
#endif

#ifdef CONFIG_USB_DA8XX

#ifdef CONFIG_MUSB_HCD		/* include support for usb host */
#define CONFIG_CMD_USB		/* include support for usb cmd */

#define CONFIG_USB_STORAGE	/* MSC class support */
#define CONFIG_CMD_STORAGE	/* inclue support for usb-storage cmd */

#ifdef CONFIG_USB_KEYBOARD	/* HID class support */
#define CONFIG_SYS_USB_EVENT_POLL
#define CONFIG_PREBOOT "usb start"
#endif /* CONFIG_USB_KEYBOARD */

#endif /* CONFIG_MUSB_HCD */

#ifdef CONFIG_MUSB_UDC
/* USB device configuration */
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x0451
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#define CONFIG_USBD_PRODUCT_NAME	"DA830EVM"
#endif /* CONFIG_MUSB_UDC */

#endif /* CONFIG_USB_DA8XX */

#ifdef CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT		"nand0=davinci_nand.1"
#define PART_BOOT		"512k(bootloader)ro,"
#define PART_PARAMS		"512k(params)ro,"
#define PART_KERNEL		"4m(kernel),"
#define PART_REST		"-(filesystem)"
#define MTDPARTS_DEFAULT        \
	"mtdparts=davinci_nand.1:" PART_BOOT PART_PARAMS PART_KERNEL PART_REST
#endif

#define CONFIG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#endif /* __CONFIG_H */
