/*
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003 Motorola,Inc.
 * Xianghua Xiao <X.Xiao@motorola.com>
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
 * pm854 board configuration file
 *
 * Please refer to doc/README.mpc85xx for more info.
 *
 * Make sure you change the MAC address and other network params first,
 * search for CONFIG_ETHADDR, CONFIG_SERVERIP, etc in this file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/MPC8560 */
#define CONFIG_MPC8540		1	/* MPC8540 specific */
#define CONFIG_PM854		1	/* PM854 board specific */

#define CONFIG_PCI
#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE
#undef	CONFIG_SPD_EEPROM		/* do not use SPD EEPROM for DDR setup*/
#define CONFIG_DDR_DLL			/* possible DLL fix needed */
#define CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */

#define CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_MEM_INIT_VALUE		0xDEADBEEF


/*
 * sysclk for MPC85xx
 *
 * Two valid values are:
 *    33000000
 *    66000000
 *
 * Most PCI cards are still 33Mhz, so in the presence of PCI, 33MHz
 * is likely the desired value here, so that is now the default.
 * The board, however, can run at 66MHz.  In any event, this value
 * must match the settings of some switches.  Details can be found
 * in the README.mpc85xxads.
 */

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	66000000
#endif


/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */
#define CONFIG_ADDR_STREAMING		/* toggle addr streaming */

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CFG_DRAM_TEST			/* memory test, takes time */
#define CFG_MEMTEST_START	0x00200000	/* memtest region */
#define CFG_MEMTEST_END		0x00400000


/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */


/*
 * DDR Setup
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE

#if defined(CONFIG_SPD_EEPROM)
    /*
     * Determine DDR configuration from I2C interface.
     */
    #define SPD_EEPROM_ADDRESS	0x58		/* DDR DIMM */

#else
    /*
     * Manually set up DDR parameters
     */
    #define CFG_SDRAM_SIZE	256		/* DDR is 256 MB */
    #define CFG_DDR_CS0_BNDS	0x0000000f	/* 0-256MB */
    #define CFG_DDR_CS0_CONFIG	0x80000102
    #define CFG_DDR_TIMING_1	0x47444321
    #define CFG_DDR_TIMING_2	0x00000800	/* P9-45,may need tuning */
    #define CFG_DDR_CONTROL	0xc2008000	/* unbuffered,no DYN_PWR */
    #define CFG_DDR_MODE	0x00000062	/* DLL,normal,seq,4/2.5 */
    #define CFG_DDR_INTERVAL	0x045b0100	/* autocharge,no open page */
#endif


/*
 * SDRAM on the Local Bus
 */
#define CFG_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#define CFG_LBC_SDRAM_SIZE	0		/* LBC SDRAM is 0 MB */

#define CFG_FLASH_BASE		0xfe000000	/* start of 32 MB FLASH */
#define CFG_BR0_PRELIM		0xfe001801	/* port size 32bit */

#define CFG_OR0_PRELIM		0xfe006f67	/* 32 MB Flash */
#define CFG_MAX_FLASH_BANKS	1		/* number of banks */
#define CFG_MAX_FLASH_SECT	128		/* sectors per device */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */


#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef	CFG_RAMBOOT
#endif

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Local Bus Definitions
 */
#define CFG_LBC_LCRR		0x00030004    /* LB clock ratio reg */
#define CFG_LBC_LBCR		0x00000000    /* LB config reg */
#define CFG_LBC_LSRT		0x20000000    /* LB sdram refresh timer */
#define CFG_LBC_MRTPR		0x20000000    /* LB refresh timer prescal*/


#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CFG_INIT_RAM_END	0x4000		/* End of used area in RAM */

#define CFG_GBL_DATA_SIZE	128		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(512 * 1024)	/* Reserve 512 kB for Mon */
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX     1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1	(CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2	(CFG_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000

/*
 * EEPROM configuration
 */
#define CFG_I2C_EEPROM_ADDR		0x58
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	4
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10

/*
 * RTC configuration
 */
#define CONFIG_RTC_PCF8563
#define CFG_I2C_RTC_ADDR		0x51

/* RapidIO MMU */
#define CFG_RIO_MEM_BASE	0xc0000000	/* base address */
#define CFG_RIO_MEM_PHYS	CFG_RIO_MEM_BASE
#define CFG_RIO_MEM_SIZE	0x20000000	/* 128M */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI1_IO_BASE	0xe2000000
#define CFG_PCI1_IO_PHYS	CFG_PCI1_IO_BASE
#define CFG_PCI1_IO_SIZE	0x1000000	/* 16M */

#if defined(CONFIG_PCI)

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#define CONFIG_EEPRO100
#define	CONFIG_E1000
#undef	CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
    #define PCI_ENET0_IOADDR	0xe0000000
    #define PCI_ENET0_MEMADDR	0xe0000000
    #define PCI_IDSEL_NUMBER	0x0c	/* slot0->3(IDSEL)=12->15 */
#endif

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CFG_PCI_SUBSYS_VENDORID 0x1057	/* Motorola */

#endif	/* CONFIG_PCI */


#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0

#define CONFIG_MPC85XX_FEC	1
#define CONFIG_MPC85XX_FEC_NAME		"FEC"
#define FEC_PHY_ADDR		3
#define FEC_PHYIDX		0

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"TSEC0"

#define	CONFIG_HAS_ETH1		1
#define	CONFIG_HAS_ETH2		1

#endif	/* CONFIG_TSEC_ENET */


/*
 * Environment
 */
#ifndef CFG_RAMBOOT
  #define CFG_ENV_IS_IN_FLASH	1
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x80000)
  #define CFG_ENV_SECT_SIZE	0x40000 /* 256K(one sector) for env */
  #define CFG_ENV_SIZE		0x2000
#else
  #define CFG_NO_FLASH		1	/* Flash is not usable now */
  #define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
  #define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EEPROM

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory */
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
    #define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
    #define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */
#define CONFIG_LOOPW

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/* Cache Configuration */
#define CFG_DCACHE_SIZE		32768
#define CFG_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/*log base 2 of the above value*/
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif


/*
 * Environment Configuration
 */

/* The mac addresses for all ethernet interface */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_ETHADDR	 00:40:42:01:00:00
#define CONFIG_ETH1ADDR	 00:40:42:01:00:01
#define CONFIG_ETH2ADDR	 00:40:42:01:00:02
#endif


#define CONFIG_ROOTPATH		/opt/eldk/ppc_85xx
#define CONFIG_BOOTFILE		pm854/uImage

#define CONFIG_HOSTNAME		pm854
#define CONFIG_IPADDR	 192.168.0.103
#define CONFIG_SERVERIP	 192.168.0.64
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_NETMASK	 255.255.255.0

#define CONFIG_LOADADDR	 200000 /* default location for tftp and bootm */

#define CONFIG_BOOTDELAY 5	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE 9600

#define CONFIG_EXTRA_ENV_SETTINGS					\
   "netdev=eth0\0"							\
   "consoledev=ttyS0\0"							\
   "ramdiskaddr=400000\0"						\
   "ramdiskfile=pm854/uRamdisk\0"

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "bootm $loadaddr"

#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "bootm $loadaddr $ramdiskaddr"

#define CONFIG_BOOTCOMMAND  CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
