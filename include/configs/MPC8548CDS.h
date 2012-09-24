/*
 * Copyright 2004, 2007, 2010-2011 Freescale Semiconductor.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * mpc8548cds board configuration file
 *
 * Please refer to doc/README.mpc85xxcds for more info.
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_36BIT
#define CONFIG_PHYS_64BIT
#endif

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8548		1	/* MPC8548 specific */
#define CONFIG_MPC8548CDS	1	/* MPC8548CDS board specific */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xfff80000
#endif

#define CONFIG_SYS_SRIO
#define CONFIG_SRIO1			/* SRIO port 1 */

#define CONFIG_PCI		/* enable any pci type devices */
#define CONFIG_PCI1		/* PCI controller 1 */
#define CONFIG_PCIE1		/* PCIE controler 1 (slot 1) */
#undef CONFIG_PCI2
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_FSL_PCIE_RESET	1	/* need PCIe reset errata */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */

#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */
#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

#define CONFIG_FSL_VIA

#ifndef __ASSEMBLY__
extern unsigned long get_clock_freq(void);
#endif
#define CONFIG_SYS_CLK_FREQ	get_clock_freq() /* sysclk for MPC85xx */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP		16	/* number of TLB1 entries */
#endif

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000

#define CONFIG_SYS_CCSRBAR		0xe0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_FSL_DDR2
#undef CONFIG_FSL_DDR_INTERACTIVE
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/
#define CONFIG_DDR_SPD

#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x51	/* CTLR 0 DIMM 0 */

/* Make sure required options are set */
#ifndef CONFIG_SPD_EEPROM
#error ("CONFIG_SPD_EEPROM is required")
#endif

#undef CONFIG_CLOCKS_IN_MHZ
/*
 * Physical Address Map
 *
 * 32bit:
 * 0x0000_0000	0x7fff_ffff	DDR			2G	cacheable
 * 0x8000_0000	0x9fff_ffff	PCI1 MEM		512M	cacheable
 * 0xa000_0000	0xbfff_ffff	PCIe MEM		512M	cacheable
 * 0xc000_0000	0xdfff_ffff	RapidIO			512M	cacheable
 * 0xe000_0000	0xe00f_ffff	CCSR			1M	non-cacheable
 * 0xe200_0000	0xe20f_ffff	PCI1 IO			1M	non-cacheable
 * 0xe300_0000	0xe30f_ffff	PCIe IO			1M	non-cacheable
 * 0xf000_0000	0xf3ff_ffff	SDRAM			64M	cacheable
 * 0xf800_0000	0xf80f_ffff	NVRAM/CADMUS		1M	non-cacheable
 * 0xff00_0000	0xff7f_ffff	FLASH (2nd bank)	8M	non-cacheable
 * 0xff80_0000	0xffff_ffff	FLASH (boot bank)	8M	non-cacheable
 *
 * 36bit:
 * 0x00000_0000	0x07fff_ffff	DDR			2G	cacheable
 * 0xc0000_0000	0xc1fff_ffff	PCI1 MEM		512M	cacheable
 * 0xc2000_0000	0xc3fff_ffff	PCIe MEM		512M	cacheable
 * 0xc4000_0000	0xc5fff_ffff	RapidIO			512M	cacheable
 * 0xfe000_0000	0xfe00f_ffff	CCSR			1M	non-cacheable
 * 0xfe200_0000	0xfe20f_ffff	PCI1 IO			1M	non-cacheable
 * 0xfe300_0000	0xfe30f_ffff	PCIe IO			1M	non-cacheable
 * 0xff000_0000	0xff3ff_ffff	SDRAM			64M	cacheable
 * 0xff800_0000	0xff80f_ffff	NVRAM/CADMUS		1M	non-cacheable
 * 0xfff00_0000	0xfff7f_ffff	FLASH (2nd bank)	8M	non-cacheable
 * 0xfff80_0000	0xfffff_ffff	FLASH (boot bank)	8M	non-cacheable
 *
 */


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
 * 0	4    8	  12   16   20	 24   28
 * 1111 1111 1000 0000 0001 0000 0000 0001 = ff801001	 BR0
 * 1111 1111 0000 0000 0001 0000 0000 0001 = ff001001	 BR1
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
 * 0	4    8	  12   16   20	 24   28
 * 1111 1111 1000 0000 0110 1110 0110 0101 = ff806e65	 ORx
 */

#define CONFIG_SYS_FLASH_BASE		0xff000000	/* start of FLASH 16M */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_FLASH_BASE_PHYS	0xfff000000ull
#else
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE
#endif

#define CONFIG_SYS_BR0_PRELIM \
	(BR_PHYS_ADDR((CONFIG_SYS_FLASH_BASE_PHYS + 0x800000)) \
	| BR_PS_16 | BR_V)
#define CONFIG_SYS_BR1_PRELIM \
	(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | BR_PS_16 | BR_V)

#define	CONFIG_SYS_OR0_PRELIM		0xff806e65
#define	CONFIG_SYS_OR1_PRELIM		0xff806e65

#define CONFIG_SYS_FLASH_BANKS_LIST \
	{CONFIG_SYS_FLASH_BASE_PHYS + 0x800000, CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	128		/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_HWCONFIG			/* enable hwconfig */

/*
 * SDRAM on the Local Bus
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_LBC_SDRAM_BASE_PHYS	0xff0000000ull
#else
#define CONFIG_SYS_LBC_SDRAM_BASE_PHYS	CONFIG_SYS_LBC_SDRAM_BASE
#endif
#define CONFIG_SYS_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

/*
 * Base Register 2 and Option Register 2 configure SDRAM.
 * The SDRAM base address, CONFIG_SYS_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR2, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f0001861
 *
 * FIXME: CONFIG_SYS_LBC_SDRAM_BASE should be masked and OR'ed into
 * FIXME: the top 17 bits of BR2.
 */

#define CONFIG_SYS_BR2_PRELIM \
	(BR_PHYS_ADDR(CONFIG_SYS_LBC_SDRAM_BASE_PHYS) \
	| BR_PS_32 | (3<<BR_MSEL_SHIFT) | BR_V)

/*
 * The SDRAM size in MB, CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR2, need:
 *    64MB mask for AM, OR2[0:7] = 1111 1100
 *		   XAM, OR2[17:18] = 11
 *    9 columns OR2[19-21] = 010
 *    13 rows	OR2[23-25] = 100
 *    EAD set for extra time OR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1100 0000 0000 0110 1001 0000 0001 = fc006901
 */

#define CONFIG_SYS_OR2_PRELIM		0xfc006901

#define CONFIG_SYS_LBC_LCRR		0x00030004	/* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR		0x00000000	/* LB config reg */
#define CONFIG_SYS_LBC_LSRT		0x20000000	/* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR		0x00000000	/* LB refresh timer prescal*/

/*
 * Common settings for all Local Bus SDRAM commands.
 * At run time, either BSMA1516 (for CPU 1.1)
 *		    or BSMA1617 (for CPU 1.0) (old)
 * is OR'ed in too.
 */
#define CONFIG_SYS_LBC_LSDMR_COMMON	( LSDMR_RFCR16		\
				| LSDMR_PRETOACT7	\
				| LSDMR_ACTTORW7	\
				| LSDMR_BL8		\
				| LSDMR_WRC4		\
				| LSDMR_CL3		\
				| LSDMR_RFEN		\
				)

/*
 * The CADMUS registers are connected to CS3 on CDS.
 * The new memory map places CADMUS at 0xf8000000.
 *
 * For BR3, need:
 *    Base address of 0xf8000000 = BR[0:16] = 1111 1000 0000 0000 0
 *    port-size = 8-bits  = BR[19:20] = 01
 *    no parity checking  = BR[21:22] = 00
 *    GPMC for MSEL	  = BR[24:26] = 000
 *    Valid		  = BR[31]    = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1000 0000 0000 0000 1000 0000 0001 = f8000801
 *
 * For OR3, need:
 *    1 MB mask for AM,	  OR[0:16]  = 1111 1111 1111 0000 0
 *    disable buffer ctrl OR[19]    = 0
 *    CSNT		  OR[20]    = 1
 *    ACS		  OR[21:22] = 11
 *    XACS		  OR[23]    = 1
 *    SCY 15 wait states  OR[24:27] = 1111	max is suboptimal but safe
 *    SETA		  OR[28]    = 0
 *    TRLX		  OR[29]    = 1
 *    EHTR		  OR[30]    = 1
 *    EAD extra time	  OR[31]    = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1111 1111 0000 0000 1111 1111 0111 = fff00ff7
 */

#define CONFIG_FSL_CADMUS

#define CADMUS_BASE_ADDR 0xf8000000
#ifdef CONFIG_PHYS_64BIT
#define CADMUS_BASE_ADDR_PHYS	0xff8000000ull
#else
#define CADMUS_BASE_ADDR_PHYS	CADMUS_BASE_ADDR
#endif
#define CONFIG_SYS_BR3_PRELIM \
	(BR_PHYS_ADDR(CADMUS_BASE_ADDR_PHYS) | BR_PS_8 | BR_V)
#define CONFIG_SYS_OR3_PRELIM	 0xfff00ff7

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(1024 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX	2
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_NOPROBES	{0x69}	/* Don't probe these addrs */
#define CONFIG_SYS_I2C_OFFSET		0x3000

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_CCID
#define CONFIG_SYS_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR     0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCI1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCI1_MEM_BUS		0xe0000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCI1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_VIRT	0xe2000000
#define CONFIG_SYS_PCI1_IO_BUS	0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCI1_IO_PHYS 0xfe2000000ull
#else
#define CONFIG_SYS_PCI1_IO_PHYS	0xe2000000
#endif
#define CONFIG_SYS_PCI1_IO_SIZE	0x00100000	/* 1M */

#ifdef CONFIG_PCIE1
#define CONFIG_SYS_PCIE1_NAME		"Slot"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xe3000000
#define CONFIG_SYS_PCIE1_IO_BUS	0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS        0xfe3000000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe3000000
#endif
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00100000	/*   1M */
#endif

/*
 * RapidIO MMU
 */
#define CONFIG_SYS_SRIO1_MEM_VIRT	0xc0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_SRIO1_MEM_PHYS	0xc40000000ull
#else
#define CONFIG_SYS_SRIO1_MEM_PHYS	0xc0000000
#endif
#define CONFIG_SYS_SRIO1_MEM_SIZE	0x20000000	/* 512M */

#ifdef CONFIG_LEGACY
#define BRIDGE_ID 17
#define VIA_ID 2
#else
#define BRIDGE_ID 28
#define VIA_ID 4
#endif

#if defined(CONFIG_PCI)

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP
#define CONFIG_E1000			/* Define e1000 pci Ethernet card */

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif	/* CONFIG_PCI */


#if defined(CONFIG_TSEC_ENET)

#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC1"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC2"
#define CONFIG_TSEC4
#define CONFIG_TSEC4_NAME	"eTSEC3"
#undef CONFIG_MPC85XX_FEC

#define CONFIG_PHY_MARVELL

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC3_PHY_ADDR		2
#define TSEC4_PHY_ADDR		3

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC4_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

/* Options are: eTSEC[0-3] */
#define CONFIG_ETHPRIME		"eTSEC0"
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#if CONFIG_SYS_MONITOR_BASE > 0xfff80000
#define CONFIG_ENV_ADDR	0xfff80000
#else
#define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#endif
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K for env */
#define CONFIG_ENV_SIZE		0x2000

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
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_CMDLINE_EDITING			/* Command-line editing */
#define CONFIG_AUTO_COMPLETE			/* add autocompletion support */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
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

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

/* The mac addresses for all ethernet interface */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR	 00:E0:0C:00:00:FD
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR	 00:E0:0C:00:01:FD
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR	 00:E0:0C:00:02:FD
#define CONFIG_HAS_ETH3
#define CONFIG_ETH3ADDR	 00:E0:0C:00:03:FD
#endif

#define CONFIG_IPADDR	 192.168.1.253

#define CONFIG_HOSTNAME	 unknown
#define CONFIG_ROOTPATH	 "/nfsroot"
#define CONFIG_BOOTFILE "8548cds/uImage.uboot"
#define CONFIG_UBOOTPATH	8548cds/u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP	 192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_NETMASK	 255.255.255.0

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs*/

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:ecc=off\0"		\
	"netdev=eth0\0"				\
	"uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"	\
	"tftpflash=tftpboot $loadaddr $uboot; "	\
		"protect off " MK_STR(CONFIG_SYS_TEXT_BASE) " +$filesize; "   \
		"erase " MK_STR(CONFIG_SYS_TEXT_BASE) " +$filesize; "	      \
		"cp.b $loadaddr " MK_STR(CONFIG_SYS_TEXT_BASE) " $filesize; " \
		"protect on " MK_STR(CONFIG_SYS_TEXT_BASE) " +$filesize; "    \
		"cmp.b $loadaddr " MK_STR(CONFIG_SYS_TEXT_BASE) " $filesize\0"\
	"consoledev=ttyS1\0"			\
	"ramdiskaddr=2000000\0"			\
	"ramdiskfile=ramdisk.uboot\0"		\
	"fdtaddr=c00000\0"			\
	"fdtfile=mpc8548cds.dtb\0"

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr - $fdtaddr"


#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND	CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
