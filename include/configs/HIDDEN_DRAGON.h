/*
 * (C) Copyright 2004
 * Yusdi Santoso, Adaptec Inc., yusdi_santoso@adaptec.com
 *
 * (C) Copyright 2001, 2002
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
#define CONFIG_HIDDEN_DRAGON	1

#if 0
#define USE_DINK32		1
#else
#undef USE_DINK32
#endif

#define CONFIG_CONS_INDEX	3		/* set to '3' for on-chip DUART */
#define CONFIG_BAUDRATE		9600
#define CONFIG_DRAM_SPEED	100		/* MHz				*/

#define CONFIG_COMMANDS		( CONFIG_CMD_DFL | \
				  CFG_CMD_EEPROM | \
				  CFG_CMD_ELF	| \
				  CFG_CMD_I2C	| \
				  CFG_CMD_NET	| \
				  CFG_CMD_PCI	| \
				  CFG_CMD_PING	)

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
#undef CONFIG_PCI_PNP

#define CONFIG_NET_MULTI			/* Multi ethernet cards support */

#define CFG_RX_ETH_BUFFER	8		/* use 8 rx buffer on eepro100	*/

#define PCI_ENET0_IOADDR	0x80000000
#define PCI_ENET0_MEMADDR	0x80000000
#define PCI_ENET1_IOADDR	0x81000000
#define PCI_ENET1_MEMADDR	0x81000000

#define CONFIG_RTL8139
#define _IO_BASE	    0x00000000
/* This macro is used by RTL8139 but not defined in PPC architecture */
#define KSEG1ADDR(x)	    (x)
/* Make sure the ethaddr can be overwritten
   TODO: Remove this on final product
*/
#define CONFIG_ENV_OVERWRITE

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MAX_RAM_SIZE	0x02000000

#define CFG_RESET_ADDRESS	0xFFF00100

#if defined (USE_DINK32)
#define CFG_MONITOR_LEN		0x00030000
#define CFG_MONITOR_BASE	0x00090000
#define CFG_RAMBOOT		1
#define CFG_INIT_RAM_ADDR	(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_INIT_RAM_END	0x10000
#define CFG_GBL_DATA_SIZE	256  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET
#else
#undef	CFG_RAMBOOT
#define CFG_MONITOR_LEN		0x00030000
#define CFG_MONITOR_BASE	TEXT_BASE

#define CFG_GBL_DATA_SIZE	128

#define CFG_INIT_RAM_ADDR     0x40000000
#define CFG_INIT_RAM_END      0x1000
#define CFG_GBL_DATA_OFFSET  (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

#endif

#define CFG_FLASH_BASE		0xFFE00000
#define CFG_FLASH_SIZE		(2 * 1024 * 1024)	/* Unity has onboard 1MByte flash */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x00004000	/* Offset of Environment Sector */
#define CFG_ENV_SIZE		0x00002000	/* Total Size of Environment Sector */

#define CFG_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()	*/

#define CFG_MEMTEST_START	0x00000000	/* memtest works on		*/
#define CFG_MEMTEST_END		0x02000000	/* 0 ... 32 MB in DRAM		*/

#define CFG_EUMB_ADDR		0xFC000000

#define CFG_ISA_MEM		0xFD000000
#define CFG_ISA_IO		0xFE000000

#define CFG_FLASH_RANGE_BASE	0xFFE00000	/* flash memory address range	*/
#define CFG_FLASH_RANGE_SIZE	0x00200000
#define FLASH_BASE0_PRELIM	0xFFE00000	/* processor board flash	*/

/*
 * select i2c support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#define CONFIG_HARD_I2C		1		/* To enable I2C support	*/
#undef	CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#ifdef CONFIG_SOFT_I2C
#error "Soft I2C is not configured properly.  Please review!"
#define I2C_PORT		3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE		(iop->pdir |=  0x00010000)
#define I2C_TRISTATE		(iop->pdir &= ~0x00010000)
#define I2C_READ		((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)		if(bit) iop->pdat |=  0x00010000; \
				else	iop->pdat &= ~0x00010000
#define I2C_SCL(bit)		if(bit) iop->pdat |=  0x00020000; \
				else	iop->pdat &= ~0x00020000
#define I2C_DELAY		udelay(5)	/* 1/4 I2C clock duration */
#endif /* CONFIG_SOFT_I2C */

#define CFG_I2C_EEPROM_ADDR	0x57		/* EEPROM IS24C02		*/
#define CFG_I2C_EEPROM_ADDR_LEN 1		/* Bytes of address		*/
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CFG_FLASH_BANKS		{ FLASH_BASE0_PRELIM }

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */


#define CFG_WINBOND_83C553	1	/*has a winbond bridge			*/
#define CFG_USE_WINBOND_IDE	0	/*use winbond 83c553 internal IDE ctrlr */
#define CFG_WINBOND_ISA_CFG_ADDR    0x80005800	/*pci-isa bridge config addr	*/
#define CFG_WINBOND_IDE_CFG_ADDR    0x80005900	/*ide config addr		*/

#define CFG_IDE_MAXBUS		2   /* max. 2 IDE busses	*/
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

/* TODO: Change this to VIA686A */

/*
 * NS87308 Configuration
 */
#define CFG_NS87308			/* Nat Semi super-io controller on ISA bus */

#define CFG_NS87308_BADDR_10	1

#define CFG_NS87308_DEVS	( CFG_NS87308_UART1   | \
				  CFG_NS87308_UART2   | \
				  CFG_NS87308_POWRMAN | \
				  CFG_NS87308_RTC_APC )

#undef	CFG_NS87308_PS2MOD

#define CFG_NS87308_CS0_BASE	0x0076
#define CFG_NS87308_CS0_CONF	0x30
#define CFG_NS87308_CS1_BASE	0x0075
#define CFG_NS87308_CS1_CONF	0x30
#define CFG_NS87308_CS2_BASE	0x0074
#define CFG_NS87308_CS2_CONF	0x30

/*
 * NS16550 Configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL

#define CFG_NS16550_REG_SIZE	1

#if (CONFIG_CONS_INDEX > 2)
#define CFG_NS16550_CLK		CONFIG_DRAM_SPEED*1000000
#else
#define CFG_NS16550_CLK		1843200
#endif

#define CFG_NS16550_COM1	(CFG_ISA_IO + CFG_NS87308_UART1_BASE)
#define CFG_NS16550_COM2	(CFG_ISA_IO + CFG_NS87308_UART2_BASE)
#define CFG_NS16550_COM3	(CFG_EUMB_ADDR + 0x4500)
#define CFG_NS16550_COM4	(CFG_EUMB_ADDR + 0x4600)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_CLK_FREQ  33333333	/* external frequency to pll */

#define CFG_ROMNAL		7	/*rom/flash next access time		*/
#define CFG_ROMFAL		11	/*rom/flash access time			*/

#define CFG_REFINT	430	/* no of clock cycles between CBR refresh cycles */

/* the following are for SDRAM only*/
#define CFG_BSTOPRE	121	/* Burst To Precharge, sets open page interval */
#define CFG_REFREC		8	/* Refresh to activate interval		*/
#define CFG_RDLAT		4	/* data latency from read command	*/
#define CFG_PRETOACT		3	/* Precharge to activate interval	*/
#define CFG_ACTTOPRE		5	/* Activate to Precharge interval	*/
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
#define CFG_BANK0_END		(CFG_MAX_RAM_SIZE - 1)
#define CFG_BANK0_ENABLE	1
#define CFG_BANK1_START		0x3ff00000
#define CFG_BANK1_END		0x3fffffff
#define CFG_BANK1_ENABLE	0
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

#define CFG_ODCR		0xff	/* configures line driver impedances,	*/
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
#define CFG_MAX_FLASH_SECT	36	/* max number of sectors on one chip	*/

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

#undef	NR_8259_INTS
#define NR_8259_INTS		1

#define CONFIG_DISK_SPINUP_TIME 1000000

#endif	/* __CONFIG_H */
