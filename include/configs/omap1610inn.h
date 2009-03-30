/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
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
#define CONFIG_ARM926EJS	1	/* This is an arm926ejs CPU core  */
#define CONFIG_OMAP	1			/* in a TI OMAP core    */
#define CONFIG_OMAP1610	1		/* which is in a 1610  */
#define CONFIG_INNOVATOROMAP1610	1	/*  a Innovator Board  */
#define CONFIG_MACH_OMAP_INNOVATOR	/* Select board mach-type */

/* input clock of PLL */
/* the OMAP1610 Innovator has 12MHz input clock */
#define CONFIG_SYS_CLK_FREQ	12000000

#undef CONFIG_USE_IRQ	/* we don't need IRQ/FIQ stuff */

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
/*
*/
#define CONFIG_DRIVER_LAN91C96
#define CONFIG_LAN91C96_BASE 0x04000300
#define CONFIG_LAN91C96_EXT_PHY

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)	/* can be 12M/32Khz or 48Mhz */
#define CONFIG_SYS_NS16550_COM1	0xfffb0000	/* uart1, bluetooth uart on helen */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1		1		/* we use SERIAL 1 on OMAP1610 Innovator */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE	115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH


#include <configs/omap1510.h>

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS	"mem=32M console=ttyS0,115200n8 noinitrd \
				root=/dev/nfs rw nfsroot=157.87.82.48:\
				/home/a0875451/mwd/myfs/target ip=dhcp"
#define CONFIG_NETMASK	255.255.254.0	/* talk on MY local net */
#define CONFIG_IPADDR	156.117.97.156	/* static IP I currently own */
#define CONFIG_SERVERIP	156.117.97.139	/* current IP of my dev pc */
#define CONFIG_BOOTFILE	"uImage"	/* file to load */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1	/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory     */
#define CONFIG_SYS_PROMPT	"OMAP1610 Innovator # "	/* Monitor Command Prompt   */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args   */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START	0x10000000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END	0x12000000	/* 32 MB in DRAM    */

#define CONFIG_SYS_LOAD_ADDR	0x10000000	/* default load address */

/* The 1610 has 6 timers, they can be driven by the RefClk (12Mhz) or by
 * DPLL1. This time is further subdivided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE	0xFFFEC500	/* use timer 1 */
#define CONFIG_SYS_PTV		7	/* 2^(PTV+1), divide by 256 */
#define CONFIG_SYS_HZ		((CONFIG_SYS_CLK_FREQ)/(2 << CONFIG_SYS_PTV))

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
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x10000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000	/* 32 MB */

#define PHYS_FLASH_1_BM1	0x00000000	/* Flash Bank #1 if booting from flash */
#define PHYS_FLASH_1_BM0	0x0C000000	/* Flash Bank #1 if booting from RAM */

#ifdef CONFIG_CS_AUTOBOOT			/* Determine CS assignment in runtime */

#ifndef __ASSEMBLY__
extern unsigned long omap_flash_base;		/* set in flash__init */
#endif
#define CONFIG_SYS_FLASH_BASE		omap_flash_base

#elif defined(CONFIG_CS0_BOOT)

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1_BM0

#else

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1_BM1

#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define PHYS_FLASH_SIZE	0x02000000	/* 32MB */
#define CONFIG_SYS_MAX_FLASH_SECT	(259)	/* max number of sectors on one chip */
/* addr of environment */
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + 0x020000)

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(20*CONFIG_SYS_HZ)	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(20*CONFIG_SYS_HZ)	/* Timeout for Flash Write */

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE	0x20000	/* Total Size of Environment Sector */
#define CONFIG_ENV_OFFSET	0x20000	/* environment starts here  */

#endif							/* __CONFIG_H */
