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
 * MicroSys PM856 board configuration file
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
#define CONFIG_MPC8560		1	/* MPC8560 specific */
#define CONFIG_CPM2		1	/* Has a CPM2 */
#define CONFIG_PM856		1	/* PM856 board specific */

#define CONFIG_PCI
#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

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

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#define CONFIG_SYS_INIT_DBCR DBCR_IDM		/* Enable Debug Exceptions */

#undef	CONFIG_SYS_DRAM_TEST			/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000


/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CONFIG_SYS_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CONFIG_SYS_CCSRBAR_PHYS	CONFIG_SYS_CCSRBAR	/* physical addr of CCSRBAR */
#define CONFIG_SYS_IMMR		CONFIG_SYS_CCSRBAR	/* PQII uses CONFIG_SYS_IMMR */

/* DDR Setup */
#define CONFIG_FSL_DDR1
#undef CONFIG_FSL_DDR_INTERACTIVE
#undef CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#undef CONFIG_DDR_SPD
#define CONFIG_DDR_DLL                      /* possible DLL fix needed */
#define CONFIG_DDR_ECC			    /* only for ECC DDR module */
#define CONFIG_FSL_DMA			    /* use DMA to init DDR ECC  */

#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x58	/* CTLR 0 DIMM 0 */

/* Manually set up DDR parameters */
#define CONFIG_SYS_SDRAM_SIZE	256		/* DDR is 256 MB */
#define CONFIG_SYS_DDR_CS0_BNDS	0x0000000f	/* 0-256MB */
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80000102
#define CONFIG_SYS_DDR_TIMING_1	0x47444321
#define CONFIG_SYS_DDR_TIMING_2	0x00000800	/* P9-45,may need tuning */
#define CONFIG_SYS_DDR_CONTROL	0xc2008000	/* unbuffered,no DYN_PWR */
#define CONFIG_SYS_DDR_MODE	0x00000062	/* DLL,normal,seq,4/2.5 */
#define CONFIG_SYS_DDR_INTERVAL	0x045b0100	/* autocharge,no open page */

/*
 * SDRAM on the Local Bus
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#define CONFIG_SYS_LBC_SDRAM_SIZE	0		/* LBC SDRAM is 0 MB */

#define CONFIG_SYS_FLASH_BASE		0xfe000000	/* start of FLASH 32M */
#define CONFIG_SYS_BR0_PRELIM		0xfe001801	/* port size 32bit */

#define CONFIG_SYS_OR0_PRELIM		0xfe006f67	/* 32MB Flash */
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	128		/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef  CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO

#undef CONFIG_CLOCKS_IN_MHZ


/*
 * Local Bus Definitions
 */

#define CONFIG_SYS_LBC_LCRR		0x00030004    /* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR		0x00000000    /* LB config reg */
#define CONFIG_SYS_LBC_LSRT		0x20000000    /* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR		0x20000000    /* LB refresh timer prescal*/


#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_END	0x4000		/* End of used area in RAM */

#define CONFIG_SYS_GBL_DATA_SIZE	128		/* num bytes initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)    /* Reserve 512 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)    /* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_ON_SCC	/* define if console on SCC */
#undef  CONFIG_CONS_NONE	/* define if console on something else */
#define CONFIG_CONS_INDEX       1  /* which serial channel for console */

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef  CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_NOPROBES        {0x69}	/* Don't probe these addrs */
#define CONFIG_SYS_I2C_OFFSET		0x3000

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x58
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/*
 * RTC configuration
 */
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR		0x51

/* RapidIO MMU */
#define CONFIG_SYS_RIO_MEM_BASE	0xc0000000	/* base address */
#define CONFIG_SYS_RIO_MEM_PHYS	CONFIG_SYS_RIO_MEM_BASE
#define CONFIG_SYS_RIO_MEM_SIZE	0x20000000	/* 128M */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_BASE	0xe2000000
#define CONFIG_SYS_PCI1_IO_PHYS	CONFIG_SYS_PCI1_IO_BASE
#define CONFIG_SYS_PCI1_IO_SIZE	0x1000000	/* 16M */

#if defined(CONFIG_PCI)

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
    #define PCI_ENET0_IOADDR	0xe0000000
    #define PCI_ENET0_MEMADDR	0xe0000000
    #define PCI_IDSEL_NUMBER	0x0c	/* slot0->3(IDSEL)=12->15 */
#endif

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */

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
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

#endif  /* CONFIG_TSEC_ENET */

#define CONFIG_ETHPRIME		"TSEC0"

#define CONFIG_ETHER_ON_FCC	/* define if ether on FCC   */
#undef  CONFIG_ETHER_NONE	/* define if ether on something else */


/*
   * - Rx-CLK is CLK15
   * - Tx-CLK is CLK14
   * - Select bus for bd/buffers
   * - Full duplex
 */
#define CONFIG_ETHER_ON_FCC3
#define CONFIG_SYS_CMXFCR_MASK3	(CMXFCR_FC3 | CMXFCR_RF3CS_MSK | CMXFCR_TF3CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE3	(CMXFCR_RF3CS_CLK15 | CMXFCR_TF3CS_CLK14)
#define CONFIG_SYS_CPMFCR_RAMTYPE	0
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE)

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
  #define CONFIG_ENV_IS_IN_FLASH	1
  #define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x80000)
  #define CONFIG_ENV_SECT_SIZE	0x40000	/* 256K(one sector) for env */
  #define CONFIG_ENV_SIZE		0x2000
#else
  #define CONFIG_SYS_NO_FLASH		1	/* Flash is not usable now */
  #define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
  #define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
  #define CONFIG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */


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

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EEPROM

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_SYS_RAMBOOT)
    #undef CONFIG_CMD_SAVEENV
    #undef CONFIG_CMD_LOADS
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_SYS_LOAD_ADDR	0x1000000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
    #define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
    #define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif

#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */
#define CONFIG_LOOPW

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

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
#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR   00:40:42:01:00:00
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR  00:40:42:01:00:01
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR  00:40:42:01:00:02
#endif


#define CONFIG_ROOTPATH		/opt/eldk/ppc_85xx
#define CONFIG_BOOTFILE		pm856/uImage

#define CONFIG_HOSTNAME		pm856
#define CONFIG_IPADDR    192.168.0.103
#define CONFIG_SERVERIP  192.168.0.64
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_NETMASK   255.255.255.0

#define CONFIG_LOADADDR  200000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY 5	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE	9600

#define	CONFIG_EXTRA_ENV_SETTINGS				        \
   "netdev=eth0\0"                                                      \
   "consoledev=ttyS0\0"                                                 \
   "ramdiskaddr=400000\0"						\
   "ramdiskfile=pm856/uRamdisk\0"

#define CONFIG_NFSBOOTCOMMAND	                                        \
   "setenv bootargs root=/dev/nfs rw "                                  \
      "nfsroot=$serverip:$rootpath "                                    \
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"                     \
   "tftp $loadaddr $bootfile;"                                          \
   "bootm $loadaddr"

#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "                                  \
      "console=$consoledev,$baudrate $othbootargs;"                     \
   "tftp $ramdiskaddr $ramdiskfile;"                                    \
   "tftp $loadaddr $bootfile;"                                          \
   "bootm $loadaddr $ramdiskaddr"

#define CONFIG_BOOTCOMMAND  CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
