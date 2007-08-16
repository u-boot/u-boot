/*
 * (C) Copyright 2002,2003 Motorola,Inc.
 * Modified by Lunsheng Wang, lunsheng@sohu.com
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

/* mpc8540eval board configuration file */
/* please refer to doc/README.mpc85xxads for more info */
/* make sure you change the MAC address and other network params first,
 * search for CONFIG_ETHADDR,CONFIG_SERVERIP,etc in this file
 */

#ifndef __CONFIG_H
#define __CONFIG_H
/* High Level Configuration Options */
#define CONFIG_BOOKE		1	    /* BOOKE 			*/
#define CONFIG_E500		1	    /* BOOKE e500 family	*/
#define CONFIG_MPC85xx		1	    /* MPC8540/MPC8560		*/
#define CONFIG_MPC8540		1	    /* MPC8540 specific	        */
#define CONFIG_MPC8540EVAL	1	    /* MPC8540EVAL board specific */

#undef  CONFIG_PCI	         	    /* pci ethernet support	*/
#define CONFIG_TSEC_ENET 		    /* tsec ethernet support  */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SPD_EEPROM                   /* Use SPD EEPROM for DDR setup */
#undef  CONFIG_DDR_ECC			    /* only for ECC DDR module */
#define CONFIG_DDR_DLL                      /* possible DLL fix needed */

/* Using Localbus SDRAM to emulate flash before we can program the flash,
 * normally you only need a flash-boot image(u-boot.bin),if unsure undef this.
 * Not availabe for EVAL board
 */
#undef CONFIG_RAM_AS_FLASH

/* sysclk for MPC8540EVAL */
#if defined(CONFIG_SYSCLK_66M)
	/*
	 * the oscillator on board is 66Mhz
	 * can also get 66M clock from external PCI
	 */
	#define CONFIG_SYS_CLK_FREQ   66000000
#else
	#define CONFIG_SYS_CLK_FREQ   33000000   /* most pci cards are 33Mhz */
#endif

/* below can be toggled for performance analysis. otherwise use default */
#define CONFIG_L2_CACHE		    	    /* toggle L2 cache 	*/
#undef  CONFIG_BTB			    /* toggle branch predition */
#undef  CONFIG_ADDR_STREAMING		    /* toggle addr streaming   */

#define CONFIG_BOARD_PRE_INIT	1	    /* Call board_pre_init	*/

#undef	CFG_DRAM_TEST			    /* memory test, takes time  */
#define CFG_MEMTEST_START	0x00200000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00400000

#if defined(CONFIG_PCI) && defined(CONFIG_TSEC_ENET)
#error "You can only use either PCI Ethernet Card or TSEC Ethernet, not both."
#endif

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT 	0xff700000	/* CCSRBAR Default	*/
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR 	*/
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR	*/

#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory  */
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE
#define CFG_SDRAM_SIZE		256             /* DDR is now 256MB     */

#if defined(CONFIG_RAM_AS_FLASH)
#define CFG_LBC_SDRAM_BASE	0xfc000000	/* Localbus SDRAM */
#else
#define CFG_LBC_SDRAM_BASE      0xf0000000      /* Localbus SDRAM */
#endif
#define CFG_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 0MB	*/

#if defined(CONFIG_RAM_AS_FLASH)
#define CFG_FLASH_BASE          0xf8000000      /* start of FLASH  16M  */
#define CFG_BR0_PRELIM          0xf8001801      /* port size 32bit */
#else /* Boot from real Flash */
#define CFG_FLASH_BASE		0xff800000	/* start of FLASH 8M    */
#define CFG_BR0_PRELIM		0xff801001	/* port size 16bit	*/
#endif

#define	CFG_OR0_PRELIM		0xff806f67	/* 8MB Flash		*/
#define CFG_MAX_FLASH_BANKS	1		/* number of banks	*/
#define CFG_MAX_FLASH_SECT	64		/* sectors per device   */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Timeout for Flash Erase (in ms)*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)*/
#define CFG_FLASH_CFI		1

#define CFG_MONITOR_BASE    	TEXT_BASE	/* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef  CFG_RAMBOOT
#endif

#define SPD_EEPROM_ADDRESS	0x51		/* DDR DIMM */

/* Here some DDR setting should be added */


#undef CONFIG_CLOCKS_IN_MHZ

/* local bus definitions */
#define CFG_BR2_PRELIM		0xf0001861	/* 64MB localbus SDRAM  */
#define CFG_OR2_PRELIM		0xfc006901
#define CFG_LBC_LCRR		0x00030004	/* local bus freq divider*/
#define CFG_LBC_LBCR		0x00000000
#define CFG_LBC_LSRT		0x20000000
#define CFG_LBC_MRTPR		0x20000000
#define CFG_LBC_LSDMR_1		0x2861b723
#define CFG_LBC_LSDMR_2		0x0861b723
#define CFG_LBC_LSDMR_3		0x0861b723
#define CFG_LBC_LSDMR_4		0x1861b723
#define CFG_LBC_LSDMR_5		0x4061b723

#if defined(CONFIG_RAM_AS_FLASH)
#define CFG_BR4_PRELIM          0xf8000801      /* 32KB, 8-bit wide for ADS config reg */
#else
#define CFG_BR4_PRELIM          0xf8000801      /* 32KB, 8-bit wide for ADS config reg */
#endif
#define CFG_OR4_PRELIM          0xffffe1f1
#define CFG_BCSR                (CFG_BR4_PRELIM & 0xffff8000)

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK 	1
#define CFG_INIT_RAM_ADDR   	0x40000000 	/* Initial RAM address	*/
#define CFG_INIT_RAM_END    	0x4000	    	/* End of used area in RAM */

#define CFG_GBL_DATA_SIZE  	128		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN	    	(256 * 1024)    /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN	    	(128 * 1024)    /* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX     1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE    1
#define CFG_NS16550_CLK		get_bus_freq(0)
#define CONFIG_BAUDRATE	 	115200

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1        (CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2        (CFG_CCSRBAR+0x4600)

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
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES        {0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000

/* General PCI */
#define CFG_PCI_MEM_BASE	0x80000000
#define CFG_PCI_MEM_PHYS	0x80000000
#define CFG_PCI_MEM_SIZE	0x20000000
#define CFG_PCI_IO_BASE         0xe2000000

#if defined(CONFIG_PCI)
#define CONFIG_NET_MULTI
#undef CONFIG_EEPRO100
#define CONFIG_TULIP
#define CONFIG_PCI_PNP	               	/* do pci plug-and-play */
#if !defined(CONFIG_PCI_PNP)
#define PCI_ENET0_IOADDR      0xe0000000
#define PCI_ENET0_MEMADDR     0xe0000000
#define PCI_IDSEL_NUMBER      0x0c 	/*slot0->3(IDSEL)=12->15*/
#endif
#define CONFIG_PCI_SCAN_SHOW    1       /* show pci devices on startup  */
#define CFG_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */
#define CFG_PCI_SUBSYS_DEVICEID 0x0008
#elif defined(CONFIG_TSEC_ENET)
#define CONFIG_NET_MULTI 	1
#define CONFIG_MII		1	/* MII PHY management	*/
#define CONFIG_TSEC1    1
#define CONFIG_TSEC1_NAME      "TSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME      "TSEC1"
#define CONFIG_MPC85XX_FEC      1
#define CONFIG_MPC85XX_FEC_NAME                "FEC"
#define TSEC1_PHY_ADDR          7
#define	TSEC2_PHY_ADDR		4
#define FEC_PHY_ADDR            2
#define TSEC1_PHYIDX            0
#define TSEC2_PHYIDX            0
#define FEC_PHYIDX              0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define FEC_FLAGS		0

/* Options are: TSEC[0-1], FEC */
#define CONFIG_ETHPRIME                "TSEC0"

#define CONFIG_PHY_M88E1011     1       /* GigaBit Ether PHY    */
#define INTEL_LXT971_PHY	1
#endif

#undef DEBUG

/* Environment */
#ifndef CFG_RAMBOOT
#if defined(CONFIG_RAM_AS_FLASH)
#define CFG_ENV_IS_NOWHERE
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x100000)
#define CFG_ENV_SIZE		0x2000
#else
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x40000)
#define CFG_ENV_SECT_SIZE	0x40000	/* 256K(one sector) for env */
#endif
#define CFG_ENV_SIZE		0x2000
#else
/* #define CFG_NO_FLASH		1 */	/* Flash is not usable now	*/
#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
#define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_BOOTARGS	"root=/dev/ram rw console=ttyS0,115200"
#define CONFIG_BOOTCOMMAND	"bootm 0xff800000 0xffa00000"
#define CONFIG_BOOTDELAY	3 	/* -1 disable autoboot */

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

#if defined(CFG_RAMBOOT) || defined(CONFIG_RAM_AS_FLASH)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_LOAD_ADDR   0x2000000       /* default load address */
#define CFG_PROMPT	"MPC8540EVAL=> "/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20) 	/* Initial Memory map for Linux */

/* Cache Configuration */
#define CFG_DCACHE_SIZE	32768
#define CFG_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

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

/*****************************/
/* Environment Configuration */
/*****************************/
/* The mac addresses for all ethernet interface */
/* NOTE: change below for your network setting!!! */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_ETHADDR  00:01:af:07:9b:8a
#define CONFIG_ETH1ADDR  00:01:af:07:9b:8b
#define CONFIG_ETH2ADDR  00:01:af:07:9b:8c
#endif

#define CONFIG_ROOTPATH  /nfsroot
#define CONFIG_BOOTFILE  your.uImage

#define CONFIG_SERVERIP         192.168.101.1
#define CONFIG_IPADDR           192.168.101.11
#define CONFIG_GATEWAYIP        192.168.101.0
#define CONFIG_NETMASK          255.255.255.0

#define CONFIG_LOADADDR  200000   /* default location for tftp and bootm */

#define CONFIG_HOSTNAME         MPC8540EVAL

#endif	/* __CONFIG_H */
