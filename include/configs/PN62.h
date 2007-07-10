/*
 * (C) Copyright 2002
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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
#define CONFIG_MPC8240		1
#define CONFIG_PN62		1

#define CONFIG_CONS_INDEX	1


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

#define CONFIG_CMD_PCI
#define CONFIG_CMD_BSP

#undef CONFIG_CMD_AUTOSCRIPT
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_ENV
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS


#define CONFIG_BAUDRATE		19200	/* console baudrate		*/

#define CONFIG_BOOTDELAY	1	/* autoboot after n seconds	*/

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_SERVERIP		10.0.0.201
#define CONFIG_IPADDR 		10.0.0.200
#define CONFIG_ROOTPATH		/opt/eldk/ppc_82xx
#define CONFIG_NETMASK		255.255.255.0
#undef CONFIG_BOOTARGS
#if 0
/* Boot Linux with NFS root filesystem */
#define CONFIG_BOOTCOMMAND \
			"setenv verify y;" \
       			"setenv bootargs console=ttyS0,19200 mem=31M quiet " \
			"root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " \
			"ip=${ipaddr}:${serverip}::${netmask}:pn62:eth0:off;" \
			"loadp 100000; bootm"
			/* "tftpboot 100000 uImage; bootm" */
#else
/* Boot Linux with RAMdisk based filesystem (initrd, BusyBox) */
#define CONFIG_BOOTCOMMAND \
			"setenv verify n;" \
       			"setenv bootargs console=ttyS0,19200 mem=31M quiet " \
			"root=/dev/ram rw " \
			"ip=${ipaddr}:${serverip}::${netmask}:pn62:eth0:off;" \
			"loadp 200000; bootm"
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		1		/* undef to save memory		*/
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt	*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size	*/
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_LOAD_ADDR		0x00100000	/* default load address		*/
#define CFG_HZ			1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_PRAM		1024		/* reserve 1 MB protected RAM	*/

#define CONFIG_MISC_INIT_R	1		/* call misc_init_r() on init	*/

#define CONFIG_HAS_ETH1		1		/* add support for eth1addr	*/

#define CONFIG_SHOW_BOOT_PROGRESS 1		/* Show boot progress on LEDs   */

/*
 * PCI stuff
 */
#define CONFIG_PCI				/* include pci support		*/
#define CONFIG_PCI_PNP				/* we need Plug 'n Play		*/
#if 0
#define CONFIG_PCI_SCAN_SHOW			/* show PCI auto-scan at boot	*/
#endif

/*
 * Networking stuff
 */
#define CONFIG_NET_MULTI       			/* Multi ethernet cards support */

#define CONFIG_PCNET				/* there are 2 AMD PCnet 79C973	*/
#define CONFIG_PCNET_79C973

#define _IO_BASE		0xfe000000	/* points to PCI I/O space	*/


/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MAX_RAM_SIZE	0x10000000

#define CFG_RESET_ADDRESS	0xfff00100

#undef	CFG_RAMBOOT
#define CFG_MONITOR_LEN		0x00030000
#define CFG_MONITOR_BASE	TEXT_BASE

/*#define CFG_GBL_DATA_SIZE    256*/
#define CFG_GBL_DATA_SIZE	128

#define CFG_INIT_RAM_ADDR   	0x40000000
#define CFG_INIT_RAM_END    	0x1000
#define CFG_GBL_DATA_OFFSET 	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)


#define CFG_NO_FLASH		1		/* There is no FLASH memory	*/

#define CFG_ENV_IS_NOWHERE	1		/* Store ENV in memory only	*/
#define CFG_ENV_OFFSET		0x00004000	/* Offset of Environment Sector */
#define CFG_ENV_SIZE		0x00002000	/* Total Size of Environment Sector */

#define CFG_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()	*/

#define CFG_MEMTEST_START	0x00004000	/* memtest works on		*/
#define CFG_MEMTEST_END		0x01f00000	/* 0 ... 32 MB in DRAM		*/

/*
 * Serial port configuration
 */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_NS16550
#define CFG_NS16550_SERIAL

#define CFG_NS16550_REG_SIZE	1

#define CFG_NS16550_CLK		1843200

#define CFG_NS16550_COM1	0xff800008
#define CFG_NS16550_COM2	0xff800000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_CLK_FREQ  33333333	/* external frequency to pll */
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER  3

#define CFG_EUMB_ADDR		0xFCE00000

/* MCCR1 */
#define CFG_ROMNAL		3	/* rom/flash next access time		*/
#define CFG_ROMFAL		7	/* rom/flash access time		*/

/* MCCR2 */
#define CFG_ASRISE		6	/* ASRISE in clocks			*/
#define CFG_ASFALL		12	/* ASFALL in clocks			*/
#define CFG_REFINT		5600	/* REFINT in clocks			*/

/* MCCR3 */
#define CFG_BSTOPRE		0x3cf	/* Burst To Precharge			*/
#define CFG_REFREC		2	/* Refresh to activate interval		*/
#define CFG_RDLAT		3	/* data latency from read command	*/

/* MCCR4 */
#define CFG_PRETOACT		1	/* Precharge to activate interval	*/
#define CFG_ACTTOPRE		3	/* Activate to Precharge interval	*/
#define CFG_ACTORW		2	/* Activate to R/W			*/
#define CFG_SDMODE_CAS_LAT	2	/* SDMODE CAS latency			*/
#define CFG_SDMODE_WRAP		0	/* SDMODE Wrap type			*/
#define CFG_SDMODE_BURSTLEN	2	/* SDMODE Burst length 2=4, 3=8		*/
#define CFG_REGISTERD_TYPE_BUFFER   1

/* Memory bank settings:
 *
 * only bits 20-29 are actually used from these vales to set the
 * start/qend address the upper two bits will be 0, and the lower 20
 * bits will be set to 0x00000 for a start address, or 0xfffff for an
 * end address
 */
#define CFG_BANK0_START		0x00000000
#define CFG_BANK0_END		(CFG_MAX_RAM_SIZE - 1)
#define CFG_BANK0_ENABLE	1
#define CFG_BANK1_START		0x00000000
#define CFG_BANK1_END		0x00000000
#define CFG_BANK1_ENABLE	0
#define CFG_BANK2_START		0x00000000
#define CFG_BANK2_END		0x00000000
#define CFG_BANK2_ENABLE	0
#define CFG_BANK3_START		0x00000000
#define CFG_BANK3_END		0x00000000
#define CFG_BANK3_ENABLE	0
#define CFG_BANK4_START		0x00000000
#define CFG_BANK4_END		0x00000000
#define CFG_BANK4_ENABLE	0
#define CFG_BANK5_START		0x00000000
#define CFG_BANK5_END		0x00000000
#define CFG_BANK5_ENABLE	0
#define CFG_BANK6_START		0x00000000
#define CFG_BANK6_END		0x00000000
#define CFG_BANK6_ENABLE	0
#define CFG_BANK7_START		0x00000000
#define CFG_BANK7_END		0x00000000
#define CFG_BANK7_ENABLE	0

/*
 * Memory bank enable bitmask, specifying which of the banks defined above
 * are actually present. MSB is for bank #7, LSB is for bank #0.
 */
#define CFG_BANK_ENABLE		0x01

#define CFG_ODCR		0xff	/* configures line driver impedances,	*/
					/* see 8240 book for bit definitions	*/
#define CFG_PGMAX		0x32	/* how long the 8240 retains the	*/
					/* currently accessed page in memory	*/
					/* see 8240 book for details		*/

/* SDRAM 0 - 256MB */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT1L	(CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

/* PCI memory space */
#define CFG_IBAT2L	(0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT2U	(0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

/* Config addrs, etc */
#define CFG_IBAT3L	(0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT3U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U
#define CFG_DBAT3L	CFG_IBAT3L
#define CFG_DBAT3U	CFG_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC8240 CPU			*/
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/


#endif	/* __CONFIG_H */
