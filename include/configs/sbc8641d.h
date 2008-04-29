/*
 * Copyright 2007 Wind River Systems <www.windriver.com>
 * Copyright 2007 Embedded Specialties, Inc.
 * Joe Hamman <joe.hamman@embeddedspecialties.com>
 *
 * Copyright 2006 Freescale Semiconductor.
 *
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
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
 * SBC8641D board configuration file
 *
 * Make sure you change the MAC address and other network params first,
 * search for CONFIG_ETHADDR, CONFIG_SERVERIP, etc in this file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_MPC86xx		1	/* MPC86xx */
#define CONFIG_MPC8641		1	/* MPC8641 specific */
#define CONFIG_SBC8641D		1	/* SBC8641D board specific */
#define CONFIG_NUM_CPUS         2       /* Number of CPUs in the system */
#define CONFIG_LINUX_RESET_VEC  0x100   /* Reset vector used by Linux */

#ifdef RUN_DIAG
#define CFG_DIAG_ADDR        0xff800000
#endif

#define CFG_RESET_ADDRESS    0xfff00100

#define CONFIG_PCI		1	/* Enable PCIE */
#define CONFIG_PCI1		1	/* PCIE controler 1 (slot 1) */
#define CONFIG_PCI2		1	/* PCIE controler 2 (slot 2) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

#define CONFIG_TSEC_ENET 		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#undef CONFIG_SPD_EEPROM		/* Do not use SPD EEPROM for DDR setup*/
#undef CONFIG_DDR_DLL			/* possible DLL fix needed */
#define CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */
#undef CONFIG_DDR_ECC  			/* only for ECC DDR module */
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef
#define CONFIG_NUM_DDR_CONTROLLERS     2
#define CACHE_LINE_INTERLEAVING		0x20000000
#define PAGE_INTERLEAVING		0x21000000
#define BANK_INTERLEAVING		0x22000000
#define SUPER_BANK_INTERLEAVING		0x23000000


#define CONFIG_ALTIVEC          1

/*
 * L2CR setup -- make sure this is right for your board!
 */
#define CFG_L2
#define L2_INIT		0
#define L2_ENABLE	(L2CR_L2E)

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ     get_board_sys_clk(0)
#endif

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CFG_DRAM_TEST				/* memory test, takes time */
#define CFG_MEMTEST_START	0x00200000	/* memtest region */
#define CFG_MEMTEST_END		0x00400000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT 	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xf8000000	/* relocated CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

#define CFG_PCI1_ADDR		(CFG_CCSRBAR+0x8000)
#define CFG_PCI2_ADDR		(CFG_CCSRBAR+0x9000)

/*
 * DDR Setup
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory */
#define CFG_DDR_SDRAM_BASE2	0x10000000	/* DDR bank 2 */
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE
#define CFG_SDRAM_BASE2		CFG_DDR_SDRAM_BASE2
#define CONFIG_VERY_BIG_RAM

#define MPC86xx_DDR_SDRAM_CLK_CNTL

#if defined(CONFIG_SPD_EEPROM)
    /*
     * Determine DDR configuration from I2C interface.
     */
    #define SPD_EEPROM_ADDRESS1		0x51		/* DDR DIMM */
    #define SPD_EEPROM_ADDRESS2		0x52		/* DDR DIMM */
    #define SPD_EEPROM_ADDRESS3		0x53		/* DDR DIMM */
    #define SPD_EEPROM_ADDRESS4		0x54		/* DDR DIMM */

#else
    /*
     * Manually set up DDR1 & DDR2 parameters
     */

    #define CFG_SDRAM_SIZE	512		/* DDR is 512MB */

    #define CFG_DDR_CS0_BNDS	0x0000000F
    #define CFG_DDR_CS1_BNDS	0x00000000
    #define CFG_DDR_CS2_BNDS	0x00000000
    #define CFG_DDR_CS3_BNDS	0x00000000
    #define CFG_DDR_CS0_CONFIG	0x80010102
    #define CFG_DDR_CS1_CONFIG	0x00000000
    #define CFG_DDR_CS2_CONFIG	0x00000000
    #define CFG_DDR_CS3_CONFIG	0x00000000
    #define CFG_DDR_TIMING_3 0x00000000
    #define CFG_DDR_TIMING_0	0x00220802
    #define CFG_DDR_TIMING_1	0x38377322
    #define CFG_DDR_TIMING_2	0x002040c7
    #define CFG_DDR_CFG_1A	0x43008008
    #define CFG_DDR_CFG_2	0x24401000
    #define CFG_DDR_MODE_1	0x23c00542
    #define CFG_DDR_MODE_2	0x00000000
    #define CFG_DDR_MODE_CTL	0x00000000
    #define CFG_DDR_INTERVAL	0x05080100
    #define CFG_DDR_DATA_INIT	0x00000000
    #define CFG_DDR_CLK_CTRL	0x03800000
    #define CFG_DDR_CFG_1B	0xC3008008

    #define CFG_DDR2_CS0_BNDS	0x0010001F
    #define CFG_DDR2_CS1_BNDS	0x00000000
    #define CFG_DDR2_CS2_BNDS	0x00000000
    #define CFG_DDR2_CS3_BNDS	0x00000000
    #define CFG_DDR2_CS0_CONFIG	0x80010102
    #define CFG_DDR2_CS1_CONFIG	0x00000000
    #define CFG_DDR2_CS2_CONFIG	0x00000000
    #define CFG_DDR2_CS3_CONFIG	0x00000000
    #define CFG_DDR2_EXT_REFRESH 0x00000000
    #define CFG_DDR2_TIMING_0	0x00220802
    #define CFG_DDR2_TIMING_1	0x38377322
    #define CFG_DDR2_TIMING_2	0x002040c7
    #define CFG_DDR2_CFG_1A	0x43008008
    #define CFG_DDR2_CFG_2	0x24401000
    #define CFG_DDR2_MODE_1	0x23c00542
    #define CFG_DDR2_MODE_2	0x00000000
    #define CFG_DDR2_MODE_CTL	0x00000000
    #define CFG_DDR2_INTERVAL	0x05080100
    #define CFG_DDR2_DATA_INIT	0x00000000
    #define CFG_DDR2_CLK_CTRL	0x03800000
    #define CFG_DDR2_CFG_1B	0xC3008008


#endif

/* #define CFG_ID_EEPROM	1
#define ID_EEPROM_ADDR 0x57 */

/*
 * The SBC8641D contains 16MB flash space at ff000000.
 */
#define CFG_FLASH_BASE      0xff000000  /* start of FLASH 16M */

/* Flash */
#define CFG_BR0_PRELIM		0xff001001	/* port size 16bit */
#define CFG_OR0_PRELIM		0xff006e65	/* 16MB Boot Flash area */

/* 64KB EEPROM */
#define CFG_BR1_PRELIM		0xf0000801	/* port size 16bit */
#define CFG_OR1_PRELIM		0xffff6e65	/* 64K EEPROM area */

/* EPLD - User switches, board id, LEDs */
#define CFG_BR2_PRELIM		0xf1000801	/* port size 16bit */
#define CFG_OR2_PRELIM		0xfff06e65	/* EPLD (switches, board ID, LEDs) area */

/* Local bus SDRAM 128MB */
#define CFG_BR3_PRELIM		0xe0001861	/* port size ?bit */
#define CFG_OR3_PRELIM		0xfc006cc0	/* 128MB local bus SDRAM area (1st half) */
#define CFG_BR4_PRELIM		0xe4001861	/* port size ?bit */
#define CFG_OR4_PRELIM		0xfc006cc0	/* 128MB local bus SDRAM area (2nd half) */

/* Disk on Chip (DOC) 128MB */
#define CFG_BR5_PRELIM		0xe8001001	/* port size ?bit */
#define CFG_OR5_PRELIM		0xf8006e65	/* 128MB local bus SDRAM area (2nd half) */

/* LCD */
#define CFG_BR6_PRELIM		0xf4000801	/* port size ?bit */
#define CFG_OR6_PRELIM		0xfff06e65	/* 128MB local bus SDRAM area (2nd half) */

/* Control logic & misc peripherals */
#define CFG_BR7_PRELIM		0xf2000801	/* port size ?bit */
#define CFG_OR7_PRELIM		0xfff06e65	/* 128MB local bus SDRAM area (2nd half) */

#define CFG_MAX_FLASH_BANKS	1		/* number of banks */
#define CFG_MAX_FLASH_SECT	131		/* sectors per device */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CFG_MONITOR_BASE    	TEXT_BASE	/* start of monitor */

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_WRITE_SWAPPED_DATA
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_PROTECTION

#undef CONFIG_CLOCKS_IN_MHZ

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK	1
#ifndef CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0x0fd00000	/* Initial RAM address */
#else
#define CFG_INIT_RAM_ADDR	0xf8400000	/* Initial RAM address */
#endif
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
 * Pass open firmware flat tree to kernel
 */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

#define CFG_64BIT_VSPRINTF	1
#define CFG_64BIT_STRTOUL	1

/*
 * I2C
 */
#define	CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define	CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES        {0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3100

/*
 * RapidIO MMU
 */
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

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS      0x00000000
#define CFG_PCI_MEMORY_PHYS     0x00000000
#define CFG_PCI_MEMORY_SIZE     0x80000000

#define CFG_PCI2_MEM_BASE	0xa0000000
#define CFG_PCI2_MEM_PHYS	CFG_PCI2_MEM_BASE
#define CFG_PCI2_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCI2_IO_BASE	0xe3000000
#define CFG_PCI2_IO_PHYS	CFG_PCI2_IO_BASE
#define CFG_PCI2_IO_SIZE	0x1000000	/* 16M */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup */

#undef CFG_SCSI_SCAN_BUS_REVERSE

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP	               	/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
    #define PCI_ENET0_IOADDR	0xe0000000
    #define PCI_ENET0_MEMADDR	0xe0000000
    #define PCI_IDSEL_NUMBER	0x0c 	/* slot0->3(IDSEL)=12->15 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#define CONFIG_DOS_PARTITION
#undef CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CFG_SCSI_MAX_SCSI_ID	4
#define CFG_SCSI_MAX_LUN	1
#define CFG_SCSI_MAX_DEVICE 	(CFG_SCSI_MAX_SCSI_ID * CFG_SCSI_MAX_LUN)
#define CFG_SCSI_MAXDEVICE	CFG_SCSI_MAX_DEVICE
#endif

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI 	1
#endif

/* #define CONFIG_MII		1 */	/* MII PHY management */

#define CONFIG_TSEC1    1
#define CONFIG_TSEC1_NAME       "eTSEC1"
#define CONFIG_TSEC2    1
#define CONFIG_TSEC2_NAME       "eTSEC2"
#define CONFIG_TSEC3    1
#define CONFIG_TSEC3_NAME       "eTSEC3"
#define CONFIG_TSEC4    1
#define CONFIG_TSEC4_NAME       "eTSEC4"

#define TSEC1_PHY_ADDR		0x1F
#define TSEC2_PHY_ADDR		0x00
#define TSEC3_PHY_ADDR		0x01
#define TSEC4_PHY_ADDR		0x02
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC3_FLAGS		TSEC_GIGABIT
#define TSEC4_FLAGS		TSEC_GIGABIT

#define CFG_TBIPA_VALUE	0x1e	/* Set TBI address not to conflict with TSEC1_PHY_ADDR */

#define CONFIG_ETHPRIME		"eTSEC1"

#endif	/* CONFIG_TSEC_ENET */

/*
 * BAT0         2G     Cacheable, non-guarded
 * 0x0000_0000  2G     DDR
 */
#define CFG_DBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_DBAT0U	(BATU_BL_2G | BATU_VS | BATU_VP)
#define CFG_IBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE )
#define CFG_IBAT0U	CFG_DBAT0U

/*
 * BAT1         1G     Cache-inhibited, guarded
 * 0x8000_0000  512M   PCI-Express 1 Memory
 * 0xa000_0000  512M   PCI-Express 2 Memory
 *	Changed it for operating from 0xd0000000
 */
#define CFG_DBAT1L	( CFG_PCI1_MEM_BASE | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT1U	(CFG_PCI1_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT1L	(CFG_PCI1_MEM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT1U	CFG_DBAT1U

/*
 * BAT2         512M   Cache-inhibited, guarded
 * 0xc000_0000  512M   RapidIO Memory
 */
#define CFG_DBAT2L	(CFG_RIO_MEM_BASE | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT2U	(CFG_RIO_MEM_BASE | BATU_BL_512M | BATU_VS | BATU_VP)
#define CFG_IBAT2L	(CFG_RIO_MEM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT2U	CFG_DBAT2U

/*
 * BAT3         4M     Cache-inhibited, guarded
 * 0xf800_0000  4M     CCSR
 */
#define CFG_DBAT3L	( CFG_CCSRBAR | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U	(CFG_CCSRBAR | BATU_BL_4M | BATU_VS | BATU_VP)
#define CFG_IBAT3L	(CFG_CCSRBAR | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT3U	CFG_DBAT3U

/*
 * BAT4         32M    Cache-inhibited, guarded
 * 0xe200_0000  16M    PCI-Express 1 I/O
 * 0xe300_0000  16M    PCI-Express 2 I/0
 *    Note that this is at 0xe0000000
 */
#define CFG_DBAT4L	( CFG_PCI1_IO_BASE | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT4U	(CFG_PCI1_IO_BASE | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_IBAT4L	(CFG_PCI1_IO_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT4U	CFG_DBAT4U

/*
 * BAT5         128K   Cacheable, non-guarded
 * 0xe401_0000  128K   Init RAM for stack in the CPU DCache (no backing memory)
 */
#define CFG_DBAT5L	(CFG_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_DBAT5U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_IBAT5L	CFG_DBAT5L
#define CFG_IBAT5U	CFG_DBAT5U

/*
 * BAT6         32M    Cache-inhibited, guarded
 * 0xfe00_0000  32M    FLASH
 */
#define CFG_DBAT6L	((CFG_FLASH_BASE & 0xfe000000) | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT6U	((CFG_FLASH_BASE & 0xfe000000) | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_IBAT6L	((CFG_FLASH_BASE & 0xfe000000) | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_IBAT6U	CFG_DBAT6U

#define CFG_DBAT7L	0x00000000
#define CFG_DBAT7U	0x00000000
#define CFG_IBAT7L	0x00000000
#define CFG_IBAT7U	0x00000000

/*
 * Environment
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x40000)
#define CFG_ENV_SECT_SIZE	0x40000	/* 256K(one sector) for env */
#define CFG_ENV_SIZE		0x2000

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

#include <config_cmd_default.h>
    #define CONFIG_CMD_PING
    #define CONFIG_CMD_I2C
    #define CONFIG_CMD_REGINFO

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
    #define CFG_CBSIZE	256		/* Console I/O Buffer Size */
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
#define CONFIG_ETHADDR   02:E0:0C:00:00:01
#define CONFIG_ETH1ADDR  02:E0:0C:00:01:FD
#define CONFIG_ETH2ADDR  02:E0:0C:00:02:FD
#define CONFIG_ETH3ADDR  02:E0:0C:00:03:FD
#endif

#define CONFIG_HAS_ETH0		1
#define CONFIG_HAS_ETH1		1
#define CONFIG_HAS_ETH2		1
#define CONFIG_HAS_ETH3		1

#define CONFIG_IPADDR		192.168.0.50

#define CONFIG_HOSTNAME		sbc8641d
#define CONFIG_ROOTPATH		/opt/eldk/ppc_74xx
#define CONFIG_BOOTFILE		uImage

#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_NETMASK		255.255.255.0

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS					\
   "netdev=eth0\0"							\
   "consoledev=ttyS0\0"							\
   "ramdiskaddr=2000000\0"						\
   "ramdiskfile=uRamdisk\0"						\
   "dtbaddr=400000\0"							\
   "dtbfile=sbc8641d.dtb\0"						\
   "en-wd=mw.b f8100010 0x08; echo -expect:- 08; md.b f8100010 1\0"	\
   "dis-wd=mw.b f8100010 0x00; echo -expect:- 00; md.b f8100010 1\0"	\
   "maxcpus=1"

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "	\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $dtbaddr $dtbfile;"						\
   "bootm $loadaddr - $dtbaddr"

#define CONFIG_RAMBOOTCOMMAND						\
   "setenv bootargs root=/dev/ram rw "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "	\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $dtbaddr $dtbfile;"						\
   "bootm $loadaddr $ramdiskaddr $dtbaddr"

#define CONFIG_FLASHBOOTCOMMAND						\
   "setenv bootargs root=/dev/ram rw "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "	\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "bootm ffd00000 ffb00000 ffa00000"

#define CONFIG_BOOTCOMMAND  CONFIG_FLASHBOOTCOMMAND

#endif	/* __CONFIG_H */
