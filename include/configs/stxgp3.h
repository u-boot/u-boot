/*
 * (C) Copyright 2003 Embedded Edge, LLC
 * Dan Malek <dan@embeddededge.com>
 * Copied from ADS85xx.
 * Updates for Silicon Tx GP3 8560 board.
 *
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

/* mpc8560ads board configuration file */
/* please refer to doc/README.mpc85xx for more info */
/* make sure you change the MAC address and other network params first,
 * search for CONFIG_ETHADDR,CONFIG_SERVERIP,etc in this file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE		*/
#define CONFIG_E500		1	/* BOOKE e500 family	*/
#define CONFIG_MPC85xx		1	/* MPC8540/MPC8560	*/
#define CONFIG_CPM2		1	/* has CPM2 */
#define CONFIG_STXGP3		1	/* Silicon Tx GPPP board specific*/

#undef  CONFIG_PCI	         	/* pci ethernet support	*/
#define CONFIG_TSEC_ENET 		/* tsec ethernet support*/
#undef  CONFIG_ETHER_ON_FCC             /* cpm FCC ethernet support */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SPD_EEPROM               /* Use SPD EEPROM for DDR setup */
#undef  CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_DDR_DLL                  /* possible DLL fix needed */
#define CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */
#define CONFIG_FSL_INIT_TLBS	1	/* Use common FSL init code */

/* sysclk for MPC85xx
 */

#define CONFIG_SYS_CLK_FREQ     33333333 /* most pci cards are 33Mhz */

/* Blinkin' LEDs for Robert :-)
*/
#define CONFIG_SHOW_ACTIVITY 1

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE                     /* toggle L2 cache         */
#define  CONFIG_BTB                          /* toggle branch predition */
#define  CONFIG_ADDR_STREAMING               /* toggle addr streaming   */

#define CONFIG_BOARD_EARLY_INIT_F   1        /* Call board_pre_init      */

#undef  CFG_DRAM_TEST                       /* memory test, takes time  */
#define CFG_MEMTEST_START       0x00200000  /* memtest region */
#define CFG_MEMTEST_END         0x00400000


/* Localbus SDRAM is an option, not all boards have it.
 * This address, however, is used to configure a 256M local bus
 * window that includes the Config latch below.
 */
#define CFG_LBC_SDRAM_BASE      0xf0000000      /* Localbus SDRAM */
#define CFG_LBC_SDRAM_SIZE	256		/* LBC SDRAM is 64MB	*/

#define CFG_FLASH_BASE        0xff000000      /* start of FLASH 16M    */
#define CFG_BR0_PRELIM        0xff001801      /* port size 32bit      */

#define CFG_OR0_PRELIM          0xff000ff7      /* 16 MB Flash           */
#define CFG_MAX_FLASH_BANKS	1		/* number of banks	*/
#define CFG_MAX_FLASH_SECT	136		/* sectors per device   */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/* The configuration latch is Chip Select 1.
 * It's an 8-bit latch in the lower 8 bits of the word.
 */
#define CFG_BR1_PRELIM		0xfc001801	/* 32-bit port */
#define CFG_OR1_PRELIM		0xffff0ff7      /* 64K is enough */
#define CFG_LBC_LCLDEVS_BASE	0xfc000000	/* Base of localbus devices */

#define CFG_MONITOR_BASE    	TEXT_BASE	/* start of monitor	*/

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef  CFG_RAMBOOT
#endif

#ifdef CFG_RAMBOOT
#define CFG_CCSRBAR_DEFAULT 	0x40000000	/* CCSRBAR by BDI cfg	*/
#else
#define CFG_CCSRBAR_DEFAULT 	0xff700000	/* CCSRBAR Default	*/
#endif
#define CFG_CCSRBAR             0xfdf00000      /* relocated CCSRBAR    */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR	*/


/*
 * DDR Setup
 */

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory  */
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE

#define SPD_EEPROM_ADDRESS 	0x54     	/*  DDR DIMM */

#undef CONFIG_CLOCKS_IN_MHZ

/* local bus definitions */
#define CFG_BR2_PRELIM		0xf8001861	/* 64MB localbus SDRAM  */
#define CFG_OR2_PRELIM		0xfc006901
#define CFG_LBC_LCRR		0x00030004	/* local bus freq 	*/
#define CFG_LBC_LBCR		0x00000000
#define CFG_LBC_LSRT		0x20000000
#define CFG_LBC_MRTPR		0x20000000
#define CFG_LBC_LSDMR_1		0x2861b723
#define CFG_LBC_LSDMR_2		0x0861b723
#define CFG_LBC_LSDMR_3		0x0861b723
#define CFG_LBC_LSDMR_4		0x1861b723
#define CFG_LBC_LSDMR_5		0x4061b723

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK 	1
#define CFG_INIT_RAM_ADDR       0x60000000      /* Initial RAM address  */
#define CFG_INIT_RAM_END    	0x4000	    	/* End of used area in RAM */

#define CFG_GBL_DATA_SIZE  	128		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN	    	(256 * 1024)    /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN	    	(128 * 1024)    /* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_ON_SCC              	/* define if console on SCC */
#undef  CONFIG_CONS_NONE                	/* define if console on something else */
#define CONFIG_CONS_INDEX       2       	/* which serial channel for console */

#define CONFIG_BAUDRATE	 	38400

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#if 0
#define CFG_I2C_NOPROBES        {0x00}  /* Don't probe these addrs */
#else
/* I did the 'if 0' so we could keep the syntax above if ever needed. */
#undef CFG_I2C_NOPROBES
#endif
#define CFG_I2C_OFFSET		0x3000

/* RapdIO Map configuration, mapped 1:1.
*/
#define CFG_RIO_MEM_BASE	0xc0000000
#define CFG_RIO_MEM_PHYS	CFG_RIO_MEM_BASE
#define CFG_RIO_MEM_SIZE	0x200000000	/* 512 M */

/* Standard 8560 PCI addressing, mapped 1:1.
*/
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI1_IO_BASE	0xe2000000
#define CFG_PCI1_IO_PHYS	CFG_PCI1_IO_BASE
#define CFG_PCI1_IO_SIZE	0x01000000	/* 16 M */

#if defined(CONFIG_PCI) 		/* PCI Ethernet card */

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP	               	/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
  #define PCI_ENET0_IOADDR    	0xe0000000
  #define PCI_ENET0_MEMADDR     0xe0000000
  #define PCI_IDSEL_NUMBER      0x0c 	/* slot0->3(IDSEL)=12->15 */
#endif

#undef CONFIG_PCI_SCAN_SHOW
#define CFG_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */

#endif /* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI 	1
#endif

#define CONFIG_MII		1	/* MII PHY management		*/

#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"TSEC1"

#define TSEC1_PHY_ADDR		2
#define TSEC2_PHY_ADDR		4
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define CONFIG_ETHPRIME		"TSEC0"

#elif defined(CONFIG_ETHER_ON_FCC)	/* CPM FCC Ethernet */

#define CONFIG_ETHER_ON_FCC2             /* define if ether on FCC   */
#undef  CONFIG_ETHER_NONE               /* define if ether on something else */
#define CONFIG_ETHER_INDEX      2       /* which channel for ether  */

#if (CONFIG_ETHER_INDEX == 2)
  /*
   * - Rx-CLK is CLK13
   * - Tx-CLK is CLK14
   * - Select bus for bd/buffers
   * - Full duplex
   */
  #define CFG_CMXFCR_MASK       (CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
  #define CFG_CMXFCR_VALUE      (CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)
  #define CFG_CPMFCR_RAMTYPE    0
#if 0
  #define CFG_FCC_PSMR          (FCC_PSMR_FDE)
#else
  #define CFG_FCC_PSMR          0
#endif
  #define FETH2_RST		0x01
#elif (CONFIG_ETHER_INDEX == 3)
  /* need more definitions here for FE3 */
  #define FETH3_RST		0x80
#endif  				/* CONFIG_ETHER_INDEX */

/* MDIO is done through the TSEC0 control.
*/
#define CONFIG_MII			/* MII PHY management */
#undef CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/

#endif

/* Environment */
/* We use the top boot sector flash, so we have some 16K sectors for env
 */
#ifndef CFG_RAMBOOT
  #define CFG_ENV_IS_IN_FLASH	1
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x60000)
  #define CFG_ENV_SECT_SIZE	0x4000	/* 16K (one top sector) for env */
  #define CFG_ENV_SIZE		0x2000
#else
  #define CFG_NO_FLASH		1	/* Flash is not usable now	*/
  #define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only	*/
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
  #define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_BOOTARGS "root=/dev/nfs rw ip=any console=ttyS1,38400"
#define CONFIG_BOOTCOMMAND	"bootm 0xff000000 0xff100000"
#define CONFIG_BOOTDELAY	3	/* -1 disable autoboot */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

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

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#else
    #define CONFIG_CMD_ELF
#endif

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
    #define CONFIG_CMD_MII
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"GPPP=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_LOAD_ADDR	0x1000000	/* default load address */
#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot		*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*Note: change below for your network setting!!! */
#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR	 00:e0:0c:07:9b:8a
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR  00:e0:0c:07:9b:8b
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR  00:e0:0c:07:9b:8c
#endif

#define CONFIG_SERVERIP 	192.168.85.1
#define CONFIG_IPADDR  		192.168.85.60
#define CONFIG_GATEWAYIP	192.168.85.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_HOSTNAME 	STX_GP3
#define CONFIG_ROOTPATH 	/gppproot
#define CONFIG_BOOTFILE 	uImage
#define CONFIG_LOADADDR		0x1000000

#endif	/* __CONFIG_H */
