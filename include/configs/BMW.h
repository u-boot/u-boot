/*
 * (C) Copyright 2001
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
 * Configuration settings for the CU824 board.
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

#define CONFIG_MPC824X		1
#define CONFIG_MPC8245		1
#define CONFIG_BMW		1

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

#define	CONFIG_MISC_INIT_F	1	/* Use misc_init_f()			*/

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600

#define CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz	*/

#define CONFIG_BOOTCOMMAND	"bootm FF820000"	/* autoboot command	*/
#define CONFIG_BOOTDELAY	5

#define CONFIG_SYS_MAX_DOC_DEVICE      1 /* Only use Onboard TSOP-16MB device */
#define DOC_PASSIVE_PROBE       1
#define CONFIG_SYS_DOC_SUPPORT_2000    1
#define CONFIG_SYS_DOC_SUPPORT_MILLENNIUM 1
#define CONFIG_SYS_DOC_SHORT_TIMEOUT    1


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

#define CONFIG_CMD_DATE
#define CONFIG_CMD_ELF
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS


#if 0
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1	/* PCI plug-and-play */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=>"	        /* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/

/* Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MAXARGS	8		/* Max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR	0x00100000	/* Default load address		*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE	    0x00000000

#define CONFIG_SYS_FLASH_BASE0_PRELIM      0xFFF00000      /* FLASH bank on RCS#0 */
#define CONFIG_SYS_FLASH_BASE1_PRELIM      0xFF800000      /* FLASH bank on RCS#1 */
#define CONFIG_SYS_FLASH_BASE  CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_FLASH_BANKS		{ CONFIG_SYS_FLASH_BASE0_PRELIM , CONFIG_SYS_FLASH_BASE1_PRELIM }

/* even though FLASHP_BASE is FF800000, with 4MB is RCS0, the
 * reset vector is actually located at FFB00100, but the 8245
 * takes care of us.
 */
#define CONFIG_SYS_RESET_ADDRESS   0xFFF00100

#define CONFIG_SYS_EUMB_ADDR	    0xFC000000

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MONITOR_LEN	    (256 << 10) /* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN	    (2048 << 10) /* Reserve 2MB for malloc()	*/

#define CONFIG_SYS_MEMTEST_START   0x00004000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END	    0x04000000	/* 0 ... 32 MB in DRAM		*/

	/* Maximum amount of RAM.
	 */
#define CONFIG_SYS_MAX_RAM_SIZE    0x04000000	/* 0 .. 64 MB of (S)DRAM */


#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
#undef CONFIG_SYS_RAMBOOT
#else
#define CONFIG_SYS_RAMBOOT
#endif


/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_RAM_ADDR CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_MONITOR_LEN
#define CONFIG_SYS_INIT_RAM_SIZE   0x2F00  /* Size of used area in DPRAM  */
#define CONFIG_SYS_GBL_DATA_OFFSET  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET  CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 */

#define CONFIG_SYS_CLK_FREQ  33000000	/* external frequency to pll */
#define CONFIG_SYS_HZ		     1000

#define CONFIG_SYS_ETH_DEV_FN	     0x7800
#define CONFIG_SYS_ETH_IOBASE	     0x00104000

	/* Bit-field values for MCCR1.
	 */
#define CONFIG_SYS_ROMNAL	    0xf
#define CONFIG_SYS_ROMFAL	    0x1f
#define CONFIG_SYS_DBUS_SIZE       0x3

	/* Bit-field values for MCCR2.
	 */
#define CONFIG_SYS_TSWAIT	    0x5		    /* Transaction Start Wait States timer */
#define CONFIG_SYS_REFINT	    0x400	    /* Refresh interval	FIXME: was 0t430		*/

	/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.
	 */
#define CONFIG_SYS_BSTOPRE	    0		/* FIXME: was 192 */

	/* Bit-field values for MCCR3.
	 */
#define CONFIG_SYS_REFREC	    2	    /* Refresh to activate interval */

	/* Bit-field values for MCCR4.
	 */
#define CONFIG_SYS_PRETOACT	    2	    /* Precharge to activate interval FIXME: was 2	*/
#define CONFIG_SYS_ACTTOPRE	    5	    /* Activate to Precharge interval FIXME: was 5	*/
#define CONFIG_SYS_SDMODE_CAS_LAT  3	    /* SDMODE CAS latancy */
#define CONFIG_SYS_SDMODE_WRAP	    0	    /* SDMODE wrap type	*/
#define CONFIG_SYS_SDMODE_BURSTLEN 3	    /* SDMODE Burst length */
#define CONFIG_SYS_ACTORW	    0xa		/* FIXME was 2 */
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER 1

#define CONFIG_SYS_PGMAX           0x0 /* how long the 8240 reatins the currently accessed page in memory FIXME: was 0x32*/

#define CONFIG_SYS_SDRAM_DSCD	0x20	/* SDRAM data in sample clock delay - note bottom 3 bits MUST be 0 */

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
 */

#define CONFIG_SYS_BANK0_START	    0x00000000
#define CONFIG_SYS_BANK0_END	    (CONFIG_SYS_MAX_RAM_SIZE - 1)
#define CONFIG_SYS_BANK0_ENABLE    1
#define CONFIG_SYS_BANK1_START	    0x3ff00000
#define CONFIG_SYS_BANK1_END	    0x3fffffff
#define CONFIG_SYS_BANK1_ENABLE    0
#define CONFIG_SYS_BANK2_START	    0x3ff00000
#define CONFIG_SYS_BANK2_END	    0x3fffffff
#define CONFIG_SYS_BANK2_ENABLE    0
#define CONFIG_SYS_BANK3_START	    0x3ff00000
#define CONFIG_SYS_BANK3_END	    0x3fffffff
#define CONFIG_SYS_BANK3_ENABLE    0
#define CONFIG_SYS_BANK4_START	    0x3ff00000
#define CONFIG_SYS_BANK4_END	    0x3fffffff
#define CONFIG_SYS_BANK4_ENABLE    0
#define CONFIG_SYS_BANK5_START	    0x3ff00000
#define CONFIG_SYS_BANK5_END	    0x3fffffff
#define CONFIG_SYS_BANK5_ENABLE    0
#define CONFIG_SYS_BANK6_START	    0x3ff00000
#define CONFIG_SYS_BANK6_END	    0x3fffffff
#define CONFIG_SYS_BANK6_ENABLE    0
#define CONFIG_SYS_BANK7_START	    0x3ff00000
#define CONFIG_SYS_BANK7_END	    0x3fffffff
#define CONFIG_SYS_BANK7_ENABLE    0

#define CONFIG_SYS_ODCR	    0xff

#define CONFIG_PCI              1 /* Include PCI support */
#undef CONFIG_PCI_PNP

/* PCI Memory space(s) */
#define PCI_MEM_SPACE1_START	0x80000000
#define PCI_MEM_SPACE2_START	0xfd000000

/* ROM Spaces */
#include "../board/bmw/bmw.h"

/* BAT configuration */
#define CONFIG_SYS_IBAT0L  (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U  (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT1L  (0x70000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U  (0x70000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT2L  (0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U  (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT3L  (0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U  (0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT0L  CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U  CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT1L  CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U  CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT2L  CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U  CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT3L  CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U  CONFIG_SYS_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	    (8 << 20)	/* Initial Memory map for Linux */

/*
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	0	/* Max number of flash banks	    */
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* Max number of sectors per  flash */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

/*
 * Warining: environment is not EMBEDDED in the U-Boot code.
 * It's stored in flash separately.
 */
#define CONFIG_ENV_IS_IN_NVRAM      1
#define CONFIG_ENV_OVERWRITE     1
#define CONFIG_SYS_NVRAM_ACCESS_ROUTINE 1
#define CONFIG_ENV_ADDR		0x7c004000 /* right at the start of NVRAM  */
#define CONFIG_ENV_SIZE		0x1ff0	/* Size of the Environment - 8K	   */
#define CONFIG_ENV_OFFSET		0	/* starting right at the beginning */

/*
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value   */
#endif

#endif	/* __CONFIG_H */
