/*
 * (C) Copyright 2003-2004
 * MPC Data Limited (http://www.mpc-data.co.uk)
 * Dave Peverley <dpeverley at mpc-data.co.uk>
 *
 * Configuation settings for the TI OMAP Perseus 2 board.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.		See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_ARM926EJS	   1	     /* This is an arm926ejs CPU core  */
#define CONFIG_OMAP		   1	     /* in a TI OMAP core    */
#define CONFIG_OMAP730		   1	     /* which is in a 730  */
#define CONFIG_P2_OMAP730	   1	     /*	 a Perseus 2 Board  */

/*
 * Input clock of PLL
 * The OMAP730 Perseus 2 has 13MHz input clock
 */

#define CONFIG_SYS_CLK_FREQ	   13000000

#undef CONFIG_USE_IRQ			     /* we don't need IRQ/FIQ stuff */

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG	   1	     /* enable passing of ATAGs	 */
#define CONFIG_SETUP_MEMORY_TAGS   1

/*
 * Size of malloc() pool
 */

#define CFG_MALLOC_LEN		   (CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	   128	     /* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

#define CONFIG_DRIVER_LAN91C96
#define CONFIG_LAN91C96_BASE	   0x04000300
#define CONFIG_LAN91C96_EXT_PHY

/*
 * NS16550 Configuration
 */

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	   (1)
#define CFG_NS16550_CLK		   (48000000)	  /* can be 12M/32Khz or 48Mhz */
#define CFG_NS16550_COM1	   0xfffb0000	  /* uart1, bluetooth uart
						   * on perseus */

/*
 * select serial console configuration
 */

#define CONFIG_SERIAL1		   1	     /* we use SERIAL 1 on OMAP730 Perseus 2 */

#define CONFIG_CONS_INDEX	   1
#define CONFIG_BAUDRATE		   115200
#define CFG_BAUDRATE_TABLE	   { 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_COMMANDS		   (CONFIG_CMD_DFL | CFG_CMD_DHCP)
#define CONFIG_BOOTP_MASK	   CONFIG_BOOTP_DEFAULT

/*
 * This must be included AFTER the definition of CONFIG_COMMANDS (if any)
 */

#include <cmd_confdefs.h>
#include <configs/omap730.h>
#include <configs/h2_p2_dbg_board.h>

#define CONFIG_BOOTDELAY	   3
#define CONFIG_BOOTARGS		   "mem=32M console=ttyS0,115200n8 noinitrd root=/dev/nfs rw ip=bootp"

#define CONFIG_LOADADDR		   0x10000000

#define CONFIG_ETHADDR
#define CONFIG_NETMASK		   255.255.255.0
#define CONFIG_IPADDR		   192.168.0.23
#define CONFIG_SERVERIP		   192.150.0.100
#define CONFIG_BOOTFILE		   "uImage"  /* File to load */

#if defined (CONFIG_COMMANDS) && defined (CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	   115200    /* Speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	   1	     /* Which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */

#define CFG_LONGHELP				       /* undef to save memory	   */
#define CFG_PROMPT		   "OMAP730 P2 # "     /* Monitor Command Prompt   */
#define CFG_CBSIZE		   256		       /* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CFG_PBSIZE		   (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS		   16		       /* max number of command args   */
#define CFG_BARGSIZE		   CFG_CBSIZE	       /* Boot Argument Buffer Size    */

#define CFG_MEMTEST_START	   0x10000000	       /* memtest works on */
#define CFG_MEMTEST_END		   0x12000000	       /* 32 MB in DRAM	   */

#undef CFG_CLKS_IN_HZ		     /* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		   0x10000000	       /* default load address */

/* The OMAP730 has 3 general purpose MPU timers, they can be driven by
 * the RefClk (12Mhz) or by DPLL1. This time is further subdivided by a
 * local divisor.
 */

#define CFG_TIMERBASE		   0xFFFEC500	       /* use timer 1 */
#define CFG_PVT			   7		       /* 2^(pvt+1), divide by 256 */
#define CFG_HZ			   ((CONFIG_SYS_CLK_FREQ)/(2 << CFG_PVT))

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */

#define CONFIG_STACKSIZE	   (128*1024)	  /* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	   (4*1024)	  /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	   (4*1024)	  /* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */

#define CONFIG_NR_DRAM_BANKS	   1		  /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		   0x10000000	  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	   0x02000000	  /* 32 MB */

#if defined(CONFIG_CS0_BOOT)
#define PHYS_FLASH_1		   0x0C000000
#elif defined(CONFIG_CS3_BOOT)
#define PHYS_FLASH_1		   0x00000000
#else
#error Unknown Boot Chip-Select number
#endif

#define CFG_FLASH_BASE		   PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CFG_MAX_FLASH_BANKS	   1		  /* max number of memory banks */
#define PHYS_FLASH_SIZE		   0x02000000	  /* 32MB */
#define CFG_MAX_FLASH_SECT	   (259)	  /* max number of sectors on one chip */
/* addr of environment */
#define CFG_ENV_ADDR		   (CFG_FLASH_BASE + 0x020000)

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	    (20*CFG_HZ)	  /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	    (20*CFG_HZ)	  /* Timeout for Flash Write */

#define CFG_ENV_IS_IN_FLASH	   1
#define CFG_ENV_SIZE		   0x20000	  /* Total Size of Environment Sector */
#define CFG_ENV_OFFSET		   0x20000	  /* environment starts here  */

#endif	  /* ! __CONFIG_H */
