/*
 * Copyright (C) 2008 Prodrive BV <pieter.voorthijsen@prodrive.nl>
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

/*=======*/
/* Board */
/*=======*/
#define CFG_PMDRA
#define CFG_NAND_LARGEPAGE
/*===================*/
/* SoC Configuration */
/*===================*/
#define CONFIG_ARM926EJS			/* arm926ejs CPU core */
#define CONFIG_SYS_CLK_FREQ	((CFG_HZ_CLOCK * (CFG_DAVINCI_PLL1_PLLM + 1))/2)
#define CFG_TIMERBASE		0x01c21400	/* use timer 0 */
#define CFG_HZ_CLOCK		27000000	/* Timer Input clock freq */
#define CFG_HZ			1000
#define CFG_DAVINCI_PINMUX_0	0x00000c1f
#define CFG_DAVINCI_WAITCFG	0x10000000
#define CFG_DAVINCI_ACFG2	0x00460385	/* NOR CE Config */
#define CFG_DAVINCI_ACFG3	0x0822218c	/* NAND CE Config */
#define CFG_DAVINCI_ACFG4	0x3ffffffd
#define CFG_DAVINCI_ACFG5	0x3ffffffd
#define CFG_DAVINCI_NANDCE	3		/* Use CE3 for NAND */
#define CFG_DAVINCI_DDRCTL	0x50006405	/* DDR timing config */
#define CFG_DAVINCI_SDREF	0x000005c3
#define CFG_DAVINCI_SDCFG	0x00178832	/* 8 banks , CAS = 4*/
#define CFG_DAVINCI_SDTIM0	0x28923211
#define CFG_DAVINCI_SDTIM1	0x0016c722
#define CFG_DAVINCI_MMARG_BRF0	0x00444400
/* DM6446 = 0x15, DM6441 = 0x12, DM6441_LV = 0x0e */
#define CFG_DAVINCI_PLL1_PLLM	0x12
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
#define CFG_NS16550_COM2	0x01c20800	/* Base address of UART2 */
#define CFG_NS16550_CLK		27000000	/* Input clock to NS16550 */
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
/*===================*/
/* I2C Configuration */
/*===================*/
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CFG_I2C_SPEED		50000	/* 100Kbps won't work, silicon bug */
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
#define CFG_USE_NAND
#define CFG_NAND_BASE		0x04000000
#undef CFG_NAND_HW_ECC
#define CFG_MAX_NAND_DEVICE	1		/* Max number of NAND devices */
#define NAND_MAX_CHIPS		1
#define DEF_BOOTM		""
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_MAX_FLASH_BANKS	1		/* max number of flash banks */
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x40000)
#define CFG_ENV_OFFSET		(CFG_ENV_ADDR)
#define CFG_FLASH_USE_BUFFER_WRITE 1 /* use buffered writes (20x faster)*/
#define PHYS_FLASH_1		0x02000000	/* CS2 Base address */
#define CFG_FLASH_BASE		PHYS_FLASH_1	/* Flash Base for U-Boot */
#define PHYS_FLASH_SIZE		0x2000000	/* Flash size 32MB */
#define CFG_MAX_FLASH_SECT	(PHYS_FLASH_SIZE/CFG_FLASH_SECT_SZ)
#define CFG_ENV_SECT_SIZE	CFG_FLASH_SECT_SZ	/* Env sector Size */
#define CFG_FLASH_SECT_SZ	0x20000	/* 128KB sect size INTEL Flash */
#define CFG_FLASH_PROTECTION	1
/*==============================*/
/* U-Boot general configuration */
/*==============================*/
#undef CONFIG_USE_IRQ				/* No IRQ/FIQ in U-Boot */
#define CONFIG_MISC_INIT_R
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CFG_PROMPT		"U-Boot > "	/* Monitor Command Prompt */
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size  */
#define CFG_PBSIZE	(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print buffer sz */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_LOAD_ADDR	0x80700000 /* default Linux kernel load address */
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE	/* Won't work with hush so far, may be later */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CFG_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_ENV_OVERWRITE
/*===================*/
/* Linux Information */
/*===================*/
#define LINUX_BOOT_PARAM_ADDR	0x80000100
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS	\
	"mem=120M console=ttyS0,115200n8 root=/dev/hda1 rw noinitrd ip=dhcp"
#define CONFIG_BOOTCOMMAND	"run nand"
#define CONFIG_EXTRA_ENV_SETTINGS "ethaddr=00:11:22:33:44:55\n"
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
#define CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_NAND
/*=======================*/
/* KGDB support (if any) */
/*=======================*/
#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1	/* which serial port to use */
#endif
#endif /* __CONFIG_H */
