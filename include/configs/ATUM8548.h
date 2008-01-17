/*
 * Copyright 2007
 * Robert Lazarski, Instituto Atlantico, robertlazarski@gmail.com
 *
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
 * atum8548 board configuration file
 *
 * Please refer to doc/README.atum8548 for more info.
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* Debug Options, Disable in production
#define ET_DEBUG		1
#define CONFIG_PANIC_HANG	1
#define DEBUG			1
*/

/* CPLD Configuration Options */
#define MPC85xx_ATUM_CLKOCR            0x80000002

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8548		1	/* MPC8548 specific */

#define CONFIG_PCI		1	/* enable any pci type devices */
#define CONFIG_PCI1		1	/* PCI controller 1 */
#define CONFIG_PCIE1		1	/* PCIE controler 1 (slot 1) */
#define CONFIG_PCI2             1	/* PCI controller 2 */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */

#define CONFIG_TSEC_ENET	1	/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SPD_EEPROM	1 	/* Use SPD EEPROM for DDR setup*/
#undef CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */

#define CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef
#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */
#define CONFIG_FSL_INIT_TLBS	1	/* Use common FSL init code */

#define MPC85xx_DDR_SDRAM_CLK_CNTL	/* 85xx has clock control reg */

#define CONFIG_SYS_CLK_FREQ	33000000

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

#define CONFIG_CMD_SDRAM 		1	/* SDRAM DIMM SPD info printout */
#define CONFIG_ENABLE_36BIT_PHYS	1
#undef	CFG_DRAM_TEST
#define CFG_MEMTEST_START	0x00200000	/* memtest works on */
#define CFG_MEMTEST_END		0x00400000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

#define PCI_SPEED		33333000        /* CPLD currenlty does not have PCI setup info */
#define CFG_PCI1_ADDR	(CFG_CCSRBAR+0x8000)
#define CFG_PCI2_ADDR	(CFG_CCSRBAR+0x9000)
#define CFG_PCIE1_ADDR	(CFG_CCSRBAR+0xa000)

/*
 * DDR Setup
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE

#if defined(CONFIG_SPD_EEPROM)
    /*
     * Determine DDR configuration from I2C interface.
     */
    #define SPD_EEPROM_ADDRESS	0x51		/* DDR DIMM */

#else
    /*
     * Manually set up DDR parameters
     */
    #define CFG_SDRAM_SIZE	1024		/* DDR is 1024MB */
    #define CFG_DDR_CS0_BNDS	0x0000000f	/* 0-1024 */
    #define CFG_DDR_CS0_CONFIG	0x80000102
    #define CFG_DDR_TIMING_0	0x00260802
    #define CFG_DDR_TIMING_1	0x38355322
    #define CFG_DDR_TIMING_2	0x039048c7
    #define CFG_DDR_CONTROL	0xc2000000	/* unbuffered,no DYN_PWR */
    #define CFG_DDR_MODE	0x00000432
    #define CFG_DDR_INTERVAL	0x05150100
    #define DDR_SDRAM_CFG	0x43000000
#endif

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Local Bus Definitions
 */

/*
 * FLASH on the Local Bus
 * based on flash chip S29GL01GP
 * One bank, 128M, using the CFI driver.
 * Boot from BR0 bank at 0xf800_0000
 *
 * BR0:
 *    Base address 0 = 0xF8000000 = BR0[0:16] = 1111 1000 0000 0000 0
 *    Port Size = 16 bits = BRx[19:20] = 10
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1000 0000 0000 0001 0000 0000 0001 = f8001001    BR0
 *
 * OR0:
 *    Addr Mask = 128M = ORx[0:16] = 1111 1000 0000 0000 0
 *    Reserved ORx[17:18] = 00
 *    CSNT = ORx[20] = 1
 *    ACS = half cycle delay = ORx[21:22] = 11
 *    SCY = 6 = ORx[24:27] = 0110
 *    TRLX = use relaxed timing = ORx[29] = 1
 *    EAD = use external address latch delay = OR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1000 0000 0000 0000 1110 0110 0101 = f8000E65    ORx
 */

#define CFG_BOOT_BLOCK		0xf8000000	/* boot TLB block */
#define CFG_FLASH_BASE		CFG_BOOT_BLOCK	/* start of FLASH 128M */

#define CFG_BR0_PRELIM		0xf8001001

#define	CFG_OR0_PRELIM		0xf8000E65

#define CFG_MAX_FLASH_BANKS	1		/* number of banks	*/
#define CFG_MAX_FLASH_SECT	1024		/* sectors per device */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	512000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	8000	/* Flash Write Timeout (ms) */


#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#define CFG_FLASH_CFI_DRIVER    1
#define CFG_FLASH_CFI           1
#define CFG_FLASH_EMPTY_INFO

/*
 * Flash on the LocalBus
 */
#define CFG_LBC_CACHE_BASE	0xf0000000	/* Localbus cacheable	 */

/* Memory */
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
#define CONFIG_CONS_INDEX	1
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
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1

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
#define CFG_PCI2_MEM_BASE	0xC0000000
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


#if !defined(CONFIG_PCI_PNP)
    #define PCI_ENET0_IOADDR	0xe0000000
    #define PCI_ENET0_MEMADDR	0xe0000000
    #define PCI_IDSEL_NUMBER	0x0c 	/* slot0->3(IDSEL)=12->15 */
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
#define CONFIG_TSEC4	1
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
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC3_FLAGS		TSEC_GIGABIT
#define TSEC4_FLAGS		TSEC_GIGABIT

/* Options are: eTSEC[0-3] */
#define CONFIG_ETHPRIME		"eTSEC2"
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
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR	 00:E0:0C:00:00:FD
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR	 00:E0:0C:00:01:FD
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR	 00:E0:0C:00:02:FD
#define CONFIG_HAS_ETH3
#define CONFIG_ETH3ADDR	 00:E0:0C:00:03:FD
#endif

#define CONFIG_IPADDR	 10.101.43.142

#define CONFIG_HOSTNAME	 atum
#define CONFIG_ROOTPATH	 /nfsroot
#define CONFIG_BOOTFILE	 /tftpboot/uImage.atum
#define CONFIG_UBOOTPATH	/tftpboot/uboot.bin	/* TFTP server */

#define CONFIG_SERVERIP	 10.101.43.10
#define CONFIG_GATEWAYIP 10.101.45.1
#define CONFIG_NETMASK	 255.255.248.0

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs*/

#define CONFIG_BAUDRATE	115200

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"	                \
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
