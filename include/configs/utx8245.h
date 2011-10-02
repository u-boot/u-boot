/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Gregory E. Allen, gallen@arlut.utexas.edu
 * Matthew E. Karger, karger@arlut.utexas.edu
 * Applied Research Laboratories, The University of Texas at Austin
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
 * Configuration settings for the utx8245 board.
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
#define CONFIG_UTX8245		1

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

#define DEBUG				1

#define CONFIG_IDENT_STRING     " [UTX5] "

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		57600
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_BOOTDELAY	2
#define CONFIG_AUTOBOOT_PROMPT	"autoboot in %d seconds\n", bootdelay
#define CONFIG_BOOTCOMMAND	"run nfsboot"	/* autoboot command	*/
#define CONFIG_BOOTARGS		"root=/dev/ram console=ttyS0,57600" /* RAMdisk */
#define CONFIG_ETHADDR		00:AA:00:14:00:05	/* UTX5 */
#define CONFIG_SERVERIP		10.8.17.105	/* Spree */
#define CONFIG_SYS_TFTP_LOADADDR	10000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr=FFA00000\0" \
	"ramdisk_addr=FF800000\0" \
	"u-boot_startaddr=FFB00000\0" \
	"u-boot_endaddr=FFB2FFFF\0" \
	"nfsargs=setenv bootargs console=ttyS0,${baudrate} root=/dev/nfs rw \
nfsroot=${nfsrootip}:${rootpath} ip=dhcp\0" \
	"ramargs=setenv bootargs console=ttyS0,${baudrate} root=/dev/ram0\0" \
	"smargs=setenv bootargs console=ttyS0,${baudrate} root=/dev/mtdblock1 ro\0" \
	"fwargs=setenv bootargs console=ttyS0,${baudrate} root=/dev/sda2 ro\0" \
	"nfsboot=run nfsargs;bootm ${kernel_addr}\0" \
	"ramboot=run ramargs;bootm ${kernel_addr} ${ramdisk_addr}\0" \
	"smboot=run smargs;bootm ${kernel_addr} ${ramdisk_addr}\0" \
	"fwboot=run fwargs;bootm ${kernel_addr} ${ramdisk_addr}\0" \
	"update_u-boot=tftp ${loadaddr} /bdi2000/u-boot.bin;protect off \
${u-boot_startaddr} ${u-boot_endaddr};era ${u-boot_startaddr} \
${u-boot_endaddr};cp.b ${loadaddr} ${u-boot_startaddr} ${filesize};\
protect on ${u-boot_startaddr} ${u-boot_endaddr}"

#define CONFIG_ENV_OVERWRITE


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

#define CONFIG_CMD_BDI
#define CONFIG_CMD_PCI
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_IMI
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_DATE


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "			/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE	256				/* Console I/O Buffer Size	*/

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR	0x00100000	/* Default load address		*/


/*-----------------------------------------------------------------------
 * PCI configuration
 *-----------------------------------------------------------------------
 */
#define CONFIG_PCI				/* include pci support		*/
#undef CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_EEPRO100
#define CONFIG_SYS_RX_ETH_BUFFER	8               /* use 8 rx buffer on eepro100  */
#define CONFIG_EEPRO100_SROM_WRITE

#define PCI_ENET0_IOADDR	0xF0000000
#define PCI_ENET0_MEMADDR	0xF0000000

#define PCI_FIREWIRE_IOADDR		0xF1000000
#define PCI_FIREWIRE_MEMADDR	0xF1000000
/*
#define PCI_ENET0_IOADDR	0xFE000000
#define PCI_ENET0_MEMADDR	0x80000000

#define PCI_FIREWIRE_IOADDR	0x81000000
#define PCI_FIREWIRE_MEMADDR	0x81000000
*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE	    0x00000000
#define CONFIG_SYS_MAX_RAM_SIZE    0x10000000	/* 256MB  */
/*#define CONFIG_SYS_VERY_BIG_RAM	1 */

/* FLASH_BASE is FF800000, with 4MB on RCS0, but the reset vector
 * is actually located at FFF00100.  Therefore, U-Boot is
 * physically located at 0xFFB0_0000, but is also mirrored at
 * 0xFFF0_0000.
 */
#define CONFIG_SYS_RESET_ADDRESS   0xFFF00100

#define CONFIG_SYS_EUMB_ADDR	    0xFC000000

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MONITOR_LEN	    (256 << 10) /* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN	    (128 << 10) /* Reserve 128 kB for malloc()	*/

/*#define CONFIG_SYS_DRAM_TEST		1 */
#define CONFIG_SYS_MEMTEST_START   0x00003000	/* memtest works on	0...256 MB	*/
#define CONFIG_SYS_MEMTEST_END	    0x0ff8ffa7	/* in SDRAM, skips exception */
										/* vectors and U-Boot */


/*--------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 *------------------------------------------------------------------*/
#define CONFIG_SYS_INIT_DATA_SIZE    128	/* Size in bytes reserved for */
									/* initial data */
#define CONFIG_SYS_INIT_RAM_ADDR     0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE      0x1000
#define CONFIG_SYS_INIT_DATA_OFFSET  (CONFIG_SYS_INIT_RAM_SIZE - CONFIG_SYS_INIT_DATA_SIZE)
#define CONFIG_SYS_GBL_DATA_OFFSET  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*--------------------------------------------------------------------
 * NS16550 Configuration
 *------------------------------------------------------------------*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_SYS_NS16550_REG_SIZE	1

#if (CONFIG_CONS_INDEX == 1 || CONFIG_CONS_INDEX == 2)
#	define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#else
#	define CONFIG_SYS_NS16550_CLK 33000000
#endif

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_EUMB_ADDR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_EUMB_ADDR + 0x4600)
#define CONFIG_SYS_NS16550_COM3	0xFF000000
#define CONFIG_SYS_NS16550_COM4	0xFF000008

/*--------------------------------------------------------------------
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 *------------------------------------------------------------------*/

#define CONFIG_SYS_CLK_FREQ  33000000
#define CONFIG_SYS_HZ				1000

/*#define CONFIG_SYS_ETH_DEV_FN	     0x7800 */
/*#define CONFIG_SYS_ETH_IOBASE	     0x00104000 */

/*--------------------------------------------------------------------
 * I2C Configuration
 *------------------------------------------------------------------*/
#if 1
#define CONFIG_HARD_I2C		1		/* To enable I2C support	*/
#undef  CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CONFIG_SYS_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F
#endif

#define CONFIG_RTC_PCF8563	1		/* enable I2C support for */
									/* Philips PCF8563 RTC */
#define CONFIG_SYS_I2C_RTC_ADDR	0x51	/* Philips PCF8563 RTC address */

/*--------------------------------------------------------------------
 *	Memory Control Configuration Register values
 *	- see sec. 4.12 of MPC8245 UM
 *------------------------------------------------------------------*/

/**** MCCR1 ****/
#define CONFIG_SYS_ROMNAL	    0
#define CONFIG_SYS_ROMFAL	    10		/* (tacc=70ns)*mem_freq - 2,
									mem_freq = 100MHz */

#define CONFIG_SYS_BANK7_ROW	0		/* SDRAM bank 7-0 row address */
#define CONFIG_SYS_BANK6_ROW	0		/*	bit count */
#define CONFIG_SYS_BANK5_ROW	0
#define CONFIG_SYS_BANK4_ROW	0
#define CONFIG_SYS_BANK3_ROW	0
#define CONFIG_SYS_BANK2_ROW	0
#define CONFIG_SYS_BANK1_ROW	2
#define CONFIG_SYS_BANK0_ROW	2

/**** MCCR2, refresh interval clock cycles ****/
#define CONFIG_SYS_REFINT	    480	    /* 33 MHz SDRAM clock was 480 */

/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4. */
#define CONFIG_SYS_BSTOPRE	    1023	/* burst to precharge[0..9], */
								/* sets open page interval */

/**** MCCR3 ****/
#define CONFIG_SYS_REFREC	    7	    /* Refresh to activate interval, trc */

/**** MCCR4 ****/
#define CONFIG_SYS_PRETOACT	    2	    /* trp */
#define CONFIG_SYS_ACTTOPRE	    7	    /* trcd + (burst length - 1) + trdl */
#define CONFIG_SYS_SDMODE_CAS_LAT  3	    /* SDMODE CAS latancy */
#define CONFIG_SYS_SDMODE_WRAP	    0	    /* SDMODE wrap type, sequential */
#define CONFIG_SYS_ACTORW		2		/* trcd min */
#define CONFIG_SYS_DBUS_SIZE2		1		/* set for 8-bit RCS1, clear for 32,64 */
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER 1
#define CONFIG_SYS_EXTROM	    0			/* we don't need extended ROM space */
#define CONFIG_SYS_REGDIMM	    0

/* calculate according to formula in sec. 6-22 of 8245 UM */
#define CONFIG_SYS_PGMAX           50		/* how long the 8245 retains the */
									/* currently accessed page in memory */
									/* was 45 */

#define CONFIG_SYS_SDRAM_DSCD	0x20	/* SDRAM data in sample clock delay - note */
								/* bits 7,6, and 3-0 MUST be 0 */

#if 0
#define CONFIG_SYS_DLL_MAX_DELAY	0x04
#else
#define CONFIG_SYS_DLL_MAX_DELAY	0
#endif
#if 0							/* need for 33MHz SDRAM */
#define CONFIG_SYS_DLL_EXTEND	0x80
#else
#define CONFIG_SYS_DLL_EXTEND	0
#endif
#define CONFIG_SYS_PCI_HOLD_DEL 0x20


/* Memory bank settings.
 * Only bits 20-29 are actually used from these values to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8245 user manual.
 */

#define CONFIG_SYS_BANK0_START	    0x00000000
#define CONFIG_SYS_BANK0_END	    (CONFIG_SYS_MAX_RAM_SIZE/2 - 1)
#define CONFIG_SYS_BANK0_ENABLE    1
#define CONFIG_SYS_BANK1_START	    CONFIG_SYS_MAX_RAM_SIZE/2
#define CONFIG_SYS_BANK1_END	    (CONFIG_SYS_MAX_RAM_SIZE - 1)
#define CONFIG_SYS_BANK1_ENABLE    1
#define CONFIG_SYS_BANK2_START	    0x3ff00000		/* not available in this design */
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

/*--------------------------------------------------------------------*/
/* 4.4 - Output Driver Control Register */
/*--------------------------------------------------------------------*/
#define CONFIG_SYS_ODCR	    0xe5

/*--------------------------------------------------------------------*/
/* 4.8 - Error Handling Registers */
/*-------------------------------CONFIG_SYS_SDMODE_BURSTLEN-------------------------------------*/
#define CONFIG_SYS_ERRENR1	0x11	/* enable SDRAM refresh overflow error */

/* SDRAM 0-256 MB */
#define CONFIG_SYS_IBAT0L  (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
/*#define CONFIG_SYS_IBAT0L  (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_CACHEINHIBIT) */
#define CONFIG_SYS_IBAT0U  (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* stack in dcache */
#define CONFIG_SYS_IBAT1L  (CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U  (CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)


#define CONFIG_SYS_IBAT2L  (CONFIG_SYS_SDRAM_BASE + 0x10000000 | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT2U  (CONFIG_SYS_SDRAM_BASE + 0x10000000| BATU_BL_256M | BATU_VS | BATU_VP)

/* PCI memory */
/*#define CONFIG_SYS_IBAT2L  (0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT) */
/*#define CONFIG_SYS_IBAT2U  (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP) */

/*Flash, config addrs, etc. */
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

/*-----------------------------------------------------------------------
 * FLASH organization
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE	    0xFF800000
#define CONFIG_SYS_MAX_FLASH_BANKS	1			/* Max number of flash banks */

/*	NOTE: environment is not EMBEDDED in the u-boot code.
	It's stored in flash in its own separate sector.  */
#define CONFIG_ENV_IS_IN_FLASH	    1

#if 1	/* AMD AM29LV033C */
#define CONFIG_SYS_MAX_FLASH_SECT	64		/* Max number of sectors in one bank */
#define CONFIG_ENV_ADDR		0xFFBF0000	/* flash sector SA63 */
#define CONFIG_ENV_SECT_SIZE	(64*1024)	/* Size of the Environment Sector */
#else	/* AMD AM29LV116D */
#define CONFIG_SYS_MAX_FLASH_SECT	35	/* Max number of sectors in one bank */
#define CONFIG_ENV_ADDR		0xFF9FA000	/* flash sector SA33 */
#define CONFIG_ENV_SECT_SIZE	(8*1024)	/* Size of the Environment Sector */
#endif /* #if */

#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE		/* Size of the Environment */
#define CONFIG_ENV_OFFSET		0			/* starting right at the beginning */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Timeout for Flash Write (in ms)	*/

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
#undef CONFIG_SYS_RAMBOOT
#else
#define CONFIG_SYS_RAMBOOT
#endif


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

#endif	/* __CONFIG_H */
