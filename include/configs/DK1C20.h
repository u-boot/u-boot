/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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

/*------------------------------------------------------------------------
 * BOARD/CPU -- TOP-LEVEL
 *----------------------------------------------------------------------*/
#define CONFIG_NIOS		1		/* NIOS-32 core		*/
#define	CONFIG_DK1C20		1		/* Cyclone DK-1C20 board*/
#define CONFIG_SYS_CLK_FREQ	50000000	/* 50 MHz core clock	*/

/*------------------------------------------------------------------------
 * BASE ADDRESSES
 *----------------------------------------------------------------------*/
#define CFG_FLASH_BASE		0x00000000	/* Flash memory base	*/
#define CFG_SRAM_BASE		0x00800000	/* External SRAM	*/
#define CFG_SRAM_SIZE		0x00100000	/* 1 MByte 		*/
#define CFG_SDRAM_BASE		0x01000000	/* SDRAM base addr	*/
#define CFG_SDRAM_SIZE		0x01000000	/* 16 MByte		*/
#define CFG_VECT_BASE		0x008fff00	/* Vector table addr	*/

/*------------------------------------------------------------------------
 * MEMORY ORGANIZATION - For the most part, you can put things pretty
 * much anywhere. This is pretty flexible for Nios. So here we make some
 * arbitrary choices & assume that the monitor is placed at the end of
 * a memory resource (so you must make sure TEXT_BASE is chosen
 * appropriately).
 *
 * 	-The heap is placed below the monitor.
 * 	-Global data is placed below the heap.
 * 	-The stack is placed below global data (&grows down).
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256k		*/
#define CFG_ENV_SIZE		0x10000		/* 64 KByte (1 sector)	*/
#define CFG_GBL_DATA_SIZE	128		/* Global data size rsvd*/
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)
#define CFG_GBL_DATA_OFFSET	(CFG_MALLOC_BASE -CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP		CFG_GBL_DATA_OFFSET

/*------------------------------------------------------------------------
 * FLASH
 *----------------------------------------------------------------------*/
#define CFG_MAX_FLASH_SECT	128		/* Max # sects per bank */
#define CFG_MAX_FLASH_BANKS	1		/* Max # of flash banks */
#define CFG_FLASH_ERASE_TOUT	8000		/* Erase timeout (msec) */
#define CFG_FLASH_WRITE_TOUT	100		/* Write timeout (msec) */

/*------------------------------------------------------------------------
 * ENVIRONMENT
 *----------------------------------------------------------------------*/
#define	CFG_ENV_IS_IN_FLASH	1		/* Environment in flash */
#define CFG_ENV_ADDR		0x00000000	/* Mem addr of env	*/
#define CONFIG_ENV_OVERWRITE			/* Serial/eth change Ok */

/*------------------------------------------------------------------------
 * CONSOLE
 *----------------------------------------------------------------------*/
#define CFG_NIOS_CONSOLE	0x00920900	/* Cons uart base addr	*/
#define CFG_NIOS_FIXEDBAUD	1		/* Baudrate is fixed	*/
#define CFG_BAUDRATE_TABLE  { 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE		115200

/*------------------------------------------------------------------------
 * TIMER FOR TIMEBASE -- Nios doesn't have the equivalent of ppc  PIT,
 * so an avalon bus timer is required.
 *----------------------------------------------------------------------*/
#define CFG_NIOS_TMRBASE	0x009209e0
#define CFG_NIOS_TMRIRQ		50
#define CFG_NIOS_TMRMS		10

/*------------------------------------------------------------------------
 * Ethernet -- needs work!
 *----------------------------------------------------------------------*/
#define CONFIG_DRIVER_SMC91111			/* Using SMC91c111	*/
#define CONFIG_SMC91111_BASE	0x00910300	/* Base address		*/
#undef  CONFIG_SMC91111_EXT_PHY			/* Internal PHY		*/
#define CONFIG_SMC_USE_32_BIT			/* 32-bit data rd/wr	*/

#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.2.21
#define CONFIG_SERVERIP		192.168.2.16

/*------------------------------------------------------------------------
 * COMMANDS
 *----------------------------------------------------------------------*/
#define CONFIG_COMMANDS		(CFG_CMD_ALL & ~( \
				 CFG_CMD_ASKENV | \
				 CFG_CMD_BEDBUG | \
				 CFG_CMD_BMP	| \
				 CFG_CMD_BSP	| \
				 CFG_CMD_CACHE	| \
				 CFG_CMD_DATE	| \
				 CFG_CMD_DOC	| \
				 CFG_CMD_DTT	| \
				 CFG_CMD_EEPROM | \
				 CFG_CMD_ELF    | \
				 CFG_CMD_FAT	| \
				 CFG_CMD_FDC	| \
				 CFG_CMD_FDOS	| \
				 CFG_CMD_HWFLOW	| \
				 CFG_CMD_IDE	| \
				 CFG_CMD_I2C	| \
				 CFG_CMD_JFFS2	| \
				 CFG_CMD_KGDB	| \
				 CFG_CMD_NAND	| \
				 CFG_CMD_MMC	| \
				 CFG_CMD_MII	| \
				 CFG_CMD_PCI	| \
				 CFG_CMD_PCMCIA | \
				 CFG_CMD_SCSI	| \
				 CFG_CMD_SPI	| \
				 CFG_CMD_VFD	| \
				 CFG_CMD_USB	) )


#include <cmd_confdefs.h>

/*------------------------------------------------------------------------
 * KGDB
 *----------------------------------------------------------------------*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	9600
#endif

/*------------------------------------------------------------------------
 * MISC
 *----------------------------------------------------------------------*/
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"==> "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#undef  CFG_CLKS_IN_HZ
#define	CFG_HZ			1562500
#define CFG_LOAD_ADDR		0x00800000	/* Default load address */

#define CFG_MEMTEST_START	0x00000000
#define CFG_MEMTEST_END		0x00000000


#endif	/* __CONFIG_H */
