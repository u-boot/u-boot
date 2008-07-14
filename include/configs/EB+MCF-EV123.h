/*
 * Configuation settings for the BuS EB+MCF-EV123 boards.
 *
 * (C) Copyright 2005 BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
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

#ifndef _CONFIG_EB_MCF_EV123_H_
#define _CONFIG_EB_MCF_EV123_H_

#define  CONFIG_EB_MCF_EV123

#undef CFG_HALT_BEFOR_RAM_JUMP

/*
 * High Level Configuration Options (easy to change)
 */

#define	CONFIG_MCF52x2			/* define processor family */
#define CONFIG_M5282			/* define processor type */

#define CONFIG_MISC_INIT_R

#define CONFIG_MCFUART
#define CFG_UART_PORT		(0)
#define CONFIG_BAUDRATE 9600
#define CFG_BAUDRATE_TABLE { 9600 , 19200 , 38400 , 57600, 115200 }

#undef	CONFIG_MONITOR_IS_IN_RAM	/* define if monitor is started from a pre-loader */

#define CONFIG_BOOTCOMMAND "printenv"

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#ifndef CONFIG_MONITOR_IS_IN_RAM
#define CFG_ENV_ADDR		0xF003C000	/* End of 256K */
#define CFG_ENV_SECT_SIZE	0x4000
#define CFG_ENV_IS_IN_FLASH	1
/*
#define CFG_ENV_IS_EMBEDDED	1
#define CFG_ENV_ADDR_REDUND		0xF0018000
#define CFG_ENV_SECT_SIZE_REDUND	0x4000
*/
#else
#define CFG_ENV_ADDR		0xFFE04000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_IS_IN_FLASH	1
#endif


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

#undef CONFIG_CMD_LOADB
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET

#define CONFIG_MCFTMR

#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#	define CONFIG_NET_MULTI		1
#	define CONFIG_MII		1
#	define CONFIG_MII_INIT		1
#	define CFG_DISCOVER_PHY
#	define CFG_RX_ETH_BUFFER	8
#	define CFG_FAULT_ECHO_LINK_DOWN

#	define CFG_FEC0_PINMUX		0
#	define CFG_FEC0_MIIBASE		CFG_FEC0_IOBASE
#	define MCFFEC_TOUT_LOOP		50000
/* If CFG_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CFG_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	else
#		ifndef CFG_FAULT_ECHO_LINK_DOWN
#			define CFG_FAULT_ECHO_LINK_DOWN
#		endif
#	endif			/* CFG_DISCOVER_PHY */
#endif

#ifdef CONFIG_MCFFEC
#	define CONFIG_ETHADDR	00:CF:52:82:EB:01
#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#	define CONFIG_OVERWRITE_ETHADDR_ONCE
#endif				/* CONFIG_MCFFEC */

#define CONFIG_BOOTDELAY	5
#define CFG_PROMPT		"\nEV123 U-Boot> "
#define	CFG_LONGHELP				/* undef to save memory		*/

#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_LOAD_ADDR		0x20000

#define CFG_MEMTEST_START	0x100000
#define CFG_MEMTEST_END		0x400000
/*#define CFG_DRAM_TEST		1 */
#undef CFG_DRAM_TEST

/* Clock and PLL Configuration */
#define CFG_HZ			10000000
#define	CFG_CLK			58982400       /* 9,8304MHz * 6 */

/* PLL Configuration: Ext Clock * 6 (see table 9-4 of MCF user manual) */

#define CFG_MFD			0x01	/* PLL Multiplication Factor Devider */
#define CFG_RFD			0x00	/* PLL Reduce Frecuency Devider */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
#define	CFG_MBAR		0x40000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR       0x20000000
#define CFG_INIT_RAM_END	0x10000		/* End of used area in internal SRAM	*/
#define CFG_GBL_DATA_SIZE	64		/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE1		0x00000000
#define	CFG_SDRAM_SIZE1		16		/* SDRAM size in MB */

/*
#define CFG_SDRAM_BASE0		CFG_SDRAM_BASE1+CFG_SDRAM_SIZE1*1024*1024
#define	CFG_SDRAM_SIZE0		16	*/	/* SDRAM size in MB */

#define CFG_SDRAM_BASE		CFG_SDRAM_BASE1
#define	CFG_SDRAM_SIZE		CFG_SDRAM_SIZE1

#define CFG_FLASH_BASE		0xFFE00000
#define	CFG_INT_FLASH_BASE	0xF0000000
#define CFG_INT_FLASH_ENABLE	0x21

/* If M5282 port is fully implemented the monitor base will be behind
 * the vector table. */
#if (TEXT_BASE !=  CFG_INT_FLASH_BASE)
#define CFG_MONITOR_BASE	(TEXT_BASE + 0x400)
#else
#define CFG_MONITOR_BASE	(TEXT_BASE + 0x418) /* 24 Byte for CFM-Config */
#endif

#define CFG_MONITOR_LEN		0x20000
#define CFG_MALLOC_LEN		(256 << 10)
#define CFG_BOOTPARAMS_LEN	64*1024

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define	CFG_MAX_FLASH_SECT	35
#define	CFG_MAX_FLASH_BANKS	2
#define	CFG_FLASH_ERASE_TOUT	10000000
#define	CFG_FLASH_PROTECTION

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */

#define CFG_CS0_BASE		CFG_FLASH_BASE
#define CFG_CS0_SIZE		2*1024*1024
#define CFG_CS0_WIDTH		16
#define CFG_CS0_RO		0
#define CFG_CS0_WS		6

#define CFG_CS3_BASE		0xE0000000
#define CFG_CS3_SIZE		1*1024*1024
#define CFG_CS3_WIDTH		16
#define CFG_CS3_RO		0
#define CFG_CS3_WS		6

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CFG_PACNT		0x0000000	/* Port A D[31:24] */
#define CFG_PADDR		0x0000000
#define CFG_PADAT		0x0000000

#define CFG_PBCNT		0x0000000	/* Port B D[23:16] */
#define CFG_PBDDR		0x0000000
#define CFG_PBDAT		0x0000000

#define CFG_PCCNT		0x0000000	/* Port C D[15:08] */
#define CFG_PCDDR		0x0000000
#define CFG_PCDAT		0x0000000

#define CFG_PDCNT		0x0000000	/* Port D D[07:00] */
#define CFG_PCDDR		0x0000000
#define CFG_PCDAT		0x0000000

#define CFG_PEHLPAR		0xC0
#define CFG_PUAPAR		0x0F		/* UA0..UA3 = Uart 0 +1 */
#define CFG_DDRUA		0x05
#define CFG_PJPAR		0xFF;

/*-----------------------------------------------------------------------
 * CCM configuration
 */

#define	CFG_CCM_SIZ		0

/*---------------------------------------------------------------------*/
#endif	/* _CONFIG_M5282EVB_H */
/*---------------------------------------------------------------------*/
