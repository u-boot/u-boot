/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
 * Define this to make U-Boot skip low level initialization when loaded
 * by initial bootloader. Not required by NAND U-Boot version but IS
 * required for a NOR version used to burn the real NOR U-Boot into
 * NOR Flash. NAND and NOR support for DaVinci chips is mutually exclusive
 * so it is NOT possible to build a U-Boot with both NAND and NOR routines.
 * NOR U-Boot is loaded directly from Flash so it must perform all the
 * low level initialization itself. NAND version is loaded by an initial
 * bootloader (UBL in TI-ese) that performs such an initialization so it's
 * skipped in NAND version. The third DaVinci boot mode loads a bootloader
 * via UART0 and that bootloader in turn loads and runs U-Boot (or whatever)
 * performing low level init prior to loading. All that means we can NOT use
 * NAND version to put U-Boot into NOR because it doesn't have NOR support and
 * we can NOT use NOR version because it performs low level initialization
 * effectively destroying itself in DDR memory. That's why a separate NOR
 * version with this define is needed. It is loaded via UART, then one uses
 * it to somehow download a proper NOR version built WITHOUT this define to
 * RAM (tftp?) and burn it to NOR Flash. I would be probably able to squeeze
 * NOR support into the initial bootloader so it won't be needed but DaVinci
 * static RAM might be too small for this (I have something like 2Kbytes left
 * as of now, without NOR support) so this might've not happened...
 *
#define CONFIG_NOR_UART_BOOT
 */

/*=======*/
/* Board */
/*=======*/
#define DV_EVM
#define CONFIG_SYS_NAND_SMALLPAGE
#define CONFIG_SYS_USE_NOR
#define CONFIG_DISPLAY_CPUINFO
/*===================*/
/* SoC Configuration */
/*===================*/
#define CONFIG_ARM926EJS			/* arm926ejs CPU core */
#define CONFIG_SYS_TIMERBASE		0x01c21400	/* use timer 0 */
#define CONFIG_SYS_HZ_CLOCK		27000000	/* Timer Input clock freq */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SOC_DM644X
/*====================================================*/
/* EEPROM definitions for Atmel 24C256BN SEEPROM chip */
/* on Sonata/DV_EVM board. No EEPROM on schmoogie.    */
/*====================================================*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20
/*=============*/
/* Memory Info */
/*=============*/
#define CONFIG_SYS_MALLOC_LEN		(0x10000 + 128*1024)	/* malloc() len */
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* reserved for initial data */
#define CONFIG_SYS_MEMTEST_START	0x80000000	/* memtest start address */
#define CONFIG_SYS_MEMTEST_END		0x81000000	/* 16MB RAM test */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define CONFIG_STACKSIZE	(256*1024)	/* regular stack */
#define PHYS_SDRAM_1		0x80000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE	0x10000000	/* DDR size 256MB */
#define DDR_8BANKS				/* 8-bank DDR2 (256MB) */
/*====================*/
/* Serial Driver info */
/*====================*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size, byteorder */
#define CONFIG_SYS_NS16550_COM1	0x01c20000	/* Base address of UART0 */
#define CONFIG_SYS_NS16550_CLK	CONFIG_SYS_HZ_CLOCK	/* Input clock to NS16550 */
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
/*===================*/
/* I2C Configuration */
/*===================*/
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CONFIG_SYS_I2C_SPEED		80000	/* 100Kbps won't work, silicon bug */
#define CONFIG_SYS_I2C_SLAVE		10	/* Bogus, master-only in U-Boot */
/*==================================*/
/* Network & Ethernet Configuration */
/*==================================*/
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#define CONFIG_NET_MULTI
/*=====================*/
/* Flash & Environment */
/*=====================*/
#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_NAND_DAVINCI
#define CONFIG_SYS_NAND_CS		2
#undef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_IN_NAND		/* U-Boot env in NAND Flash  */
#ifdef CONFIG_SYS_NAND_SMALLPAGE
#define CONFIG_ENV_SECT_SIZE	512	/* Env sector Size */
#define CONFIG_ENV_SIZE		(16 << 10)	/* 16 KiB */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		\
	"nand0=davinci_nand.0"
#define MTDPARTS_DEFAULT	\
	"mtdparts=davinci_nand.0:384k(bootloader)ro,4m(kernel),-(filesystem)"
#else
#define CONFIG_ENV_SECT_SIZE	2048	/* Env sector Size */
#define CONFIG_ENV_SIZE		(128 << 10)	/* 128 KiB */
#endif
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is loaded by a bootloader */
#define CONFIG_SKIP_RELOCATE_UBOOT	/* to a proper address, init done */
#define CONFIG_SYS_NAND_BASE		0x02000000
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1	/* Max number of NAND devices */
#define CONFIG_ENV_OFFSET		0x0	/* Block 0--not used by bootcode */
#define DEF_BOOTM		""
#elif defined(CONFIG_SYS_USE_NOR)
#ifdef CONFIG_NOR_UART_BOOT
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is loaded by a bootloader */
#define CONFIG_SKIP_RELOCATE_UBOOT	/* to a proper address, init done */
#else
#undef CONFIG_SKIP_LOWLEVEL_INIT
#undef CONFIG_SKIP_RELOCATE_UBOOT
#endif
#define CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_SYS_NO_FLASH
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of flash banks */
#define CONFIG_SYS_FLASH_SECT_SZ	0x10000		/* 64KB sect size AMD Flash */
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_FLASH_SECT_SZ*3)
#define PHYS_FLASH_1		0x02000000	/* CS2 Base address	 */
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1	/* Flash Base for U-Boot */
#define PHYS_FLASH_SIZE		0x2000000	/* Flash size 32MB	 */
#define CONFIG_SYS_MAX_FLASH_SECT	(PHYS_FLASH_SIZE/CONFIG_SYS_FLASH_SECT_SZ)
#define CONFIG_ENV_SECT_SIZE	CONFIG_SYS_FLASH_SECT_SZ	/* Env sector Size */
#endif
/*==============================*/
/* U-Boot general configuration */
/*==============================*/
#undef	CONFIG_USE_IRQ			/* No IRQ/FIQ in U-Boot */
#define CONFIG_MISC_INIT_R
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CONFIG_SYS_PROMPT		"U-Boot > "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print buffer sz */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_LOAD_ADDR		0x80700000	/* default Linux kernel load address */
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE		/* Won't work with hush so far, may be later */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_MUSB_HCD
#define CONFIG_USB_DAVINCI
/*===================*/
/* Linux Information */
/*===================*/
#define LINUX_BOOT_PARAM_ADDR	0x80000100
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS		"mem=120M console=ttyS0,115200n8 root=/dev/hda1 rw noinitrd ip=dhcp"
#define CONFIG_BOOTCOMMAND	"setenv setboot setenv bootargs \\$(bootargs) video=dm64xxfb:output=\\$(videostd);run setboot; bootm 0x2050000"
/*=================*/
/* U-Boot commands */
/*=================*/
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_EEPROM
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR
#ifdef CONFIG_SYS_USE_NAND
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_NAND
#elif defined(CONFIG_SYS_USE_NOR)
#define CONFIG_CMD_JFFS2
#else
#error "Either CONFIG_SYS_USE_NAND or CONFIG_SYS_USE_NOR _MUST_ be defined !!!"
#endif
/*==========================*/
/* USB MSC support (if any) */
/*==========================*/
#ifdef CONFIG_USB_DAVINCI
#define CONFIG_CMD_USB
#ifdef CONFIG_MUSB_HCD
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_STORAGE
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif
#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_SYS_USB_EVENT_POLL
#define CONFIG_PREBOOT "usb start"
#endif
#endif
/*=======================*/
/* KGDB support (if any) */
/*=======================*/
#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1	/* which serial port to use */
#endif
#endif /* __CONFIG_H */
