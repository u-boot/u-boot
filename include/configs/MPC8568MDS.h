/*
 * Copyright 2004-2007 Freescale Semiconductor.
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
 * mpc8568mds board configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48/68 */
#define CONFIG_MPC8568		1	/* MPC8568 specific */
#define CONFIG_MPC8568MDS	1	/* MPC8568MDS board specific */

#define CONFIG_PCI
#define CONFIG_TSEC_ENET 		/* tsec ethernet support */
#define CONFIG_QE			/* Enable QE */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/
#define CONFIG_DDR_DLL			/* possible DLL fix needed */
/*#define CONFIG_DDR_2T_TIMING		 Sets the 2T timing bit */

/*#define CONFIG_DDR_ECC*/			/* only for ECC DDR module */
/*#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER*/	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef


/*
 * When initializing flash, if we cannot find the manufacturer ID,
 * assume this is the AMD flash associated with the MDS board.
 * This allows booting from a promjet.
 */
#define CONFIG_ASSUME_AMD_FLASH

#define MPC85xx_DDR_SDRAM_CLK_CNTL	/* 85xx has clock control reg */

#ifndef __ASSEMBLY__
extern unsigned long get_clock_freq(void);
#endif						  /*Replace a call to get_clock_freq (after it is implemented)*/
#define CONFIG_SYS_CLK_FREQ	66000000 /*TODO: restore if wanting to read from BCSR: get_clock_freq()*/ /* sysclk for MPC85xx */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE				/* toggle L2 cache 	*/
#define CONFIG_BTB				/* toggle branch predition */
#define CONFIG_ADDR_STREAMING			/* toggle addr streaming   */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1


#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CFG_DRAM_TEST			/* memory test, takes time */
#define CFG_MEMTEST_START	0x00200000	/* memtest works on */
#define CFG_MEMTEST_END		0x00400000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT 	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

/*
 * DDR Setup
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE

#define SPD_EEPROM_ADDRESS	0x51		/* DDR DIMM */

/*
 * Make sure required options are set
 */
#ifndef CONFIG_SPD_EEPROM
#error ("CONFIG_SPD_EEPROM is required")
#endif

#undef CONFIG_CLOCKS_IN_MHZ


/*
 * Local Bus Definitions
 */

/*
 * FLASH on the Local Bus
 * Two banks, 8M each, using the CFI driver.
 * Boot from BR0/OR0 bank at 0xff00_0000
 * Alternate BR1/OR1 bank at 0xff80_0000
 *
 * BR0, BR1:
 *    Base address 0 = 0xff00_0000 = BR0[0:16] = 1111 1111 0000 0000 0
 *    Base address 1 = 0xff80_0000 = BR1[0:16] = 1111 1111 1000 0000 0
 *    Port Size = 16 bits = BRx[19:20] = 10
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0001 0000 0000 0001 = ff801001    BR0
 * 1111 1111 0000 0000 0001 0000 0000 0001 = ff001001    BR1
 *
 * OR0, OR1:
 *    Addr Mask = 8M = ORx[0:16] = 1111 1111 1000 0000 0
 *    Reserved ORx[17:18] = 11, confusion here?
 *    CSNT = ORx[20] = 1
 *    ACS = half cycle delay = ORx[21:22] = 11
 *    SCY = 6 = ORx[24:27] = 0110
 *    TRLX = use relaxed timing = ORx[29] = 1
 *    EAD = use external address latch delay = OR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0110 1110 0110 0101 = ff806e65    ORx
 */
#define CFG_BCSR_BASE		0xf8000000

#define CFG_FLASH_BASE		0xfe000000	/* start of FLASH 32M */

/*Chip select 0 - Flash*/
#define CFG_BR0_PRELIM		0xfe001001
#define	CFG_OR0_PRELIM		0xfe006ff7

/*Chip slelect 1 - BCSR*/
#define CFG_BR1_PRELIM		0xf8000801
#define	CFG_OR1_PRELIM		0xffffe9f7

/*#define CFG_FLASH_BANKS_LIST	{0xff800000, CFG_FLASH_BASE} */
#define CFG_MAX_FLASH_BANKS		1		/* number of banks */
#define CFG_MAX_FLASH_SECT		512		/* sectors per device */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500		/* Flash Write Timeout (ms) */

#define CFG_MONITOR_BASE    	TEXT_BASE	/* start of monitor */

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO


/*
 * SDRAM on the LocalBus
 */
#define CFG_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM	 */
#define CFG_LBC_SDRAM_SIZE	64			/* LBC SDRAM is 64MB */


/*Chip select 2 - SDRAM*/
#define CFG_BR2_PRELIM      0xf0001861
#define CFG_OR2_PRELIM		0xfc006901

#define CFG_LBC_LCRR		0x00030004    	/* LB clock ratio reg */
#define CFG_LBC_LBCR		0x00000000    	/* LB config reg */
#define CFG_LBC_LSRT		0x20000000  	/* LB sdram refresh timer */
#define CFG_LBC_MRTPR		0x00000000  	/* LB refresh timer prescal*/

/*
 * LSDMR masks
 */
#define CFG_LBC_LSDMR_RFEN	(1 << (31 -  1))
#define CFG_LBC_LSDMR_BSMA1516	(3 << (31 - 10))
#define CFG_LBC_LSDMR_BSMA1617	(4 << (31 - 10))
#define CFG_LBC_LSDMR_RFCR16	(7 << (31 - 16))
#define CFG_LBC_LSDMR_PRETOACT7	(7 << (31 - 19))
#define CFG_LBC_LSDMR_ACTTORW7	(7 << (31 - 22))
#define CFG_LBC_LSDMR_ACTTORW6	(6 << (31 - 22))
#define CFG_LBC_LSDMR_BL8	(1 << (31 - 23))
#define CFG_LBC_LSDMR_WRC4	(0 << (31 - 27))
#define CFG_LBC_LSDMR_CL3	(3 << (31 - 31))

#define CFG_LBC_LSDMR_OP_NORMAL	(0 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ARFRSH	(1 << (31 - 4))
#define CFG_LBC_LSDMR_OP_SRFRSH	(2 << (31 - 4))
#define CFG_LBC_LSDMR_OP_MRW	(3 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PRECH	(4 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PCHALL	(5 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ACTBNK	(6 << (31 - 4))
#define CFG_LBC_LSDMR_OP_RWINV	(7 << (31 - 4))

/*
 * Common settings for all Local Bus SDRAM commands.
 * At run time, either BSMA1516 (for CPU 1.1)
 *                  or BSMA1617 (for CPU 1.0) (old)
 * is OR'ed in too.
 */
#define CFG_LBC_LSDMR_COMMON	( CFG_LBC_LSDMR_RFCR16		\
				| CFG_LBC_LSDMR_PRETOACT7	\
				| CFG_LBC_LSDMR_ACTTORW7	\
				| CFG_LBC_LSDMR_BL8		\
				| CFG_LBC_LSDMR_WRC4		\
				| CFG_LBC_LSDMR_CL3		\
				| CFG_LBC_LSDMR_RFEN		\
				)

/*
 * The bcsr registers are connected to CS3 on MDS.
 * The new memory map places bcsr at 0xf8000000.
 *
 * For BR3, need:
 *    Base address of 0xf8000000 = BR[0:16] = 1111 1000 0000 0000 0
 *    port-size = 8-bits  = BR[19:20] = 01
 *    no parity checking  = BR[21:22] = 00
 *    GPMC for MSEL       = BR[24:26] = 000
 *    Valid               = BR[31]    = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1000 0000 0000 0000 1000 0000 0001 = f8000801
 *
 * For OR3, need:
 *    1 MB mask for AM,   OR[0:16]  = 1111 1111 1111 0000 0
 *    disable buffer ctrl OR[19]    = 0
 *    CSNT                OR[20]    = 1
 *    ACS                 OR[21:22] = 11
 *    XACS                OR[23]    = 1
 *    SCY 15 wait states  OR[24:27] = 1111	max is suboptimal but safe
 *    SETA                OR[28]    = 0
 *    TRLX                OR[29]    = 1
 *    EHTR                OR[30]    = 1
 *    EAD extra time      OR[31]    = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1111 0000 0000 1111 1111 0111 = fff00ff7
 */
#define CFG_BCSR (0xf8000000)

/*Chip slelect 4 - PIB*/
#define CFG_BR4_PRELIM   0xf8008801
#define CFG_OR4_PRELIM   0xffffe9f7

/*Chip select 5 - PIB*/
#define CFG_BR5_PRELIM	 0xf8010801
#define CFG_OR5_PRELIM	 0xffff69f7

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK 	1
#define CFG_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CFG_INIT_RAM_END    	0x4000	    /* End of used area in RAM */

#define CFG_GBL_DATA_SIZE  	128	    /* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN	    	(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN	    	(128 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX		1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE    1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1        (CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2        (CFG_CCSRBAR+0x4600)

/* Use the HUSH parser*/
#define CFG_HUSH_PARSER
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_FLAT_TREE	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,8568@0"
#define OF_SOC			"soc8568@e0000000"
#define OF_QE			"qe@e0080000"
#define OF_TBCLK		(bd->bi_busfreq / 8)
#define OF_STDOUT_PATH		"/soc8568@e0000000/serial@4600"

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_EEPROM_ADDR	0x52
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES        {{0,0x69}}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000
#define CFG_I2C2_OFFSET		0x3100

/*
 * General PCI
 * Memory Addresses are mapped 1-1. I/O is mapped from 0
 */
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xe2000000
#define CFG_PCI1_IO_SIZE	0x00800000	/* 8M */

#define CFG_PEX_MEM_BASE	0xa0000000
#define CFG_PEX_MEM_PHYS	CFG_PEX_MEM_BASE
#define CFG_PEX_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PEX_IO_BASE		0x00000000
#define CFG_PEX_IO_PHYS		0xe2800000
#define CFG_PEX_IO_SIZE		0x00800000	/* 8M */

#define CFG_SRIO_MEM_BASE	0xc0000000

#if defined(CONFIG_PCI)

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP	               	/* do pci plug-and-play */

#ifdef CONFIG_QE
/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#ifndef CONFIG_TSEC_ENET
#define CONFIG_ETHPRIME         "FSL UEC0"
#endif
#define CONFIG_PHY_MODE_NEED_CHANGE
#define CONFIG_eTSEC_MDIO_BUS

#ifdef CONFIG_eTSEC_MDIO_BUS
#define CONFIG_MIIM_ADDRESS 	0xE0024520
#endif

#define CONFIG_UEC_ETH1         /* GETH1 */

#ifdef CONFIG_UEC_ETH1
#define CFG_UEC1_UCC_NUM        0       /* UCC1 */
#define CFG_UEC1_RX_CLK         QE_CLK_NONE
#define CFG_UEC1_TX_CLK         QE_CLK16
#define CFG_UEC1_ETH_TYPE       GIGA_ETH
#define CFG_UEC1_PHY_ADDR       7
#define CFG_UEC1_INTERFACE_MODE ENET_1000_GMII
#endif

#define CONFIG_UEC_ETH2         /* GETH2 */

#ifdef CONFIG_UEC_ETH2
#define CFG_UEC2_UCC_NUM        1       /* UCC2 */
#define CFG_UEC2_RX_CLK         QE_CLK_NONE
#define CFG_UEC2_TX_CLK         QE_CLK16
#define CFG_UEC2_ETH_TYPE       GIGA_ETH
#define CFG_UEC2_PHY_ADDR       1
#define CFG_UEC2_INTERFACE_MODE ENET_1000_GMII
#endif
#endif /* CONFIG_QE */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CFG_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */

#endif	/* CONFIG_PCI */

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI 	1
#endif

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC1"

#define TSEC1_PHY_ADDR		2
#define TSEC2_PHY_ADDR		3

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0

#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: eTSEC[0-1] */
#define CONFIG_ETHPRIME		"eTSEC0"

#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x40000)
#define CFG_ENV_SECT_SIZE	0x40000	/* 256K(one sector) for env */
#define CFG_ENV_SIZE		0x2000

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */


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
#define CONFIG_CMD_MII

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	*/
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE	256			/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20) 	/* Initial Memory map for Linux*/

/* Cache Configuration */
#define CFG_DCACHE_SIZE	32768
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
#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_UEC_ETH)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR   00:E0:0C:00:00:FD
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR  00:E0:0C:00:01:FD
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR  00:E0:0C:00:02:FD
#define CONFIG_HAS_ETH3
#define CONFIG_ETH3ADDR  00:E0:0C:00:03:FD
#endif

#define CONFIG_IPADDR    192.168.1.253

#define CONFIG_HOSTNAME  unknown
#define CONFIG_ROOTPATH  /nfsroot
#define CONFIG_BOOTFILE  your.uImage

#define CONFIG_SERVERIP  192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_NETMASK   255.255.255.0

#define CONFIG_LOADADDR  200000   /*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 10       /* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS           /* the boot command will set bootargs*/

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS				        \
   "netdev=eth0\0"                                                      \
   "consoledev=ttyS0\0"                                                 \
   "ramdiskaddr=600000\0"                                               \
   "ramdiskfile=your.ramdisk.u-boot\0"					\
   "fdtaddr=400000\0"							\
   "fdtfile=your.fdt.dtb\0"						\
   "nfsargs=setenv bootargs root=/dev/nfs rw "				\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs\0"			\
   "ramargs=setenv bootargs root=/dev/ram rw "				\
      "console=$consoledev,$baudrate $othbootargs\0"			\


#define CONFIG_NFSBOOTCOMMAND	                                        \
   "run nfsargs;"							\
   "tftp $loadaddr $bootfile;"                                          \
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr - $fdtaddr"


#define CONFIG_RAMBOOTCOMMAND \
   "run ramargs;"							\
   "tftp $ramdiskaddr $ramdiskfile;"                                    \
   "tftp $loadaddr $bootfile;"                                          \
   "bootm $loadaddr $ramdiskaddr"

#define CONFIG_BOOTCOMMAND  CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
