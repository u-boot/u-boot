/*
 * (C) Copyright 2002-2005
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

/*
 *
 * Configuration settings for the PCIPPC-2 board.
 *
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_PCIPPC2		1	/* this is a PCIPPC2 board	*/

#define CONFIG_BOARD_EARLY_INIT_F 1
#define CONFIG_MISC_INIT_R	1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_PREBOOT		""
#define CONFIG_BOOTDELAY	5

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DOC
#define CONFIG_CMD_ELF
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SNTP

#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1	/* PCI plug-and-play */

#define CFG_NAND_LEGACY

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define	CFG_HUSH_PARSER		1	/* use "hush" command parser	*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/

/* Print Buffer Size
 */
#define CFG_PBSIZE	(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)

#define	CFG_MAXARGS	64		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_LOAD_ADDR	0x00100000	/* Default load address		*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE	    0x00000000
#define CFG_FLASH_BASE	    0xFFF00000
#define CFG_FLASH_MAX_SIZE  0x00100000
/* Maximum amount of RAM.
 */
#define CFG_MAX_RAM_SIZE    0x20000000  /* 512Mb			*/

#define CFG_RESET_ADDRESS   0xFFF00100

#define CFG_MONITOR_BASE    TEXT_BASE

#define CFG_MONITOR_LEN	    (256 << 10) /* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN	    (128 << 10) /* Reserve 128 kB for malloc()	*/

#if CFG_MONITOR_BASE >= CFG_SDRAM_BASE && \
    CFG_MONITOR_BASE < CFG_SDRAM_BASE + CFG_MAX_RAM_SIZE
#define CFG_RAMBOOT
#else
#undef CFG_RAMBOOT
#endif

#define CFG_MEMTEST_START   0x00004000	/* memtest works on		*/
#define CFG_MEMTEST_END	    0x02000000	/* 0 ... 32 MB in DRAM		*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */

/* Size in bytes reserved for initial data
 */
#define CFG_GBL_DATA_SIZE    128

#define CFG_INIT_RAM_ADDR     0x40000000
#define CFG_INIT_RAM_END      0x8000
#define CFG_GBL_DATA_OFFSET  (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET    CFG_GBL_DATA_OFFSET

#define CFG_INIT_RAM_LOCK

/*
 * Temporary buffer for serial data until the real serial driver
 * is initialised (memtest will destroy this buffer)
 */
#define CFG_SCONSOLE_ADDR     CFG_INIT_RAM_ADDR
#define CFG_SCONSOLE_SIZE     0x0002000

/* SDRAM 0 - 256MB
 */
#define CFG_DBAT0L	      (CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_DBAT0U	      (CFG_SDRAM_BASE | \
			       BATU_BL_256M | BATU_VS | BATU_VP)
/* SDRAM 1 - 256MB
 */
#define CFG_DBAT1L	      ((CFG_SDRAM_BASE + 0x10000000) | \
			       BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_DBAT1U	      ((CFG_SDRAM_BASE + 0x10000000) | \
			       BATU_BL_256M | BATU_VS | BATU_VP)

/* Init RAM in the CPU DCache (no backing memory)
 */
#define CFG_DBAT2L	      (CFG_INIT_RAM_ADDR | \
			       BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_DBAT2U	      (CFG_INIT_RAM_ADDR | \
			       BATU_BL_128K | BATU_VS | BATU_VP)

/* I/O and PCI memory at 0xf0000000
 */
#define CFG_DBAT3L	      (0xf0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_DBAT3U	      (0xf0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT0L	      CFG_DBAT0L
#define CFG_IBAT0U	      CFG_DBAT0U
#define CFG_IBAT1L	      CFG_DBAT1L
#define CFG_IBAT1U	      CFG_DBAT1U
#define CFG_IBAT2L	      CFG_DBAT2L
#define CFG_IBAT2U	      CFG_DBAT2U
#define CFG_IBAT3L	      CFG_DBAT3L
#define CFG_IBAT3U	      CFG_DBAT3U

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the PCIPPC2 user's manual.
 */
#define CFG_HZ		      1000
#define CFG_BUS_HZ            100000000 /* bus speed - 100 mhz          */
#define CFG_CPU_CLK	      300000000
#define CFG_BUS_CLK	      100000000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	      (8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* Max number of flash banks		*/
#define CFG_MAX_FLASH_SECT	16	/* Max number of sectors in one bank	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	1000	/* Timeout for Flash Write (in ms)	*/

/*
 * Note: environment is not EMBEDDED in the U-Boot code.
 * It's stored in flash separately.
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x70000)
#define CFG_ENV_SIZE		0x1000	/* Size of the Environment		*/
#define CFG_ENV_SECT_SIZE	0x10000 /* Size of the Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * L2 cache
 */
#undef CFG_L2
#define L2_INIT   (L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
		   L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)
#define L2_ENABLE (L2_INIT | L2CR_L2E)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

/*-----------------------------------------------------------------------
 * Disk-On-Chip configuration
 */

#define CFG_MAX_DOC_DEVICE	1	/* Max number of DOC devices		*/

#define CFG_DOC_SUPPORT_2000
#undef CFG_DOC_SUPPORT_MILLENNIUM

/*-----------------------------------------------------------------------
  RTC m48t59
*/
#define CONFIG_RTC_MK48T59

#define CONFIG_WATCHDOG

#define CONFIG_NET_MULTI			/* Multi ethernet cards support */

#define CONFIG_EEPRO100
#define CFG_RX_ETH_BUFFER	8               /* use 8 rx buffer on eepro100  */
#define CONFIG_TULIP

#endif	/* __CONFIG_H */
