/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM925T	1		/* This is an arm925t CPU	*/
#define CONFIG_OMAP	1		/* in a TI OMAP core	*/
#define CONFIG_OMAP1510 1		/* which is in a 1510 (helen) */
#define CONFIG_OMAP_SX1 1		/*	a SX1 Board  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000	/* the SX1 has 12MHz input clock */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	(-4)
#define CFG_NS16550_CLK		(CONFIG_SYS_CLK_FREQ)	/* can be 12M/32Khz or 48Mhz  */
#define CFG_NS16550_COM1	0xfffb0000		/* uart1, bluetooth uart on helen */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1		1	/* we use SERIAL 1 on SX1 */

/*
 * USB device configuration
 */
#define CONFIG_USB_DEVICE	1
#define CONFIG_USB_TTY		1

#define CONFIG_USBD_VENDORID		0x1234
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Siemens"
#define CONFIG_USBD_PRODUCT_NAME	"SX1"

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C
#define CFG_I2C_SPEED		100000
#define CFG_I2C_SLAVE		1
#define CONFIG_DRIVER_OMAP1510_I2C

#define CONFIG_ENV_OVERWRITE

#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_I2C

#undef CONFIG_CMD_NET


#include <configs/omap1510.h>

#define CONFIG_BOOTARGS		"mem=16M console=ttyS0,115200n8 root=/dev/mtdblock3 rw"
#define CONFIG_PREBOOT		"setenv stdout usbtty;setenv stdin usbtty"

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory		*/
#define CFG_PROMPT		"SX1# " /* Monitor Command Prompt	*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x10000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x12000000	/* 32 MB in DRAM	*/

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0x10000000	/* default load address */

/* The 1510 has 3 timers, they can be driven by the RefClk (12Mhz) or by DPLL1.
 * This time is further subdivided by a local divisor.
 */
#define CFG_TIMERBASE	0xFFFEC500	    /* use timer 1 */
#define CFG_PVT		7		    /* 2^(pvt+1), divide by 256 */
#define CFG_HZ			((CONFIG_SYS_CLK_FREQ)/(2 << CFG_PVT))

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x10000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0x04000000 /* Flash Bank #2 */

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */
#define PHYS_FLASH_SIZE		(16 << 10) /* 16 MB */
#define PHYS_FLASH_SECT_SIZE	(128*1024) /* Size of a sector (128kB) */
#define CFG_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR	(CFG_FLASH_BASE + PHYS_FLASH_SECT_SIZE) /* addr of environment */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE	/* Monitor at beginning of flash */
#define CFG_MONITOR_LEN		PHYS_FLASH_SECT_SIZE	/* Reserve 1 sector */
#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE, CFG_FLASH_BASE + PHYS_FLASH_SIZE }

/*-----------------------------------------------------------------------
 * FLASH driver setup
 */
#define CFG_FLASH_CFI		1	/* Flash memory is CFI compliant */
#define CFG_FLASH_CFI_DRIVER	1	/* Use drivers/cfi_flash.c */
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* Use buffered writes (~10x faster) */
#define CFG_FLASH_PROTECTION	1	/* Use hardware sector protection */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(20*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(20*CFG_HZ) /* Timeout for Flash Write */

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SECT_SIZE	PHYS_FLASH_SECT_SIZE
#define CFG_ENV_SIZE		CFG_ENV_SECT_SIZE /* Total Size of Environment Sector */
#define CFG_ENV_OFFSET	( CFG_MONITOR_BASE + CFG_MONITOR_LEN ) /* Environment after Monitor */

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_SIZE_REDUND	0x20000
#define CFG_ENV_OFFSET_REDUND	0x40000

#endif	/* __CONFIG_H */
