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
 * Configuration settings for the PCIPPC-6 board.
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

#define	CONFIG_SYS_TEXT_BASE	0xfff00000

#define CONFIG_BOARD_EARLY_INIT_F 1
#define CONFIG_MISC_INIT_R	1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600

#define CONFIG_PREBOOT		""
#define CONFIG_BOOTDELAY	5

#ifndef __ASSEMBLY__
#include <galileo/core.h>
#endif

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
#define CONFIG_CMD_ELF
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_SNTP


#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1	/* PCI plug-and-play */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define CONFIG_SYS_HUSH_PARSER		1	/* use "hush" command parser	*/
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/

/* Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MAXARGS	64		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR	0x00100000	/* Default load address		*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE	    0x00000000
#define CONFIG_SYS_FLASH_BASE	    0xFFF00000
#define CONFIG_SYS_FLASH_MAX_SIZE  0x00100000
/* Maximum amount of RAM.
 */
#define CONFIG_SYS_MAX_RAM_SIZE    0x20000000	/* 512Mb			*/

#define CONFIG_SYS_RESET_ADDRESS   0xFFF00100

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MONITOR_LEN	    (256 << 10) /* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN	    (128 << 10) /* Reserve 128 kB for malloc()	*/

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_SDRAM_BASE && \
    CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_MAX_RAM_SIZE
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MEMTEST_START   0x00004000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END	    0x02000000	/* 0 ... 32 MB in DRAM		*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */

#define CONFIG_SYS_INIT_RAM_ADDR     0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE      0x8000
#define CONFIG_SYS_GBL_DATA_OFFSET  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET    CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_INIT_RAM_LOCK

/*
 * Temporary buffer for serial data until the real serial driver
 * is initialised (memtest will destroy this buffer)
 */
#define CONFIG_SYS_SCONSOLE_ADDR     CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SYS_SCONSOLE_SIZE     0x0002000

/* SDRAM 0 - 256MB
 */
#define CONFIG_SYS_DBAT0L	      (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT0U	      (CONFIG_SYS_SDRAM_BASE | \
			       BATU_BL_256M | BATU_VS | BATU_VP)
/* SDRAM 1 - 256MB
 */
#define CONFIG_SYS_DBAT1L	      ((CONFIG_SYS_SDRAM_BASE + 0x10000000) | \
			       BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT1U	      ((CONFIG_SYS_SDRAM_BASE + 0x10000000) | \
			       BATU_BL_256M | BATU_VS | BATU_VP)

/* Init RAM in the CPU DCache (no backing memory)
 */
#define CONFIG_SYS_DBAT2L	      (CONFIG_SYS_INIT_RAM_ADDR | \
			       BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT2U	      (CONFIG_SYS_INIT_RAM_ADDR | \
			       BATU_BL_128K | BATU_VS | BATU_VP)

/* I/O and PCI memory at 0xf0000000
 */
#define CONFIG_SYS_DBAT3L	      (0xf0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_DBAT3U	      (0xf0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT0L	      CONFIG_SYS_DBAT0L
#define CONFIG_SYS_IBAT0U	      CONFIG_SYS_DBAT0U
#define CONFIG_SYS_IBAT1L	      CONFIG_SYS_DBAT1L
#define CONFIG_SYS_IBAT1U	      CONFIG_SYS_DBAT1U
#define CONFIG_SYS_IBAT2L	      CONFIG_SYS_DBAT2L
#define CONFIG_SYS_IBAT2U	      CONFIG_SYS_DBAT2U
#define CONFIG_SYS_IBAT3L	      CONFIG_SYS_DBAT3L
#define CONFIG_SYS_IBAT3U	      CONFIG_SYS_DBAT3U

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the PCIPPC2 user's manual.
 */
#define CONFIG_SYS_HZ		      1000
#define CONFIG_SYS_BUS_CLK	      100000000 /* bus speed - 100 mhz		*/
#define CONFIG_SYS_CPU_CLK	      300000000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	      (8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* Max number of flash banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	16	/* Max number of sectors in one bank	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000	/* Timeout for Flash Write (in ms)	*/

/*
 * Note: environment is not EMBEDDED in the U-Boot code.
 * It's stored in flash separately.
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x70000)
#define CONFIG_ENV_SIZE		0x1000	/* Size of the Environment		*/
#define CONFIG_ENV_SECT_SIZE	0x10000 /* Size of the Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * L2 cache
 */
#undef CONFIG_SYS_L2
#define L2_INIT	  (L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
		   L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)
#define L2_ENABLE (L2_INIT | L2CR_L2E)

/*-----------------------------------------------------------------------
  RTC m48t59
*/
#define CONFIG_RTC_MK48T59

#define CONFIG_WATCHDOG


#define CONFIG_EEPRO100
#define CONFIG_SYS_RX_ETH_BUFFER	8               /* use 8 rx buffer on eepro100  */
#define CONFIG_TULIP


#define CONFIG_SCSI_SYM53C8XX
#define CONFIG_SCSI_DEV_ID	0x000B	/* 53c896 */
#define CONFIG_SYS_SCSI_MAX_LUN	8	/* number of supported LUNs */
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	15	/* maximum SCSI ID (0..6) */
#define CONFIG_SYS_SCSI_MAX_DEVICE	CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN /* maximum Target devices */
#define CONFIG_SYS_SCSI_SPIN_UP_TIME	2
#define CONFIG_SYS_SCSI_SCAN_BUS_REVERSE 0
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION

#endif	/* __CONFIG_H */
