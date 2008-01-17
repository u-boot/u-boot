/*
 * (C) Copyright 2002,2003 Motorola,Inc.
 * Xianghua Xiao <X.Xiao@motorola.com>
 *
 * (C) Copyright 2004 Wind River Systems Inc <www.windriver.com>.
 * Added support for Wind River SBC8540 board
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

#if XXX
#define DEBUG		      /* General debug */
#define ET_DEBUG
#endif
#define TSEC_DEBUG

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE			*/
#define CONFIG_E500		1	/* BOOKE e500 family		*/
#define CONFIG_MPC85xx		1	/* MPC8540/MPC8560		*/
#define CONFIG_MPC85xx_REV1	1	/* MPC85xx Rev 1.0 chip		*/


#define CONFIG_CPM2		1	/* has CPM2 */

#define CONFIG_SBC8540      	1   	/* configuration for SBC8560 board */

#define CONFIG_MPC8560ADS	1	/* MPC8560ADS board specific (supplement)	*/

#define CONFIG_TSEC_ENET		/* tsec ethernet support	*/
#undef	CONFIG_PCI			/* pci ethernet support		*/
#undef  CONFIG_ETHER_ON_FCC		/* cpm FCC ethernet support	*/

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */
#define CONFIG_FSL_INIT_TLBS	1	/* Use common FSL init code */

#define CONFIG_ENV_OVERWRITE

/* Using Localbus SDRAM to emulate flash before we can program the flash,
 * normally you need a flash-boot image(u-boot.bin), if so undef this.
 */
#undef CONFIG_RAM_AS_FLASH

#if defined(CONFIG_PCI_66)			/* some PCI card is 33Mhz only	*/
  #define CONFIG_SYS_CLK_FREQ	66000000	/* sysclk for MPC85xx		*/
#else
  #define CONFIG_SYS_CLK_FREQ	33000000	/* most pci cards are 33Mhz	*/
#endif

/* below can be toggled for performance analysis. otherwise use default */
#define CONFIG_L2_CACHE			    /* toggle L2 cache		*/
#undef	CONFIG_BTB			    /* toggle branch predition	*/
#undef	CONFIG_ADDR_STREAMING		    /* toggle addr streaming	*/

#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_early_init_f	*/

#undef	CFG_DRAM_TEST			    /* memory test, takes time	*/
#define CFG_MEMTEST_START	0x00200000  /* memtest region */
#define CFG_MEMTEST_END		0x00400000

#if (defined(CONFIG_PCI) && defined(CONFIG_TSEC_ENET) || \
     defined(CONFIG_PCI) && defined(CONFIG_ETHER_ON_FCC) || \
     defined(CONFIG_TSEC_ENET) && defined(CONFIG_ETHER_ON_FCC))
#error "You can only use ONE of PCI Ethernet Card or TSEC Ethernet or CPM FCC."
#endif

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default	*/

#if XXX
  #define CFG_CCSRBAR		0xfdf00000	/* relocated CCSRBAR	*/
#else
  #define CFG_CCSRBAR		0xff700000	/* default CCSRBAR	*/
#endif
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR	*/

#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory	 */
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE
#define CFG_SDRAM_SIZE		512		/* DDR is 512MB */
#define SPD_EEPROM_ADDRESS	0x55		/*  DDR DIMM */

#undef  CONFIG_DDR_ECC				/* only for ECC DDR module	*/
#undef  CONFIG_SPD_EEPROM			/* Use SPD EEPROM for DDR setup */

#if defined(CONFIG_MPC85xx_REV1)
  #define CONFIG_DDR_DLL			/* possible DLL fix needed	*/
#endif

#undef CONFIG_CLOCKS_IN_MHZ

#if defined(CONFIG_RAM_AS_FLASH)
  #define CFG_LBC_SDRAM_BASE	0xfc000000	/* Localbus SDRAM */
  #define CFG_FLASH_BASE	0xf8000000      /* start of FLASH 8M  */
  #define CFG_BR0_PRELIM	0xf8000801      /* port size 8bit */
  #define CFG_OR0_PRELIM	0xf8000ff7	/* 8MB Flash		*/
#else /* Boot from real Flash */
  #define CFG_LBC_SDRAM_BASE	0xf8000000	/* Localbus SDRAM */
  #define CFG_FLASH_BASE	0xff800000      /* start of FLASH 8M    */
  #define CFG_BR0_PRELIM	0xff800801      /* port size 8bit      */
  #define CFG_OR0_PRELIM	0xff800ff7	/* 8MB Flash		*/
#endif
#define CFG_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB	*/

/* local bus definitions */
#define CFG_BR1_PRELIM		0xe4001801	/* 64M, 32-bit flash */
#define CFG_OR1_PRELIM		0xfc000ff7

#define CFG_BR2_PRELIM		0x00000000	/* CS2 not used */
#define CFG_OR2_PRELIM		0x00000000

#define CFG_BR3_PRELIM		0xf0001861	/* 64MB localbus SDRAM	*/
#define CFG_OR3_PRELIM		0xfc000cc1

#if defined(CONFIG_RAM_AS_FLASH)
  #define CFG_BR4_PRELIM	0xf4001861	/* 64M localbus SDRAM */
#else
  #define CFG_BR4_PRELIM	0xf8001861	/* 64M localbus SDRAM */
#endif
#define CFG_OR4_PRELIM		0xfc000cc1

#define CFG_BR5_PRELIM		0xfc000801	/* 16M CS5 misc devices */
#if 1
  #define CFG_OR5_PRELIM	0xff000ff7
#else
  #define CFG_OR5_PRELIM	0xff0000f0
#endif

#define CFG_BR6_PRELIM		0xe0001801	/* 64M, 32-bit flash */
#define CFG_OR6_PRELIM		0xfc000ff7
#define CFG_LBC_LCRR		0x00030002	/* local bus freq	*/
#define CFG_LBC_LBCR		0x00000000
#define CFG_LBC_LSRT		0x20000000
#define CFG_LBC_MRTPR		0x20000000
#define CFG_LBC_LSDMR_1		0x2861b723
#define CFG_LBC_LSDMR_2		0x0861b723
#define CFG_LBC_LSDMR_3		0x0861b723
#define CFG_LBC_LSDMR_4		0x1861b723
#define CFG_LBC_LSDMR_5		0x4061b723

/* just hijack the MOT BCSR def for SBC8560 misc devices */
#define CFG_BCSR		((CFG_BR5_PRELIM & 0xff000000)|0x00400000)
/* the size of CS5 needs to be >= 16M for TLB and LAW setups */

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0x70000000	/* Initial RAM address	*/
#define CFG_INIT_RAM_END	0x4000		/* End of used area in RAM */

#define CFG_GBL_DATA_SIZE	128		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserved for malloc */

/* Serial Port */
#undef  CONFIG_CONS_ON_SCC			/* define if console on SCC */
#undef	CONFIG_CONS_NONE			/* define if console on something else */

#define CONFIG_CONS_INDEX     1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#if 0
#define CFG_NS16550_CLK		1843200 /* get_bus_freq(0) */
#else
#define CFG_NS16550_CLK		264000000 /* get_bus_freq(0) */
#endif

#define CONFIG_BAUDRATE		9600

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#if 0
#define CFG_NS16550_COM1	((CFG_BR5_PRELIM & 0xff000000)+0x00700000)
#define CFG_NS16550_COM2	((CFG_BR5_PRELIM & 0xff000000)+0x00800000)
#else
/* SBC8540 uses internal COMM controller */
#define CFG_NS16550_COM1	((CFG_CCSRBAR & 0xfff00000)+0x00004500)
#define CFG_NS16550_COM2	((CFG_CCSRBAR & 0xfff00000)+0x00004600)
#endif

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
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000

#define CFG_PCI_MEM_BASE	0xC0000000
#define CFG_PCI_MEM_PHYS	0xC0000000
#define CFG_PCI_MEM_SIZE	0x10000000

#if defined(CONFIG_TSEC_ENET)		/* TSEC Ethernet port */

#  define CONFIG_NET_MULTI	1
#  define CONFIG_MPC85xx_TSEC1
#  define CONFIG_MPC85xx_TSEC1_NAME	"TSEC0"
#  define CONFIG_MII		1	/* MII PHY management		*/
#  define TSEC1_PHY_ADDR	25
#  define TSEC1_PHYIDX		0
/* Options are: TSEC0 */
#  define CONFIG_ETHPRIME		"TSEC0"


#elif defined(CONFIG_ETHER_ON_FCC)	/* CPM FCC Ethernet */

  #undef  CONFIG_ETHER_NONE		/* define if ether on something else */
  #define CONFIG_ETHER_ON_FCC2		/* cpm FCC ethernet support	*/
  #define CONFIG_ETHER_INDEX	2	/* which channel for ether  */

  #if (CONFIG_ETHER_INDEX == 2)
    /*
     * - Rx-CLK is CLK13
     * - Tx-CLK is CLK14
     * - Select bus for bd/buffers
     * - Full duplex
     */
    #define CFG_CMXFCR_MASK	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
    #define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)
    #define CFG_CPMFCR_RAMTYPE	0
    #define CFG_FCC_PSMR	(FCC_PSMR_FDE)

  #elif (CONFIG_ETHER_INDEX == 3)
    /* need more definitions here for FE3 */
  #endif				/* CONFIG_ETHER_INDEX */

  #define CONFIG_MII			/* MII PHY management */
  #define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
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

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CFG_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CFG_FLASH_CFI_DRIVER	1	/* Use the common driver		*/
#if 0
#define CFG_FLASH_USE_BUFFER_WRITE 1    /* use buffered writes (20x faster)     */
#define CFG_FLASH_PROTECTION		/* use hardware protection		*/
#endif
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	200000		/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	50000		/* Timeout for Flash Write (in ms)	*/

#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor	*/

#if 0
/* XXX This doesn't work and I don't want to fix it */
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
  #define CFG_RAMBOOT
#else
  #undef  CFG_RAMBOOT
#endif
#endif

/* Environment */
#if !defined(CFG_RAMBOOT)
  #if defined(CONFIG_RAM_AS_FLASH)
    #define CFG_ENV_IS_NOWHERE
    #define CFG_ENV_ADDR	(CFG_FLASH_BASE + 0x100000)
    #define CFG_ENV_SIZE	0x2000
  #else
    #define CFG_ENV_IS_IN_FLASH	1
    #define CFG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
    #define CFG_ENV_ADDR	(CFG_MONITOR_BASE - CFG_ENV_SECT_SIZE)
    #define CFG_ENV_SIZE	0x2000 /* CFG_ENV_SECT_SIZE */
  #endif
#else
  #define CFG_NO_FLASH		1	/* Flash is not usable now	*/
  #define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only	*/
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
  #define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_BOOTARGS "root=/dev/nfs rw nfsroot=192.168.0.251:/tftpboot ip=192.168.0.105:192.168.0.251::255.255.255.0:sbc8560:eth0:off console=ttyS0,9600"
/*#define CONFIG_BOOTARGS      "root=/dev/ram rw console=ttyS0,115200"*/
#define CONFIG_BOOTCOMMAND	"bootm 0xff800000 0xffa00000"
#define CONFIG_BOOTDELAY	5	/* -1 disable autoboot */

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

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
    #define CONFIG_CMD_MII
#endif

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"SBC8540=> " /* Monitor Command Prompt	*/
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
#  define CONFIG_ETHADDR	00:vv:ww:xx:yy:8a
#  define CONFIG_HAS_ETH1
#  define CONFIG_ETH1ADDR	00:vv:ww:xx:yy:8b
#  define CONFIG_HAS_ETH2
#  define CONFIG_ETH2ADDR	00:vv:ww:xx:yy:8c
#endif

#define CONFIG_SERVERIP		YourServerIP
#define CONFIG_IPADDR		YourTargetIP
#define CONFIG_GATEWAYIP	YourGatewayIP
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_HOSTNAME		SBC8560
#define CONFIG_ROOTPATH		YourRootPath
#define CONFIG_BOOTFILE		YourImageName

#endif	/* __CONFIG_H */
