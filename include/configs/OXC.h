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
#define CONFIG_OXC		1

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/

#define CONFIG_IDENT_STRING     " [oxc] "

#define CONFIG_WATCHDOG		1
#define CONFIG_SHOW_ACTIVITY	1
#define CONFIG_SHOW_BOOT_PROGRESS 1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL | CFG_CMD_ELF)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any)	*/
#include <cmd_confdefs.h>

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

#define CONFIG_MISC_INIT_R	1		/* call misc_init_r() on init	*/

/*-----------------------------------------------------------------------
 * Boot options
 */

#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_GATEWAYIP	10.0.0.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_LOADADDR		0x10000
#define CONFIG_BOOTFILE		"/mnt/ide0/p2/usr/tftp/oxc.elf"
#define CONFIG_BOOTCOMMAND	"tftp 0x10000 ; bootelf 0x10000"
#define CONFIG_BOOTDELAY	10

#define CFG_OXC_GENERATE_IP	1		/* Generate IP automatically	*/
#define CFG_OXC_IPMASK		0x0A000000	/* 10.0.0.x			*/

/*-----------------------------------------------------------------------
 * PCI stuff
 */

#define CONFIG_PCI				/* include pci support		*/

#define CONFIG_NET_MULTI			/* Multi ethernet cards support */

#define CONFIG_EEPRO100				/* Ethernet Express PRO 100	*/
#define CFG_RX_ETH_BUFFER	8               /* use 8 rx buffer on eepro100  */

#define PCI_ENET0_IOADDR	0x80000000
#define PCI_ENET0_MEMADDR	0x80000000
#define	PCI_ENET1_IOADDR	0x81000000
#define	PCI_ENET1_MEMADDR	0x81000000

/*-----------------------------------------------------------------------
 * FLASH
 */

#define CFG_FLASH_PRELIMBASE	0xFF800000
#define CFG_FLASH_BASE		(0-flash_info[0].size)

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	32	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/*-----------------------------------------------------------------------
 * RAM
 */

#define CFG_SDRAM_BASE		0x00000000
#define CFG_MAX_RAM_SIZE	0x10000000

#define CFG_RESET_ADDRESS	0xFFF00100

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		0x00030000

#if (CFG_MONITOR_BASE < CFG_FLASH_PRELIMBASE)
# define CFG_RAMBOOT		1
#else
# undef CFG_RAMBOOT
#endif

#define CFG_INIT_RAM_ADDR	0x40000000
#define CFG_INIT_RAM_END	0x1000

#define CFG_GBL_DATA_SIZE	128
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()	*/

#define CFG_MEMTEST_START	0x00000000	/* memtest works on		*/
#define CFG_MEMTEST_END		0x04000000	/* 0 ... 32 MB in DRAM		*/

/*-----------------------------------------------------------------------
 * Memory mapping
 */

#define CFG_CPLD_BASE		0xff000000	/* CPLD registers */
#define CFG_CPLD_WATCHDOG	(CFG_CPLD_BASE)			/* Watchdog */
#define CFG_CPLD_RESET		(CFG_CPLD_BASE + 0x040000)	/* Minor resets */
#define CFG_UART_BASE		(CFG_CPLD_BASE + 0x700000)	/* debug UART */

/*-----------------------------------------------------------------------
 * NS16550 Configuration
 */

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	-4
#define CFG_NS16550_CLK		1843200
#define CFG_NS16550_COM1	CFG_UART_BASE

/*-----------------------------------------------------------------------
 * I2C Bus
 */

#define CONFIG_I2C		1		/* I2C support on ... */
#define CONFIG_HARD_I2C		1		/* ... hardware one */
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F		/* I2C slave address */

#define CFG_I2C_EXPANDER0_ADDR	0x20		/* PCF8574 expander 0 addrerr */
#define CFG_I2C_EXPANDER1_ADDR	0x21		/* PCF8574 expander 1 addrerr */
#define CFG_I2C_EXPANDER2_ADDR	0x26		/* PCF8574 expander 2 addrerr */

/*-----------------------------------------------------------------------
 * Environment
 */

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0xFFF30000	/* Offset of Environment Sector	*/
#define CFG_ENV_SIZE		0x00010000	/* Total Size of Environment Sector */
#define	CFG_ENV_IS_EMBEDDED	1		/* short-cut compile-time test	*/
#define CONFIG_ENV_OVERWRITE    1		/* Allow modifying the environment */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_CLK_FREQ  33000000	/* external frequency to pll */
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER  2

#define CFG_EUMB_ADDR		0xFC000000

/* MCCR1 */
#define CFG_ROMNAL		0	/* rom/flash next access time		*/
#define CFG_ROMFAL		19	/* rom/flash access time		*/

/* MCCR2 */
#define CFG_ASRISE		15	/* ASRISE=15 clocks			*/
#define CFG_ASFALL		3	/* ASFALL=3 clocks			*/
#define CFG_REFINT		1000	/* REFINT=1000 clocks			*/

/* MCCR3 */
#define CFG_BSTOPRE		0x35c	/* Burst To Precharge			*/
#define CFG_REFREC		7	/* Refresh to activate interval		*/
#define CFG_RDLAT		4	/* data latency from read command	*/

/* MCCR4 */
#define CFG_PRETOACT		2	/* Precharge to activate interval	*/
#define CFG_ACTTOPRE		5	/* Activate to Precharge interval	*/
#define CFG_ACTORW		2	/* Activate to R/W			*/
#define CFG_SDMODE_CAS_LAT	3	/* SDMODE CAS latency			*/
#define CFG_SDMODE_WRAP		0	/* SDMODE wrap type			*/
#define CFG_SDMODE_BURSTLEN	3	/* SDMODE Burst length 2=4, 3=8		*/
#define CFG_REGISTERD_TYPE_BUFFER   1

/* memory bank settings*/
/*
 * only bits 20-29 are actually used from these vales to set the
 * start/end address the upper two bits will be 0, and the lower 20
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
 are actually present. MSB is for bank #7, LSB is for bank #0.
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

/* stack in DCACHE @ 1GB (no backing mem) */
#define CFG_IBAT1L	(CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

/* PCI memory */
#define CFG_IBAT2L	(0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT2U	(0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

/* Flash, config addrs, etc */
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

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC8240 CPU			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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
