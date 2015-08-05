/*
 * (C) Copyright 2003 Embedded Edge, LLC
 * Dan Malek <dan@embeddededge.com>
 * Copied from ADS85xx.
 * Updates for Silicon Tx GP3 8560 board.
 *
 * (C) Copyright 2002,2003 Motorola,Inc.
 * Xianghua Xiao <X.Xiao@motorola.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* mpc8560ads board configuration file */
/* please refer to doc/README.mpc85xx for more info */
/* make sure you change the MAC address and other network params first,
 * search for CONFIG_SERVERIP, etc. in this file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE		*/
#define CONFIG_E500		1	/* BOOKE e500 family	*/
#define CONFIG_CPM2		1	/* has CPM2 */
#define CONFIG_STXGP3		1	/* Silicon Tx GPPP board specific*/
#define CONFIG_MPC8560		1

#define	CONFIG_SYS_TEXT_BASE	0xfff80000

#undef  CONFIG_PCI			/* pci ethernet support	*/
#define CONFIG_TSEC_ENET		/* tsec ethernet support*/
#undef  CONFIG_ETHER_ON_FCC             /* cpm FCC ethernet support */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

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

#define CONFIG_BOARD_EARLY_INIT_F   1        /* Call board_pre_init      */
#define CONFIG_RESET_PHY_R	1	/* Call reset_phy()		*/

#undef  CONFIG_SYS_DRAM_TEST                       /* memory test, takes time  */
#define CONFIG_SYS_MEMTEST_START       0x00200000  /* memtest region */
#define CONFIG_SYS_MEMTEST_END         0x00400000


/* Localbus SDRAM is an option, not all boards have it.
 * This address, however, is used to configure a 256M local bus
 * window that includes the Config latch below.
 */
#define CONFIG_SYS_LBC_SDRAM_BASE      0xf0000000      /* Localbus SDRAM */
#define CONFIG_SYS_LBC_SDRAM_SIZE	256		/* LBC SDRAM is 64MB	*/

#define CONFIG_SYS_FLASH_BASE        0xff000000      /* start of FLASH 16M    */
#define CONFIG_SYS_BR0_PRELIM        0xff001801      /* port size 32bit      */

#define CONFIG_SYS_OR0_PRELIM          0xff000ff7      /* 16 MB Flash           */
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	136		/* sectors per device   */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/* The configuration latch is Chip Select 1.
 * It's an 8-bit latch in the lower 8 bits of the word.
 */
#define CONFIG_SYS_BR1_PRELIM		0xfc001801	/* 32-bit port */
#define CONFIG_SYS_OR1_PRELIM		0xffff0ff7      /* 64K is enough */
#define CONFIG_SYS_LBC_LCLDEVS_BASE	0xfc000000	/* Base of localbus devices */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor	*/

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef  CONFIG_SYS_RAMBOOT
#endif

#ifdef CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_CCSRBAR_DEFAULT	0x40000000	/* CCSRBAR by BDI cfg	*/
#endif
#define CONFIG_SYS_CCSRBAR		0xfdf00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_SYS_FSL_DDR1
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/
#define CONFIG_DDR_SPD
#undef CONFIG_FSL_DDR_INTERACTIVE

#undef  CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_SYS_FSL_ERRATUM_DDR_MSYNC_IN	/* possible DLL fix needed */
#define CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */

#define CONFIG_MEM_INIT_VALUE		0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x54	/* CTLR 0 DIMM 0 */

#undef CONFIG_CLOCKS_IN_MHZ

/* local bus definitions */
#define CONFIG_SYS_BR2_PRELIM		0xf8001861	/* 64MB localbus SDRAM  */
#define CONFIG_SYS_OR2_PRELIM		0xfc006901
#define CONFIG_SYS_LBC_LCRR		0x00030004	/* local bus freq	*/
#define CONFIG_SYS_LBC_LBCR		0x00000000
#define CONFIG_SYS_LBC_LSRT		0x20000000
#define CONFIG_SYS_LBC_MRTPR		0x20000000
#define CONFIG_SYS_LBC_LSDMR_1		0x2861b723
#define CONFIG_SYS_LBC_LSDMR_2		0x0861b723
#define CONFIG_SYS_LBC_LSDMR_3		0x0861b723
#define CONFIG_SYS_LBC_LSDMR_4		0x1861b723
#define CONFIG_SYS_LBC_LSDMR_5		0x4061b723

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR       0x60000000      /* Initial RAM address  */
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)    /* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)    /* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef  CONFIG_CONS_NONE		/* define if console on something else */
#define CONFIG_CONS_INDEX       2	/* which serial channel for console */

#define CONFIG_BAUDRATE		38400

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef  CONFIG_SYS_HUSH_PARSER
#endif

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000

#if 0
#define CONFIG_SYS_I2C_NOPROBES        {0x00}  /* Don't probe these addrs */
#else
/* I did the 'if 0' so we could keep the syntax above if ever needed. */
#undef CONFIG_SYS_I2C_NOPROBES
#endif

/* RapdIO Map configuration, mapped 1:1.
*/
#define CONFIG_SYS_RIO_MEM_BASE	0xc0000000
#define CONFIG_SYS_RIO_MEM_PHYS	CONFIG_SYS_RIO_MEM_BASE
#define CONFIG_SYS_RIO_MEM_SIZE	0x200000000	/* 512 M */

/* Standard 8560 PCI addressing, mapped 1:1.
*/
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_BASE	0xe2000000
#define CONFIG_SYS_PCI1_IO_PHYS	CONFIG_SYS_PCI1_IO_BASE
#define CONFIG_SYS_PCI1_IO_SIZE	0x01000000	/* 16 M */

#if defined(CONFIG_PCI)			/* PCI Ethernet card */

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
  #define PCI_ENET0_IOADDR	0xe0000000
  #define PCI_ENET0_MEMADDR     0xe0000000
  #define PCI_IDSEL_NUMBER      0x0c	/* slot0->3(IDSEL)=12->15 */
#endif

#undef CONFIG_PCI_SCAN_SHOW
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */

#endif /* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)

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
  #define CONFIG_SYS_CMXFCR_MASK2      (CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
  #define CONFIG_SYS_CMXFCR_VALUE2     (CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)
  #define CONFIG_SYS_CPMFCR_RAMTYPE    0
#if 0
  #define CONFIG_SYS_FCC_PSMR          (FCC_PSMR_FDE)
#else
  #define CONFIG_SYS_FCC_PSMR          0
#endif
  #define FETH2_RST		0x01
#elif (CONFIG_ETHER_INDEX == 3)
  /* need more definitions here for FE3 */
  #define FETH3_RST		0x80
#endif	/* CONFIG_ETHER_INDEX */

/* MDIO is done through the TSEC0 control.
*/
#define CONFIG_MII			/* MII PHY management */
#undef CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/

#endif

/* Environment */
/* We use the top boot sector flash, so we have some 16K sectors for env
 */
#ifndef CONFIG_SYS_RAMBOOT
  #define CONFIG_ENV_IS_IN_FLASH	1
  #define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x60000)
  #define CONFIG_ENV_SECT_SIZE	0x4000	/* 16K (one top sector) for env */
  #define CONFIG_ENV_SIZE		0x2000
#else
  #define CONFIG_SYS_NO_FLASH		1	/* Flash is not usable now	*/
  #define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only	*/
  #define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
  #define CONFIG_ENV_SIZE		0x2000
#endif

#define CONFIG_BOOTARGS "root=/dev/nfs rw ip=any console=ttyS1,38400"
#define CONFIG_BOOTCOMMAND	"bootm 0xff000000 0xff100000"
#define CONFIG_BOOTDELAY	3	/* -1 disable autoboot */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

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
#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_REGINFO

#if !defined(CONFIG_SYS_RAMBOOT)
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
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"GPPP=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR	0x1000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#endif

#define CONFIG_SERVERIP		192.168.85.1
#define CONFIG_IPADDR		192.168.85.60
#define CONFIG_GATEWAYIP	192.168.85.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_HOSTNAME		STX_GP3
#define CONFIG_ROOTPATH		"/gppproot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_LOADADDR		0x1000000

#endif	/* __CONFIG_H */
