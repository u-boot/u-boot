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
#define CONFIG_MPC85xx_REV1	1	/* MPC85xx Rev 1.0 chip */
#define CONFIG_MPC8560		1	/* MPC8560 specific	*/
#define CONFIG_STXGP3		1	/* Silicon Tx GPPP board specific*/

#undef  CONFIG_PCI	         	/* pci ethernet support	*/
#define CONFIG_TSEC_ENET 		/* tsec ethernet support*/
#undef  CONFIG_ETHER_ON_FCC             /* cpm FCC ethernet support */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SPD_EEPROM               /* Use SPD EEPROM for DDR setup */
#undef  CONFIG_DDR_ECC			/* only for ECC DDR module */

#if defined(CONFIG_MPC85xx_REV1)
#define CONFIG_DDR_DLL                  /* possible DLL fix needed */
#endif

/* Using Localbus SDRAM to emulate flash before we can program the flash,
 * normally you need a flash-boot image(u-boot.bin), if so undef this.
 */
#undef CONFIG_RAM_AS_FLASH

#define CONFIG_SYS_CLK_FREQ     33333333 /* most pci cards are 33Mhz */

/* Blinkin' LEDs for Robert :-)
*/
#define CONFIG_SHOW_ACTIVITY 1

#if !defined(CONFIG_SPD_EEPROM)		/* manually set up DDR parameters */
#define CONFIG_DDR_SETTING
#endif

/* below can be toggled for performance analysis. otherwise use default */
#define CONFIG_L2_CACHE                     /* toggle L2 cache         */
#undef  CONFIG_BTB                          /* toggle branch predition */
#undef  CONFIG_ADDR_STREAMING               /* toggle addr streaming   */

#define CONFIG_BOARD_PRE_INIT   1           /* Call board_pre_init      */

#undef  CFG_DRAM_TEST                       /* memory test, takes time  */
#define CFG_MEMTEST_START       0x00200000  /* memtest region */
#define CFG_MEMTEST_END         0x00400000

#if (defined(CONFIG_PCI) && defined(CONFIG_TSEC_ENET) || \
     defined(CONFIG_PCI) && defined(CONFIG_ETHER_ON_FCC) || \
     defined(CONFIG_TSEC_ENET) && defined(CONFIG_ETHER_ON_FCC))
#error "You can only use ONE of PCI Ethernet Card or TSEC Ethernet or CPM FCC."
#endif

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory  */
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE

/* GPPP supports up to 2G of DRAM.  Allocate up to 1G until we get
 * a chance to try it out.  Actual size is always read from sdram eeprom.
 */
#define CFG_SDRAM_SIZE		1024		/* DDR is 1GB	*/

/* Localbus SDRAM is an option, not all boards have it.
*/
#if defined(CONFIG_RAM_AS_FLASH)
#define CFG_LBC_SDRAM_BASE      0xfc000000      /* Localbus SDRAM */
#else
#define CFG_LBC_SDRAM_BASE      0xf8000000      /* Localbus SDRAM */
#endif
#define CFG_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB	*/

#if defined(CONFIG_RAM_AS_FLASH)
#define CFG_FLASH_BASE        0xf8000000      /* start of FLASH  16M  */
#define CFG_BR0_PRELIM        0xf8001801      /* port size 32bit */
#else /* Boot from real Flash */
#define CFG_FLASH_BASE        0xff000000      /* start of FLASH 16M    */
#define CFG_BR0_PRELIM        0xff001801      /* port size 32bit      */
#endif

#define CFG_OR0_PRELIM          0xff000ff7      /* 16 MB Flash           */
#define CFG_MAX_FLASH_BANKS	1		/* number of banks	*/
#define CFG_MAX_FLASH_SECT	136		/* sectors per device   */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/* The configuration latch is Chip Select 1.
 * It's an 8-bit latch in the upper 8 bits of the word.
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


#define SPD_EEPROM_ADDRESS 	0x54     	/*  DDR DIMM */

#if defined(CONFIG_DDR_SETTING)
#define	CFG_DDR_CS0_BNDS	0x00000007	/* 0-128MB */
#define CFG_DDR_CS0_CONFIG	0x80000002
#define CFG_DDR_TIMING_1	0x37344321
#define CFG_DDR_TIMING_2	0x00000800	/* P9-45,may need tuning*/
#define CFG_DDR_CONTROL		0xc2000000	/* unbuffered,no DYN_PWR*/
#define CFG_DDR_MODE		0x00000062	/* DLL,normal,seq,4/2.5 */
#define CFG_DDR_INTERVAL	0x05200100	/* autocharge,no open page*/
#endif

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

/* I2C */
#define  CONFIG_HARD_I2C    		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#if 0
#define CFG_I2C_NOPROBES        {0x00}  /* Don't probe these addrs */
#else
/* I did the 'if 0' so we could keep the syntax above if ever needed. */
#undef CFG_I2C_NOPROBES
#endif

#define CFG_PCI_MEM_BASE	0xe0000000
#define CFG_PCI_MEM_PHYS	0xe0000000
#define CFG_PCI_MEM_SIZE	0x10000000

#if defined(CONFIG_PCI) 		/* PCI Ethernet card */
#define CONFIG_NET_MULTI
#define CONFIG_EEPRO100
#undef CONFIG_TULIP
#define CONFIG_PCI_PNP	               	/* do pci plug-and-play */
  #if !defined(CONFIG_PCI_PNP)
  #define PCI_ENET0_IOADDR    	0xe0000000
  #define PCI_ENET0_MEMADDR     0xe0000000
  #define PCI_IDSEL_NUMBER      0x0c 	/* slot0->3(IDSEL)=12->15 */
  #endif
#define CONFIG_PCI_SCAN_SHOW    1       /* show pci devices on startup  */
#define CFG_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */
#if defined(CONFIG_MPC85xx_REV1) 	/* Errata PCI 7 */
  #define CFG_PCI_SUBSYS_DEVICEID 0x0003
#else
  #define CFG_PCI_SUBSYS_DEVICEID 0x0009
#endif
#elif defined(CONFIG_TSEC_ENET) 	/* TSEC Ethernet port */
#define CONFIG_NET_MULTI 	1
#define CONFIG_PHY_M88E1011      1       /* GigaBit Ether PHY        */
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		8	/* PHY address			*/
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
#define CONFIG_MII			/* MII PHY management */
#undef CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT	2		/* Port C */
#define MDIO_ACTIVE	(iop->pdir |=  0x00400000)
#define MDIO_TRISTATE	(iop->pdir &= ~0x00400000)
#define MDIO_READ	((iop->pdat &  0x00400000) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  0x00400000; \
			else	iop->pdat &= ~0x00400000

#define MDC(bit)	if(bit) iop->pdat |=  0x00200000; \
			else	iop->pdat &= ~0x00200000

#define MIIDELAY	udelay(1)
#endif

/* Environment */
/* We use the top boot sector flash, so we have some 16K sectors for env
 * But....functions don't seem smart enough yet.
 */
#ifndef CFG_RAMBOOT
  #if defined(CONFIG_RAM_AS_FLASH)
  #define CFG_ENV_IS_NOWHERE
  #define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x100000)
  #define CFG_ENV_SIZE		0x2000
  #else
  #define CFG_ENV_IS_IN_FLASH	1
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x60000)
  #define CFG_ENV_SECT_SIZE	0x4000	/* 16K (one top sector) for env */
  #endif
  #define CFG_ENV_SIZE		0x2000
#else
#define CFG_NO_FLASH		1	/* Flash is not usable now	*/
#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
#define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_BOOTARGS "root=/dev/nfs rw ip=any console=ttyS1,38400"
#define CONFIG_BOOTCOMMAND	"bootm 0xff800000 0xff900000"
#define CONFIG_BOOTDELAY	3	/* -1 disable autoboot */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#if defined(CFG_RAMBOOT) || defined(CONFIG_RAM_AS_FLASH)
  #if defined(CONFIG_PCI)
  #define  CONFIG_COMMANDS	((CONFIG_CMD_DFL | CFG_CMD_PCI | \
				CFG_CMD_PING | CFG_CMD_I2C) & \
				 ~(CFG_CMD_ENV | \
				  CFG_CMD_LOADS ))
  #elif defined(CONFIG_TSEC_ENET)
  #define  CONFIG_COMMANDS	((CONFIG_CMD_DFL | CFG_CMD_PING | \
				CFG_CMD_MII | CFG_CMD_I2C ) & \
				~(CFG_CMD_ENV))
  #elif defined(CONFIG_ETHER_ON_FCC)
  #define  CONFIG_COMMANDS	((CONFIG_CMD_DFL | CFG_CMD_MII | \
				CFG_CMD_PING | CFG_CMD_I2C) & \
				~(CFG_CMD_ENV))
  #endif
#else
  #if defined(CONFIG_PCI)
  #define  CONFIG_COMMANDS	(CONFIG_CMD_DFL | CFG_CMD_PCI | \
				CFG_CMD_PING | CFG_CMD_I2C)
  #elif defined(CONFIG_TSEC_ENET)
  #define  CONFIG_COMMANDS	(CONFIG_CMD_DFL | CFG_CMD_PING | \
				CFG_CMD_MII | CFG_CMD_I2C)
  #elif defined(CONFIG_ETHER_ON_FCC)
  #define  CONFIG_COMMANDS	(CONFIG_CMD_DFL | CFG_CMD_MII | \
				CFG_CMD_PING | CFG_CMD_I2C)
  #endif
#endif
#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"GPPP=> "	/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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

/* Cache Configuration */
#define CFG_DCACHE_SIZE		32768
#define CFG_CACHELINE_SIZE	32
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot		*/

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*Note: change below for your network setting!!! */
#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
#define CONFIG_ETHADDR  00:01:af:07:9b:8a
#define CONFIG_ETH1ADDR  00:01:af:07:9b:8b
#define CONFIG_ETH2ADDR  00:01:af:07:9b:8c
#endif

#define CONFIG_SERVERIP 	192.168.85.1
#define CONFIG_IPADDR  		192.168.85.60
#define CONFIG_GATEWAYIP	192.168.85.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_HOSTNAME 	STX_GP3
#define CONFIG_ROOTPATH 	/gppproot
#define CONFIG_BOOTFILE 	uImage

#endif	/* __CONFIG_H */
