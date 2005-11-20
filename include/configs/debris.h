/*
 * (C) Copyright 2001, 2002
 * Sangmoon Kim, Etin Systems, dogoil@etinsys.com.
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

/* Environments */

/* bootargs */
#define CONFIG_BOOTARGS \
	"console=ttyS0,9600 init=/linuxrc " \
	"root=/dev/nfs rw nfsroot=192.168.0.1:" \
	"/tftpboot/target " \
	"ip=192.168.0.2:192.168.0.1:192.168.0.1:" \
	"255.255.255.0:debris:eth0:none " \
	"mtdparts=phys:12m(root),-(kernel)"

/* bootcmd */
#define CONFIG_BOOTCOMMAND \
	"tftp 800000 pImage; " \
	"setenv bootargs console=ttyS0,9600 init=/linuxrc " \
	"root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:" \
	"${netmask}:${hostname}:eth0:none " \
	"mtdparts=phys:12m(root),-(kernel); " \
	"bootm 800000"

/* bootdelay */
#define CONFIG_BOOTDELAY	5	/* autoboot 5s */

/* baudrate */
#define CONFIG_BAUDRATE		9600	/* console baudrate = 9600bps	*/

/* loads_echo */
#define CONFIG_LOADS_ECHO	0	/* echo off for serial download */

/* ethaddr */
#undef CONFIG_ETHADDR

/* eth2addr */
#undef CONFIG_ETH2ADDR

/* eth3addr */
#undef CONFIG_ETH3ADDR

/* ipaddr */
#define CONFIG_IPADDR	192.168.0.2

/* serverip */
#define CONFIG_SERVERIP	192.168.0.1

/* autoload */
#undef CFG_AUTOLOAD

/* rootpath */
#define CONFIG_ROOTPATH /tftpboot/target

/* gatewayip */
#define CONFIG_GATEWAYIP 192.168.0.1

/* netmask */
#define CONFIG_NETMASK 255.255.255.0

/* hostname */
#define CONFIG_HOSTNAME debris

/* bootfile */
#define CONFIG_BOOTFILE pImage

/* loadaddr */
#define CONFIG_LOADADDR 800000

/* preboot */
#undef CONFIG_PREBOOT

/* clocks_in_mhz */
#undef CONFIG_CLOCKS_IN_MHZ


/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC824X		1
#define CONFIG_MPC8245		1
#define CONFIG_DEBRIS		1

#if 0
#define USE_DINK32		1
#else
#undef USE_DINK32
#endif

#define CONFIG_CONS_INDEX       1
#define CONFIG_BAUDRATE		9600
#define CONFIG_DRAM_SPEED	100		/* MHz */

#define CONFIG_COMMANDS		( CONFIG_CMD_DFL	| \
				  CFG_CMD_ASKENV	| \
				  CFG_CMD_CACHE		| \
				  CFG_CMD_DATE		| \
				  CFG_CMD_DHCP		| \
				  CFG_CMD_DIAG		| \
				  CFG_CMD_EEPROM	| \
				  CFG_CMD_ELF		| \
				  CFG_CMD_I2C		| \
				  CFG_CMD_JFFS2		| \
				  CFG_CMD_KGBD		| \
				  CFG_CMD_PCI		| \
				  CFG_CMD_PING		| \
				  CFG_CMD_SAVES		| \
				  CFG_CMD_SDRAM)
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

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_PCI				/* include pci support		*/
#define CONFIG_PCI_PNP

#define CONFIG_NET_MULTI		/* Multi ethernet cards support */
#define CONFIG_EEPRO100
#define CFG_RX_ETH_BUFFER	8	/* use 8 rx buffer on eepro100  */
#define CONFIG_EEPRO100_SROM_WRITE

#define PCI_ENET0_IOADDR	0x80000000
#define PCI_ENET0_MEMADDR	0x80000000
#define	PCI_ENET1_IOADDR	0x81000000
#define	PCI_ENET1_MEMADDR	0x81000000
/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MAX_RAM_SIZE	0x20000000
#define CONFIG_VERY_BIG_RAM

#define CFG_RESET_ADDRESS	0xFFF00100

#if defined (USE_DINK32)
#define CFG_MONITOR_LEN		0x00040000
#define CFG_MONITOR_BASE	0x00090000
#define CFG_RAMBOOT		1
#define CFG_INIT_RAM_ADDR	(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_INIT_RAM_END	0x10000
#define CFG_GBL_DATA_SIZE	256  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET
#else
#undef	CFG_RAMBOOT
#define CFG_MONITOR_LEN		0x00040000
#define CFG_MONITOR_BASE	TEXT_BASE

/*#define CFG_GBL_DATA_SIZE    256*/
#define CFG_GBL_DATA_SIZE	128

#define CFG_INIT_RAM_ADDR     0x40000000
#define CFG_INIT_RAM_END      0x1000
#define CFG_GBL_DATA_OFFSET  (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

#endif

#define CFG_FLASH_BASE		0x7C000000
#define CFG_FLASH_SIZE		(16*1024*1024)	/* debris has tiny eeprom	*/

#define CFG_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()	*/

#define CFG_MEMTEST_START	0x00000000	/* memtest works on		*/
#define CFG_MEMTEST_END		0x04000000	/* 0 ... 32 MB in DRAM		*/

#define CFG_EUMB_ADDR		0xFC000000

#define CFG_FLASH_RANGE_BASE	0xFF000000	/* flash memory address range	*/
#define CFG_FLASH_RANGE_SIZE	0x01000000
#define FLASH_BASE0_PRELIM	0x7C000000	/* debris flash		*/

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */

/* Use first bank for JFFS2, second bank contains U-Boot.
 *
 * Note: fake mtd_id's used, no linux mtd map file.
 */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=debris-0"
#define MTDPARTS_DEFAULT	"mtdparts=debris-0:-(jffs2)"
*/

#define CFG_ENV_IS_IN_NVRAM      1
#define CONFIG_ENV_OVERWRITE     1
#define CFG_NVRAM_ACCESS_ROUTINE 1
#define CFG_ENV_ADDR		0xFF000000 /* right at the start of NVRAM  */
#define CFG_ENV_SIZE		0x400	/* Size of the Environment - 8K	   */
#define CFG_ENV_OFFSET		0	/* starting right at the beginning */

#define CFG_NVRAM_BASE_ADDR	0xff000000

/*
 * CFG_NVRAM_BASE_ADDR + CFG_NVRAM_VXWORKS_OFFS =
 * NV_RAM_ADDRS + NV_BOOT_OFFSET + NV_ENET_OFFSET
 */
#define CFG_NVRAM_VXWORKS_OFFS	0x6900

/*
 * select i2c support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#define CONFIG_HARD_I2C		1		/* To enable I2C support	*/
#undef  CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#ifdef CONFIG_SOFT_I2C
#error "Soft I2C is not configured properly.  Please review!"
#define I2C_PORT		3               /* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE		(iop->pdir |=  0x00010000)
#define I2C_TRISTATE		(iop->pdir &= ~0x00010000)
#define I2C_READ		((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)		if(bit) iop->pdat |=  0x00010000; \
				else    iop->pdat &= ~0x00010000
#define I2C_SCL(bit)		if(bit) iop->pdat |=  0x00020000; \
				else    iop->pdat &= ~0x00020000
#define I2C_DELAY		udelay(5)	/* 1/4 I2C clock duration */
#endif /* CONFIG_SOFT_I2C */

#define CFG_I2C_EEPROM_ADDR	0x57		/* EEPROM IS24C02		*/
#define CFG_I2C_EEPROM_ADDR_LEN	1		/* Bytes of address		*/
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CFG_FLASH_BANKS		{ FLASH_BASE0_PRELIM }

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */

/*
 * NS16550 Configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL

#define CFG_NS16550_REG_SIZE	1

#define CFG_NS16550_CLK		7372800

#define CFG_NS16550_COM1	0xFF080000
#define CFG_NS16550_COM2	(CFG_NS16550_COM1 + 8)
#define CFG_NS16550_COM3	(CFG_NS16550_COM1 + 16)
#define CFG_NS16550_COM4	(CFG_NS16550_COM1 + 24)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_CLK_FREQ  33333333	/* external frequency to pll */
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER 3

#define CFG_DLL_EXTEND		0x00
#define CFG_PCI_HOLD_DEL	0x20

#define CFG_ROMNAL	15	/* rom/flash next access time */
#define CFG_ROMFAL	31	/* rom/flash access time */

#define CFG_REFINT	430	/* # of clocks between CBR refresh cycles */

#define CFG_DBUS_SIZE2	1	/* set for 8-bit RCS1, clear for 32,64 */

/* the following are for SDRAM only*/
#define CFG_BSTOPRE	121	/* Burst To Precharge, sets open page interval */
#define CFG_REFREC	8	/* Refresh to activate interval		*/
#define CFG_RDLAT	4	/* data latency from read command	*/
#define CFG_PRETOACT	3	/* Precharge to activate interval	*/
#define CFG_ACTTOPRE	5	/* Activate to Precharge interval	*/
#define CFG_ACTORW		3	/* Activate to R/W			*/
#define CFG_SDMODE_CAS_LAT	3	/* SDMODE CAS latency			*/
#define CFG_SDMODE_WRAP		0	/* SDMODE wrap type			*/
#if 0
#define CFG_SDMODE_BURSTLEN	2	/* OBSOLETE!  SDMODE Burst length 2=4, 3=8		*/
#endif

#define CFG_REGISTERD_TYPE_BUFFER   1
#define CFG_EXTROM 1
#define CFG_REGDIMM 0


/* memory bank settings*/
/*
 * only bits 20-29 are actually used from these vales to set the
 * start/end address the upper two bits will be 0, and the lower 20
 * bits will be set to 0x00000 for a start address, or 0xfffff for an
 * end address
 */
#define CFG_BANK0_START		0x00000000
#define CFG_BANK0_END		(0x4000000 - 1)
#define CFG_BANK0_ENABLE	1
#define CFG_BANK1_START		0x04000000
#define CFG_BANK1_END		(0x8000000 - 1)
#define CFG_BANK1_ENABLE	1
#define CFG_BANK2_START		0x3ff00000
#define CFG_BANK2_END		0x3fffffff
#define CFG_BANK2_ENABLE	0
#define CFG_BANK3_START		0x3ff00000
#define CFG_BANK3_END		0x3fffffff
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

#define CFG_ODCR		0x75	/* configures line driver impedances,	*/
					/* see 8240 book for bit definitions	*/
#define CFG_PGMAX		0x32	/* how long the 8240 retains the	*/
					/* currently accessed page in memory	*/
					/* see 8240 book for details		*/

/* SDRAM 0 - 256MB */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* stack in DCACHE @ 1GB (no backing mem) */
#if defined(USE_DINK32)
#define CFG_IBAT1L	(0x40000000 | BATL_PP_00 )
#define CFG_IBAT1U	(0x40000000 | BATU_BL_128K )
#else
#define CFG_IBAT1L	(CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#endif

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
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

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


/* values according to the manual */

#define CONFIG_DRAM_50MHZ	1
#define CONFIG_SDRAM_50MHZ

#define CONFIG_DISK_SPINUP_TIME 1000000

#endif	/* __CONFIG_H */
