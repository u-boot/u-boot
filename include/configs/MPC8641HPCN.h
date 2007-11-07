/*
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
 * MPC8641HPCN board configuration file
 *
 * Make sure you change the MAC address and other network params first,
 * search for CONFIG_ETHADDR, CONFIG_SERVERIP, etc in this file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_MPC86xx		1	/* MPC86xx */
#define CONFIG_MPC8641		1	/* MPC8641 specific */
#define CONFIG_MPC8641HPCN	1	/* MPC8641HPCN board specific */
#define CONFIG_NUM_CPUS         2       /* Number of CPUs in the system */
#define CONFIG_LINUX_RESET_VEC  0x100   /* Reset vector used by Linux */
#undef DEBUG

#ifdef RUN_DIAG
#define CFG_DIAG_ADDR        0xff800000
#endif

#define CFG_RESET_ADDRESS    0xfff00100

#define CONFIG_PCI		1	/* Enable PCI/PCIE */
#define CONFIG_PCI1		1	/* PCIE controler 1 (ULI bridge) */
#define CONFIG_PCI2		1	/* PCIE controler 2 (slot) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */

#define CONFIG_TSEC_ENET 		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/
#undef CONFIG_DDR_DLL			/* possible DLL fix needed */
#define CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */
#define CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef
#define CONFIG_NUM_DDR_CONTROLLERS     2
/* #define CONFIG_DDR_INTERLEAVE               1 */
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
#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif
#define CONFIG_SYS_CLK_FREQ     get_board_sys_clk(0)
#endif

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CFG_DRAM_TEST			/* memory test, takes time */
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
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE
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
     * Manually set up DDR1 parameters
     */

    #define CFG_SDRAM_SIZE	256		/* DDR is 256MB */

    #define CFG_DDR_CS0_BNDS	0x0000000F
    #define CFG_DDR_CS0_CONFIG	0x80010102      /* Enable, no interleaving */
    #define CFG_DDR_EXT_REFRESH 0x00000000
    #define CFG_DDR_TIMING_0    0x00260802
    #define CFG_DDR_TIMING_1	0x39357322
    #define CFG_DDR_TIMING_2	0x14904cc8
    #define CFG_DDR_MODE_1	0x00480432
    #define CFG_DDR_MODE_2	0x00000000
    #define CFG_DDR_INTERVAL	0x06090100
    #define CFG_DDR_DATA_INIT   0xdeadbeef
    #define CFG_DDR_CLK_CTRL    0x03800000
    #define CFG_DDR_OCD_CTRL    0x00000000
    #define CFG_DDR_OCD_STATUS  0x00000000
    #define CFG_DDR_CONTROL	0xe3008000	/* Type = DDR2 */
    #define CFG_DDR_CONTROL2	0x04400000

    /* Not used in fixed_sdram function */

    #define CFG_DDR_MODE	0x00000022
    #define CFG_DDR_CS1_BNDS	0x00000000
    #define CFG_DDR_CS2_BNDS	0x00000FFF	/* Not done */
    #define CFG_DDR_CS3_BNDS	0x00000FFF	/* Not done */
    #define CFG_DDR_CS4_BNDS	0x00000FFF	/* Not done */
    #define CFG_DDR_CS5_BNDS	0x00000FFF	/* Not done */
#endif

#define CFG_ID_EEPROM	1
#define ID_EEPROM_ADDR 0x57

/*
 * In MPC8641HPCN, allocate 16MB flash spaces at fe000000 and ff000000.
 * There is an 8MB flash.  In effect, the addresses from fe000000 to fe7fffff
 * map to fe800000 to ffffffff, and ff000000 to ff7fffff map to ffffffff.
 * However, when u-boot comes up, the flash_init needs hard start addresses
 * to build its info table.  For user convenience, the flash addresses is
 * fe800000 and ff800000.  That way, u-boot knows where the flash is
 * and the user can download u-boot code from promjet to fef00000, a
 * more intuitive location than fe700000.
 *
 * Note that, on switching the boot location, fef00000 becomes fff00000.
 */
#define CFG_FLASH_BASE          0xfe800000     /* start of FLASH 32M */
#define CFG_FLASH_BASE2		0xff800000

#define CFG_FLASH_BANKS_LIST {CFG_FLASH_BASE, CFG_FLASH_BASE2}

#define CFG_BR0_PRELIM		0xff001001	/* port size 16bit */
#define CFG_OR0_PRELIM		0xff006ff7	/* 16MB Boot Flash area*/

#define CFG_BR1_PRELIM		0xfe001001	/* port size 16bit */
#define CFG_OR1_PRELIM		0xff006ff7	/* 16MB Alternate Boot Flash area*/

#define CFG_BR2_PRELIM		0xf8201001	/* port size 16bit */
#define CFG_OR2_PRELIM		0xfff06ff7	/* 1MB Compact Flash area*/

#define CFG_BR3_PRELIM		0xf8100801	/* port size 8bit */
#define CFG_OR3_PRELIM		0xfff06ff7	/* 1MB PIXIS area*/


#define CONFIG_FSL_PIXIS	1	/* use common PIXIS code */
#define PIXIS_BASE	0xf8100000      /* PIXIS registers */
#define PIXIS_ID		0x0	/* Board ID at offset 0 */
#define PIXIS_VER		0x1	/* Board version at offset 1 */
#define PIXIS_PVER		0x2	/* PIXIS FPGA version at offset 2 */
#define PIXIS_RST		0x4	/* PIXIS Reset Control register */
#define PIXIS_AUX		0x6	/* PIXIS Auxiliary register; Scratch register */
#define PIXIS_SPD		0x7	/* Register for SYSCLK speed */
#define PIXIS_VCTL		0x10	/* VELA Control Register */
#define PIXIS_VCFGEN0		0x12	/* VELA Config Enable 0 */
#define PIXIS_VCFGEN1		0x13	/* VELA Config Enable 1 */
#define PIXIS_VBOOT		0x16	/* VELA VBOOT Register */
#define PIXIS_VSPEED0		0x17	/* VELA VSpeed 0 */
#define PIXIS_VSPEED1		0x18	/* VELA VSpeed 1 */
#define PIXIS_VCLKH		0x19	/* VELA VCLKH register */
#define PIXIS_VCLKL		0x1A	/* VELA VCLKL register */

#define CFG_MAX_FLASH_BANKS	2		/* number of banks */
#define CFG_MAX_FLASH_SECT	128		/* sectors per device */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CFG_MONITOR_BASE    	TEXT_BASE	/* start of monitor */

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef  CFG_RAMBOOT
#endif

#if defined(CFG_RAMBOOT)
#undef CONFIG_SPD_EEPROM
#define CFG_SDRAM_SIZE	256
#endif

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
#define CFG_MALLOC_LEN	    	(1024 * 1024)    /* Reserved for malloc */

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
#define CONFIG_OF_FLAT_TREE	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU		"PowerPC,8641@0"
#define OF_SOC		"soc8641@f8000000"
#define OF_TBCLK	(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH	"/soc8641@f8000000/serial@4500"

#define CFG_64BIT_VSPRINTF	1
#define CFG_64BIT_STRTOUL	1

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
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
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xe2000000
#define CFG_PCI1_IO_SIZE	0x00100000	/* 1M */

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS      0x00000000
#define CFG_PCI_MEMORY_PHYS     0x00000000
#define CFG_PCI_MEMORY_SIZE     0x80000000

/* For RTL8139 */
#define KSEG1ADDR(x)		({u32 _x=le32_to_cpu(*(u32 *)(x)); (&_x);})
#define _IO_BASE                0x00000000

#define CFG_PCI2_MEM_BASE	0xa0000000
#define CFG_PCI2_MEM_PHYS	CFG_PCI2_MEM_BASE
#define CFG_PCI2_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI2_IO_BASE	0x00000000
#define CFG_PCI2_IO_PHYS	0xe3000000
#define CFG_PCI2_IO_SIZE	0x00100000	/* 1M */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup */

#undef CFG_SCSI_SCAN_BUS_REVERSE

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP	               	/* do pci plug-and-play */

#define CONFIG_RTL8139

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

/************************************************************
 * USB support
 ************************************************************/
#define CONFIG_PCI_OHCI		1
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_USB_KEYBOARD	1
#define CFG_DEVICE_DEREGISTER
#define CFG_USB_EVENT_POLL	1
#define CFG_USB_OHCI_SLOT_NAME 	"ohci_pci"
#define CFG_USB_OHCI_MAX_ROOT_PORTS 15
#define CFG_OHCI_SWAP_REG_ACCESS	1

#if !defined(CONFIG_PCI_PNP)
    #define PCI_ENET0_IOADDR	0xe0000000
    #define PCI_ENET0_MEMADDR	0xe0000000
    #define PCI_IDSEL_NUMBER	0x0c 	/* slot0->3(IDSEL)=12->15 */
#endif

/*PCIE video card used*/
#define VIDEO_IO_OFFSET		CFG_PCI2_IO_PHYS

/*PCI video card used*/
/*#define VIDEO_IO_OFFSET	CFG_PCI1_IO_PHYS*/

/* video */
#define CONFIG_VIDEO

#if defined(CONFIG_VIDEO)
#define CONFIG_BIOSEMU
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_ATI_RADEON_FB
#define CONFIG_VIDEO_LOGO
/*#define CONFIG_CONSOLE_CURSOR*/
#define CFG_ISA_IO_BASE_ADDRESS CFG_PCI2_IO_PHYS
#endif

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#define CONFIG_DOS_PARTITION
#define CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CFG_SCSI_MAX_SCSI_ID	4
#define CFG_SCSI_MAX_LUN	1
#define CFG_SCSI_MAX_DEVICE 	(CFG_SCSI_MAX_SCSI_ID * CFG_SCSI_MAX_LUN)
#define CFG_SCSI_MAXDEVICE	CFG_SCSI_MAX_DEVICE
#endif

#define CONFIG_MPC86XX_PCI2

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI 	1
#endif

#define CONFIG_MII		1	/* MII PHY management */

#define CONFIG_TSEC1    1
#define CONFIG_TSEC1_NAME       "eTSEC1"
#define CONFIG_TSEC2    1
#define CONFIG_TSEC2_NAME       "eTSEC2"
#define CONFIG_TSEC3    1
#define CONFIG_TSEC3_NAME       "eTSEC3"
#define CONFIG_TSEC4    1
#define CONFIG_TSEC4_NAME       "eTSEC4"

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC3_PHY_ADDR		2
#define TSEC4_PHY_ADDR		3
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0
#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC4_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define CONFIG_ETHPRIME		"eTSEC1"

#endif	/* CONFIG_TSEC_ENET */

/*
 * BAT0         2G     Cacheable, non-guarded
 * 0x0000_0000  2G     DDR
 */
#define CFG_DBAT0L      (BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_DBAT0U      (BATU_BL_2G | BATU_VS | BATU_VP)
#define CFG_IBAT0L      (BATL_PP_RW | BATL_MEMCOHERENCE )
#define CFG_IBAT0U      CFG_DBAT0U

/*
 * BAT1         1G     Cache-inhibited, guarded
 * 0x8000_0000  512M   PCI-Express 1 Memory
 * 0xa000_0000  512M   PCI-Express 2 Memory
 *	Changed it for operating from 0xd0000000
 */
#define CFG_DBAT1L      ( CFG_PCI1_MEM_PHYS | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT1U	(CFG_PCI1_MEM_PHYS | BATU_BL_1G | BATU_VS | BATU_VP)
#define CFG_IBAT1L	(CFG_PCI1_MEM_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT1U      CFG_DBAT1U

/*
 * BAT2         512M   Cache-inhibited, guarded
 * 0xc000_0000  512M   RapidIO Memory
 */
#define CFG_DBAT2L      (CFG_RIO_MEM_PHYS | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT2U	(CFG_RIO_MEM_PHYS | BATU_BL_512M | BATU_VS | BATU_VP)
#define CFG_IBAT2L	(CFG_RIO_MEM_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT2U      CFG_DBAT2U

/*
 * BAT3         4M     Cache-inhibited, guarded
 * 0xf800_0000  4M     CCSR
 */
#define CFG_DBAT3L      ( CFG_CCSRBAR | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U      (CFG_CCSRBAR | BATU_BL_4M | BATU_VS | BATU_VP)
#define CFG_IBAT3L      (CFG_CCSRBAR | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT3U      CFG_DBAT3U

/*
 * BAT4         32M    Cache-inhibited, guarded
 * 0xe200_0000  16M    PCI-Express 1 I/O
 * 0xe300_0000  16M    PCI-Express 2 I/0
 *    Note that this is at 0xe0000000
 */
#define CFG_DBAT4L      ( CFG_PCI1_IO_PHYS | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT4U	(CFG_PCI1_IO_PHYS | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_IBAT4L	(CFG_PCI1_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT4U      CFG_DBAT4U

/*
 * BAT5         128K   Cacheable, non-guarded
 * 0xe401_0000  128K   Init RAM for stack in the CPU DCache (no backing memory)
 */
#define CFG_DBAT5L      (CFG_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_DBAT5U      (CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_IBAT5L      CFG_DBAT5L
#define CFG_IBAT5U      CFG_DBAT5U

/*
 * BAT6         32M    Cache-inhibited, guarded
 * 0xfe00_0000  32M    FLASH
 */
#define CFG_DBAT6L      ((CFG_FLASH_BASE & 0xfe000000) | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT6U      ((CFG_FLASH_BASE & 0xfe000000) | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_IBAT6L      ((CFG_FLASH_BASE & 0xfe000000) | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_IBAT6U      CFG_DBAT6U

#define CFG_DBAT7L 0x00000000
#define CFG_DBAT7U 0x00000000
#define CFG_IBAT7L 0x00000000
#define CFG_IBAT7U 0x00000000

/*
 * Environment
 */
#ifndef CFG_RAMBOOT
    #define CFG_ENV_IS_IN_FLASH	1
    #define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x60000)
    #define CFG_ENV_SECT_SIZE		0x10000	/* 64K(one sector) for env */
    #define CFG_ENV_SIZE		0x2000
#else
    #define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
    #define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
    #define CFG_ENV_SIZE		0x2000
#endif

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

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
#endif

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
    #define CONFIG_CMD_SCSI
    #define CONFIG_CMD_EXT2
    #define CONFIG_CMD_USB
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
#define CONFIG_ETHADDR   00:E0:0C:00:00:01
#define CONFIG_ETH1ADDR  00:E0:0C:00:01:FD
#define CONFIG_ETH2ADDR  00:E0:0C:00:02:FD
#define CONFIG_ETH3ADDR  00:E0:0C:00:03:FD
#endif

#define CONFIG_HAS_ETH0		1
#define CONFIG_HAS_ETH1		1
#define CONFIG_HAS_ETH2		1
#define CONFIG_HAS_ETH3		1

#define CONFIG_IPADDR		192.168.1.100

#define CONFIG_HOSTNAME		unknown
#define CONFIG_ROOTPATH		/opt/nfsroot
#define CONFIG_BOOTFILE		uImage
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */

#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.255.0

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS				        \
   "netdev=eth0\0"                                                      \
   "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0" 				\
   "tftpflash=tftpboot $loadaddr $uboot; " 			\
	"protect off " MK_STR(TEXT_BASE) " +$filesize; " 	\
	"erase " MK_STR(TEXT_BASE) " +$filesize; " 		\
	"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; " 	\
	"protect on " MK_STR(TEXT_BASE) " +$filesize; " 	\
	"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0" 	\
   "consoledev=ttyS0\0"                                                 \
   "ramdiskaddr=2000000\0"						\
   "ramdiskfile=your.ramdisk.u-boot\0"                                  \
   "dtbaddr=c00000\0"						\
   "dtbfile=mpc8641_hpcn.dtb\0"                                  \
   "en-wd=mw.b f8100010 0x08; echo -expect:- 08; md.b f8100010 1\0" \
   "dis-wd=mw.b f8100010 0x00; echo -expect:- 00; md.b f8100010 1\0" \
   "maxcpus=2"


#define CONFIG_NFSBOOTCOMMAND	                                        \
   "setenv bootargs root=/dev/nfs rw "                                  \
      "nfsroot=$serverip:$rootpath "                                    \
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"                     \
   "tftp $loadaddr $bootfile;"                                          \
   "tftp $dtbaddr $dtbfile;"                                          \
   "bootm $loadaddr - $dtbaddr"

#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "                                  \
      "console=$consoledev,$baudrate $othbootargs;"                     \
   "tftp $ramdiskaddr $ramdiskfile;"                                    \
   "tftp $loadaddr $bootfile;"                                          \
   "tftp $dtbaddr $dtbfile;"                                          \
   "bootm $loadaddr $ramdiskaddr $dtbaddr"

#define CONFIG_BOOTCOMMAND  CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
