/*
 * Copyright 2004, 2007 Freescale Semiconductor.
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

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8548		1	/* MPC8548 specific */
#define CONFIG_MPC8548CDS	1	/* MPC8548CDS board specific */

#define CONFIG_PCI		/* enable any pci type devices */
#define CONFIG_PCI1		/* PCI controller 1 */
#define CONFIG_PCIE1		/* PCIE controler 1 (slot 1) */
#undef CONFIG_RIO
#undef CONFIG_PCI2
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */

#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/
#define CONFIG_DDR_DLL			/* possible DLL fix needed */
#undef CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */

#define CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef
#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */


/*
 * When initializing flash, if we cannot find the manufacturer ID,
 * assume this is the AMD flash associated with the CDS board.
 * This allows booting from a promjet.
 */
#define CONFIG_ASSUME_AMD_FLASH

#define MPC85xx_DDR_SDRAM_CLK_CNTL	/* 85xx has clock control reg */

#ifndef __ASSEMBLY__
extern unsigned long get_clock_freq(void);
#endif
#define CONFIG_SYS_CLK_FREQ	get_clock_freq() /* sysclk for MPC85xx */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */
#define CONFIG_ADDR_STREAMING		/* toggle addr streaming */
#define CONFIG_CLEAR_LAW0		/* Clear LAW0 in cpu_init_r */

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
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

#define CFG_PCI1_ADDR	(CFG_CCSRBAR+0x8000)
#define CFG_PCI2_ADDR	(CFG_CCSRBAR+0x9000)
#define CFG_PCIE1_ADDR	(CFG_CCSRBAR+0xa000)

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

#define CFG_BOOT_BLOCK		0xff000000	/* boot TLB block */
#define CFG_FLASH_BASE		CFG_BOOT_BLOCK	/* start of FLASH 16M */

#define CFG_BR0_PRELIM		0xff801001
#define CFG_BR1_PRELIM		0xff001001

#define	CFG_OR0_PRELIM		0xff806e65
#define	CFG_OR1_PRELIM		0xff806e65

#define CFG_FLASH_BANKS_LIST	{0xff800000, CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS	2		/* number of banks */
#define CFG_MAX_FLASH_SECT	128		/* sectors per device */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO


/*
 * SDRAM on the Local Bus
 */
#define CFG_LBC_CACHE_BASE	0xf0000000	/* Localbus cacheable */
#define CFG_LBC_CACHE_SIZE	64
#define CFG_LBC_NONCACHE_BASE	0xf8000000	/* Localbus non-cacheable */
#define CFG_LBC_NONCACHE_SIZE	64

#define CFG_LBC_SDRAM_BASE	CFG_LBC_CACHE_BASE	/* Localbus SDRAM */
#define CFG_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

/*
 * Base Register 2 and Option Register 2 configure SDRAM.
 * The SDRAM base address, CFG_LBC_SDRAM_BASE, is 0xf0000000.
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
 * FIXME: CFG_LBC_SDRAM_BASE should be masked and OR'ed into
 * FIXME: the top 17 bits of BR2.
 */

#define CFG_BR2_PRELIM		0xf0001861

/*
 * The SDRAM size in MB, CFG_LBC_SDRAM_SIZE, is 64.
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

#define CFG_OR2_PRELIM		0xfc006901

#define CFG_LBC_LCRR		0x00030004	/* LB clock ratio reg */
#define CFG_LBC_LBCR		0x00000000	/* LB config reg */
#define CFG_LBC_LSRT		0x20000000	/* LB sdram refresh timer */
#define CFG_LBC_MRTPR		0x00000000	/* LB refresh timer prescal*/

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
 *		    or BSMA1617 (for CPU 1.0) (old)
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

#define CADMUS_BASE_ADDR 0xf8000000
#define CFG_BR3_PRELIM	 0xf8000801
#define CFG_OR3_PRELIM	 0xfff00ff7

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CFG_INIT_RAM_END	0x4000		/* End of used area in RAM */

#define CFG_INIT_L2_ADDR	0xf8f80000	/* relocate boot L2SRAM */

#define CFG_GBL_DATA_SIZE	128		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX	2
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1	(CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2	(CFG_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_FLAT_TREE	1
#define CONFIG_OF_BOARD_SETUP	1

/* maximum size of the flat tree (8K) */
#define OF_FLAT_TREE_MAX_SIZE	8192

#define OF_CPU			"PowerPC,8548@0"
#define OF_SOC			"soc8548@e0000000"
#define OF_TBCLK		(bd->bi_busfreq / 8)
#define OF_STDOUT_PATH		"/soc8548@e0000000/serial@4600"

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_EEPROM_ADDR	0x57
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CFG_PCI_PHYS		0x80000000	/* 1G PCI TLB */

#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xe2000000
#define CFG_PCI1_IO_SIZE	0x00100000	/* 1M */

#ifdef CONFIG_PCI2
#define CFG_PCI2_MEM_BASE	0xa0000000
#define CFG_PCI2_MEM_PHYS	CFG_PCI2_MEM_BASE
#define CFG_PCI2_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI2_IO_BASE	0x00000000
#define CFG_PCI2_IO_PHYS	0xe2800000
#define CFG_PCI2_IO_SIZE	0x00100000	/* 1M */
#endif

#ifdef CONFIG_PCIE1
#define CFG_PCIE1_MEM_BASE	0xa0000000
#define CFG_PCIE1_MEM_PHYS	CFG_PCIE1_MEM_BASE
#define CFG_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCIE1_IO_BASE	0x00000000
#define CFG_PCIE1_IO_PHYS	0xe3000000
#define CFG_PCIE1_IO_SIZE	0x00100000	/*   1M */
#endif

#ifdef CONFIG_RIO
/*
 * RapidIO MMU
 */
#define CFG_RIO_MEM_BASE	0xC0000000
#define CFG_RIO_MEM_SIZE	0x20000000	/* 512M */
#endif

#ifdef CONFIG_LEGACY
#define BRIDGE_ID 17
#define VIA_ID 2
#else
#define BRIDGE_ID 28
#define VIA_ID 4
#endif

#if defined(CONFIG_PCI)

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS	0x00000000
#define CFG_PCI_MEMORY_PHYS	0x00000000
#define CFG_PCI_MEMORY_SIZE	0x80000000

#endif	/* CONFIG_PCI */


#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

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

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC3_PHY_ADDR		2
#define TSEC4_PHY_ADDR		3

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0

/* Options are: eTSEC[0-3] */
#define CONFIG_ETHPRIME		"eTSEC0"
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
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
#if defined(CONFIG_TSEC_ENET)
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
#define CONFIG_ROOTPATH	 /nfsroot
#define CONFIG_BOOTFILE	8548cds/uImage.uboot
#define CONFIG_UBOOTPATH	8548cds/u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP	 192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_NETMASK	 255.255.255.0

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs*/

#define CONFIG_BAUDRATE	115200

#if defined(CONFIG_PCIE1)
#define PCIE_ENV \
 "pciereg=md ${a}000 6; md ${a}020 4; md ${a}bf8 2; echo o;md ${a}c00 25;" \
	"echo i; md ${a}da0 15; echo e;md ${a}e00 e; echo d; md ${a}f00 c\0" \
 "pcieerr=md ${a}020 1; md ${a}e00 e; pci d.b $b.0 7 1; pci d.w $b.0 1e 1;" \
	"pci d.w $b.0 56 1; pci d $b.0 104 1; pci d $b.0 110 1;" \
	"pci d $b.0 130 1\0" \
 "pcieerrc=mw ${a}020 ffffffff; mw ${a}e00 ffffffff; pci w.b $b.0 7 ff;" \
	"pci w.w $b.0 1e ffff; pci w.w $b.0 56 ffff; pci w $b.0 104 ffffffff;"\
	"pci w $b.0 110 ffffffff; pci w $b.0 130 ffffffff\0" \
 "pciecfg=pci d $b.0 0 20; pci d $b.0 100 e; pci d $b.0 400 69\0" \
 "pcie1regs=setenv a e000a; run pciereg\0" \
 "pcie1cfg=setenv b 3; run pciecfg\0" \
 "pcie1err=setenv a e000a; setenv b 3; run pcieerr\0" \
 "pcie1errc=setenv a e000a; setenv b 3; run pcieerrc\0"
#else
#define	PCIE_ENV ""
#endif

#if defined(CONFIG_PCI1) || defined(CONFIG_PCI2)
#define PCI_ENV \
 "pcireg=md ${a}000 3; echo o;md ${a}c00 25; echo i; md ${a}da0 15;" \
	"echo e;md ${a}e00 9\0" \
 "pcierr=md ${a}e00 8; pci d.b $b.0 7 1;pci d.w $b.0 1e 1;" \
	"pci d.w $b.0 56 1\0" \
 "pcierrc=mw ${a}e00 ffffffff; mw ${a}e0c 0; pci w.b $b.0 7 ff;" \
	"pci w.w $b.0 1e ffff; pci w.w $b.0 56 ffff\0"
#else
#define	PCI_ENV ""
#endif

#if defined(CONFIG_PCI1)
#define PCI_ENV1 \
 "pci1regs=setenv a e0008; run pcireg\0" \
 "pci1err=setenv a e0008; setenv b 0; run pcierr\0" \
 "pci1errc=setenv a e0008; setenv b 0; run pcierrc\0"
#else
#define	PCI_ENV1 ""
#endif

#if defined(CONFIG_PCI2)
#define PCI_ENV2 \
 "pci2regs=setenv a e0009; run pcireg\0" \
 "pci2err=setenv a e0009; setenv b 123; run pcierr\0"	\
 "pci2errc=setenv a e0009; setenv b 123; run pcierrc\0"
#else
#define	PCI_ENV2 ""
#endif

#if defined(CONFIG_TSEC_ENET)
#define ENET_ENV \
 "enetreg1=md ${a}000 2; md ${a}010 9; md ${a}050 4; md ${a}08c 1;" \
	"md ${a}098 2\0" \
 "enetregt=echo t;md ${a}100 6; md ${a}140 2; md ${a}180 10; md ${a}200 10\0" \
 "enetregr=echo r;md ${a}300 6; md ${a}330 5; md ${a}380 10; md ${a}400 10\0" \
 "enetregm=echo mac;md ${a}500 5; md ${a}520 28;echo fifo;md ${a}a00 1;" \
	"echo mib;md ${a}680 31\0" \
 "enetreg=run enetreg1; run enetregm; run enetregt; run enetregr\0" \
 "enet1regs=setenv a e0024; run enetreg\0" \
 "enet2regs=setenv a e0025; run enetreg\0" \
 "enet3regs=setenv a e0026; run enetreg\0" \
 "enet4regs=setenv a e0027; run enetreg\0"
#else
#define ENET_ENV ""
#endif

#if 0
#define	CONFIG_EXTRA_ENV_SETTINGS				\
 "netdev=eth0\0"						\
 "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
 "tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
	"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
	"protect on " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
 "consoledev=ttyS1\0"				\
 "ramdiskaddr=2000000\0"			\
 "ramdiskfile=ramdisk.uboot\0"			\
 "dtbaddr=c00000\0"				\
 "dtbfile=mpc8548cds.dtb\0"			\
 "eoi=mw e00400b0 0\0"				\
 "iack=md e00400a0 1\0"				\
 "ddrreg=md ${a}000 8; md ${a}080 8;md ${a}100 d; md ${a}140 4; md ${a}bf0 4;" \
	"md ${a}e00 3; md ${a}e20 3; md ${a}e40 7; md ${a}f00 5\0" \
 "ddrregs=setenv a e0002; run ddrreg\0"		\
 "gureg=md ${a}000 2c; md ${a}0b0 1; md ${a}0c0 1; md ${a}b20 3;" \
	"md ${a}e00 1; md ${a}e60 1; md ${a}ef0 15\0"	\
 "guregs=setenv a e00e0; run gureg\0"		\
 "ecmreg=md ${a}000 1; md ${a}010 1; md ${a}bf8 2; md ${a}e00 6\0" \
 "ecmregs=setenv a e0001; run ecmreg\0" \
 "lawregs=md e0000c08 4b\0" \
 "lbcregs=md e0005000 36\0" \
 "dma0regs=md e0021100 12\0" \
 "dma1regs=md e0021180 12\0" \
 "dma2regs=md e0021200 12\0" \
 "dma3regs=md e0021280 12\0" \
 PCIE_ENV \
 PCI_ENV \
 PCI_ENV1 \
 PCI_ENV2 \
 ENET_ENV
#endif


#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $dtbaddr $dtbfile;"						\
   "bootm $loadaddr - $dtbaddr"


#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $dtbaddr $dtbfile;"						\
   "bootm $loadaddr $ramdiskaddr $dtbaddr"

#define CONFIG_BOOTCOMMAND	CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
