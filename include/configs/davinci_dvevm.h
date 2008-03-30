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
#include <asm/sizes.h>

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
#define CFG_NAND_SMALLPAGE
#define CFG_USE_NOR
#define CFG_USE_INTEL_NOR	/* Define this when your DVEVM has Intel
				 * flash instead of AMD flash
				 */
/*===================*/
/* SoC Configuration */
/*===================*/
#define CONFIG_ARM926EJS			/* arm926ejs CPU core */
#define CONFIG_SYS_CLK_FREQ	297000000	/* Arm Clock frequency */
#define CFG_TIMERBASE		0x01c21400	/* use timer 0 */
#define CFG_HZ_CLOCK		27000000	/* Timer Input clock freq */
#define CFG_HZ			1000
#define CFG_DAVINCI_PINMUX_0	0x00000c1f
#define CFG_DAVINCI_WAITCFG	0x00000000
#define CFG_DAVINCI_ACFG2	0x3ffffffd	/* CE configs */
#define CFG_DAVINCI_ACFG3	0x3ffffffd
#define CFG_DAVINCI_ACFG4	0x3ffffffd
#define CFG_DAVINCI_ACFG5	0x3ffffffd
#undef	CFG_DAVINCI_NANDCE		/* When using NAND, define 2,3 or 4 */
#define CFG_DAVINCI_DDRCTL	0x50006405	/* DDR timing config */
#define CFG_DAVINCI_SDREF	0x000005c3
#define CFG_DAVINCI_SDCFG	0x00178632	/* 8 banks */
#define CFG_DAVINCI_SDTIM0	0x28923211
#define CFG_DAVINCI_SDTIM1	0x0016c722
#define CFG_DAVINCI_MMARG_BRF0	0x00444400
/* DM6446 = 0x15, DM6441 = 0x12, DM6441_LV = 0x0e */
#define CFG_DAVINCI_PLL1_PLLM	0x15
#define CFG_DAVINCI_PLL2_PLLM	0x17		/* 162 MHz */
#define CFG_DAVINCI_PLL2_DIV1	0x0b		/* 54 MHz */
#define CFG_DAVINCI_PLL2_DIV2	0x01
/*====================================================*/
/* EEPROM definitions for Atmel 24C256BN SEEPROM chip */
/* on Sonata/DV_EVM board. No EEPROM on schmoogie.    */
/*====================================================*/
#define CFG_I2C_EEPROM_ADDR_LEN		2
#define CFG_I2C_EEPROM_ADDR		0x50
#define CFG_EEPROM_PAGE_WRITE_BITS	6
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	20
/*=============*/
/* Memory Info */
/*=============*/
#define CFG_MALLOC_LEN		(0x10000 + 128*1024)	/* malloc() len */
#define CFG_GBL_DATA_SIZE	128		/* reserved for initial data */
#define CFG_MEMTEST_START	0x80000000	/* memtest start address */
#define CFG_MEMTEST_END		0x81000000	/* 16MB RAM test */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define CONFIG_STACKSIZE	(256*1024)	/* regular stack */
#define PHYS_SDRAM_1		0x80000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE	0x10000000	/* DDR size 256MB */
#define DDR_8BANKS				/* 8-bank DDR2 (256MB) */
/*====================*/
/* Serial Driver info */
/*====================*/
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	4		/* NS16550 register size */
#define CFG_NS16550_COM1	0x01c20000	/* Base address of UART0 */
#define CFG_NS16550_CLK		27000000	/* Input clock to NS16550 */
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
/*===================*/
/* I2C Configuration */
/*===================*/
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CFG_I2C_SPEED		80000	/* 100Kbps won't work, silicon bug */
#define CFG_I2C_SLAVE		10	/* Bogus, master-only in U-Boot */
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
/*=====================*/
/* Flash & Environment */
/*=====================*/
#ifdef CFG_USE_NAND
#undef CFG_ENV_IS_IN_FLASH
#define CFG_NO_FLASH
#define CFG_ENV_IS_IN_NAND		/* U-Boot env in NAND Flash */
#ifdef CFG_NAND_SMALLPAGE
#define CFG_ENV_SECT_SIZE	512	/* Env sector Size */
#define CFG_ENV_SIZE		SZ_16K
#else
#define CFG_ENV_SECT_SIZE	2048	/* Env sector Size */
#define CFG_ENV_SIZE		SZ_128K
#endif
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is loaded by a bootloader */
#define CONFIG_SKIP_RELOCATE_UBOOT	/* to a proper address, init done */
#define CFG_NAND_BASE		0x02000000
#define CFG_NAND_HW_ECC
#define CFG_MAX_NAND_DEVICE	1	/* Max number of NAND devices */
#define NAND_MAX_CHIPS		1
#define CFG_ENV_OFFSET		0x0	/* Block 0--not used by bootcode */
#define DEF_BOOTM		""
#elif defined(CFG_USE_NOR)
#ifdef CONFIG_NOR_UART_BOOT
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is loaded by a bootloader */
#define CONFIG_SKIP_RELOCATE_UBOOT	/* to a proper address, init done */
#else
#undef CONFIG_SKIP_LOWLEVEL_INIT
#undef CONFIG_SKIP_RELOCATE_UBOOT
#endif
#define CFG_ENV_IS_IN_FLASH
#undef	CFG_NO_FLASH
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_MAX_FLASH_BANKS	1		/* max number of flash banks */
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x40000)
#define CFG_ENV_OFFSET		(CFG_ENV_ADDR)
#define CFG_FLASH_USE_BUFFER_WRITE 1 /* use buffered writes (20x faster) */
#define PHYS_FLASH_1		0x02000000	/* CS2 Base address */
#define CFG_FLASH_BASE		PHYS_FLASH_1	/* Flash Base for U-Boot */
#define PHYS_FLASH_SIZE		0x2000000	/* Flash size 32MB */
#define CFG_MAX_FLASH_SECT	(PHYS_FLASH_SIZE/CFG_FLASH_SECT_SZ)
#define CFG_ENV_SECT_SIZE	CFG_FLASH_SECT_SZ	/* Env sector Size */
#ifdef CFG_USE_INTEL_NOR
#define CFG_FLASH_SECT_SZ	0x20000	/* 128KB sect size INTEL Flash */
#define CFG_FLASH_PROTECTION	1
#else
#define CFG_FLASH_SECT_SZ	0x10000	/* 64KB sect size AMD Flash */
#endif
#endif
/*==============================*/
/* U-Boot general configuration */
/*==============================*/
#undef	CONFIG_USE_IRQ				/* No IRQ/FIQ in U-Boot */
#define CONFIG_MISC_INIT_R
#undef	CONFIG_BOOTDELAY
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CFG_PROMPT		"U-Boot > "	/* Monitor Command Prompt */
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size  */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print buffer sz */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_LOAD_ADDR		0x80700000	/* default Linux kernel load address */
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE		/* Won't work with hush so far, may be later */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CFG_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
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
#ifdef CFG_USE_NAND
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_NAND
#elif defined(CFG_USE_NOR)
#define CONFIG_CMD_JFFS2
#else
#error "Either CFG_USE_NAND or CFG_USE_NOR _MUST_ be defined !!!"
#endif
/*=======================*/
/* KGDB support (if any) */
/*=======================*/
#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1	/* which serial port to use */
#endif
#endif /* __CONFIG_H */
