/*
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * Configuation settings for the IXDP425 board.
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
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_IXP425           1       /* This is an IXP425 CPU    */
#define CONFIG_IXDP425          1       /* on an IXDP425 Board      */

#define CONFIG_DISPLAY_CPUINFO	1	/* display cpu info (and speed)	*/
#define CONFIG_DISPLAY_BOARDINFO 1	/* display board info		*/

/***************************************************************
 * U-boot generic defines start here.
 ***************************************************************/

#undef CONFIG_USE_IRQ                   /* we don't need IRQ/FIQ stuff */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN      (CONFIG_ENV_SIZE + 128*1024)

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE         115200


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_PCI


#define CONFIG_PCI
#define CONFIG_IXP_PCI
#define CONFIG_NET_MULTI
#define CONFIG_EEPRO100

#define CONFIG_BOOTDELAY        3
/*#define CONFIG_ETHADDR          08:00:3e:26:0a:5b*/
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR           192.168.0.21
#define CONFIG_SERVERIP         192.168.0.148
#define CONFIG_BOOTCOMMAND      "bootm 50040000"
#define CONFIG_BOOTARGS         "root=/dev/mtdblock2 rootfstype=cramfs console=ttyS0,115200"
#define CONFIG_CMDLINE_TAG

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE    230400          /* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX   2               /* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP                            /* undef to save memory         */
#define CONFIG_SYS_PROMPT              "=> "   /* Monitor Command Prompt       */
#define CONFIG_SYS_CBSIZE              256             /* Console I/O Buffer Size      */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS             16              /* max number of command args   */
#define CONFIG_SYS_BARGSIZE            CONFIG_SYS_CBSIZE      /* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START       0x00400000      /* memtest works on     */
#define CONFIG_SYS_MEMTEST_END         0x00800000      /* 4 ... 8 MB in DRAM   */

#define CONFIG_SYS_LOAD_ADDR           0x00010000      /* default load address */

#define CONFIG_SYS_HZ                  3333333         /* spec says 66.666 MHz, but it appears to be 33 */
						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE        (128*1024)      /* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    (4*1024)        /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ    (4*1024)        /* FIQ stack */
#endif

/***************************************************************
 * Platform/Board specific defines start here.
 ***************************************************************/

/*
 * Hardware drivers
 */


/*
 * select serial console configuration
 */
#define CONFIG_IXP_SERIAL
#define CONFIG_SYS_IXP425_CONSOLE	IXP425_UART1   /* we use UART1 for console */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS    1          /* we have 2 banks of DRAM */
#define PHYS_SDRAM_1            0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE       0x01000000 /* 16 MB */

#define PHYS_FLASH_1            0x50000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE         0x00800000 /* 8 MB */
#define PHYS_FLASH_BANK_SIZE    0x00800000 /* 8 MB Banks */
#define PHYS_FLASH_SECT_SIZE    0x00020000 /* 128 KB sectors (x1) */

#define CONFIG_SYS_DRAM_BASE           0x00000000
#define CONFIG_SYS_DRAM_SIZE           0x01000000

#define CONFIG_SYS_FLASH_BASE          PHYS_FLASH_1
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/

/*
 * Expansion bus settings
 */
#define CONFIG_SYS_EXP_CS0				0xbcd23c42

/*
 * SDRAM settings
 */
#define CONFIG_SYS_SDR_CONFIG		0xd
#define CONFIG_SYS_SDR_MODE_CONFIG	0x1
#define CONFIG_SYS_SDRAM_REFRESH_CNT	0x81a

/*
 * GPIO settings
 */

/*
 * FLASH and environment organization
 */
/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS     1       /* max number of memory banks           */
#define CONFIG_SYS_MAX_FLASH_SECT      128	/* max number of sectors on one chip    */

#define CONFIG_SYS_FLASH_CFI				/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/
#define	CONFIG_ENV_IS_IN_FLASH	1

#define CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1 }

#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT	/* no byte writes on IXP4xx	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define CONFIG_ENV_SECT_SIZE	0x20000	/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + 0x20000)
#define	CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

#endif  /* __CONFIG_H */
