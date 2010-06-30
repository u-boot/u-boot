/*
 * Copyright (C) 2008 Texas Instruments, Inc <www.ti.com>
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
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
 * Board
 */
#define CONFIG_DRIVER_TI_EMAC

/*
 * SoC Configuration
 */
#define CONFIG_MACH_DAVINCI_DA830_EVM
#define CONFIG_ARM926EJS		/* arm926ejs CPU core */
#define CONFIG_SOC_DA8XX		/* TI DA8xx SoC */
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)
#define CONFIG_SYS_HZ			1000
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT	/* to a proper address, init done */

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
#define CONFIG_SYS_GBL_DATA_SIZE	128 /* reserved for initial data */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(64 << 20) /* SDRAM size 64MB */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1 /* memtest start addr */
#define CONFIG_SYS_MEMTEST_END 	(PHYS_SDRAM_1 + 16*1024*1024) /* 16MB test */
#define CONFIG_NR_DRAM_BANKS	1 /* we have 1 bank of DRAM */
#define CONFIG_STACKSIZE	(256*1024) /* regular stack */

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
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * I2C Configuration
 */
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CONFIG_SYS_I2C_SPEED		25000 /* 100Kbps won't work, H/W bug */
#define CONFIG_SYS_I2C_SLAVE		10 /* Bogus, master-only in U-Boot */

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
#define CONFIG_EMAC_MDIO_PHY_NUM	1
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#define CONFIG_NET_MULTI
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
#define CONFIG_ENV_SIZE			(512 << 10)
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define CONFIG_SYS_NAND_CS		3
#define CONFIG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE3_BASE
#define CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_64BIT_VSPRINTF	/* needed for nand_util.c */
#define CONFIG_SYS_CLE_MASK		0x10
#define CONFIG_SYS_ALE_MASK		0x8
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */
#define NAND_MAX_CHIPS			1
#define DEF_BOOTM			""
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
#define CONFIG_SF_DEFAULT_SPEED		50000000
#define CONFIG_SYS_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED
#endif

/*
 * USB configuration
 */
#define CONFIG_USB_DA8XX	/* Platform hookup to MUSB controller */
#define CONFIG_MUSB_HCD

/*
 * U-Boot general configuration
 */
#undef CONFIG_USE_IRQ			/* No IRQ/FIQ in U-Boot */
#undef CONFIG_MISC_INIT_R
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_PROMPT	"DA830-evm > " /* Command Prompt */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16 /* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_MEMTEST_START + 0x700000)
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE	/* Won't work with hush so far, may be later */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(CONFIG_SYS_MEMTEST_START + 0x100)
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
#define CONFIG_CMD_EEPROM

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
#define CONFIG_CMD_SAVEENV
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
#define CONFIG_CMD_FAT		/* inclue support for FAT/storage */
#define CONFIG_DOS_PARTITION	/* inclue support for FAT/storage */

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

#endif /* __CONFIG_H */
