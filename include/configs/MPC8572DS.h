/*
 * Copyright 2007-2008 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * mpc8572ds board configuration file
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8572		1
#define CONFIG_MPC8572DS	1
#define CONFIG_MP		1	/* support multiple processors */
#define CONFIG_NUM_CPUS		2	/* Number of CPUs in the system */

#define CONFIG_PCI		1	/* Enable PCI/PCIE */
#define CONFIG_PCIE1		1	/* PCIE controler 1 (slot 1) */
#define CONFIG_PCIE2		1	/* PCIE controler 2 (slot 2) */
#define CONFIG_PCIE3		1	/* PCIE controler 3 (ULI bridge) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_FSL_PCIE_RESET	1	/* need PCIe reset errata */

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

/*
 * When initializing flash, if we cannot find the manufacturer ID,
 * assume this is the AMD flash associated with the CDS board.
 * This allows booting from a promjet.
 */
#define CONFIG_ASSUME_AMD_FLASH

#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
extern unsigned long get_board_ddr_clk(unsigned long dummy);
#endif
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0) /* sysclk for MPC85xx */
#define CONFIG_DDR_CLK_FREQ	get_board_ddr_clk(0) /* ddrclk for MPC85xx */
#define CONFIG_ICS307_REFCLK_HZ	33333333  /* ICS307 clock chip ref freq */
#define CONFIG_GET_CLK_FROM_ICS307	  /* decode sysclk and ddrclk freq
					     from ICS307 instead of switches */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */
#define CONFIG_ADDR_STREAMING		/* toggle addr streaming */

#define CONFIG_ENABLE_36BIT_PHYS	1

#define CFG_MEMTEST_START	0x00000000	/* memtest works on */
#define CFG_MEMTEST_END		0x7fffffff
#define CONFIG_PANIC_HANG	/* do not reset board on panic */

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xffe00000	/* relocated CCSRBAR */
#define CFG_CCSRBAR_PHYS	CFG_CCSRBAR	/* physical addr of CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

#define CFG_PCIE3_ADDR		(CFG_CCSRBAR+0x8000)
#define CFG_PCIE2_ADDR		(CFG_CCSRBAR+0x9000)
#define CFG_PCIE1_ADDR		(CFG_CCSRBAR+0xa000)

/* DDR Setup */
#define CONFIG_FSL_DDR2
#undef CONFIG_FSL_DDR_INTERACTIVE
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#define CONFIG_DDR_SPD
#undef CONFIG_DDR_DLL

#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CFG_DDR_SDRAM_BASE	0x00000000
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS	2
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

/* I2C addresses of SPD EEPROMs */
#define CFG_SPD_BUS_NUM		1	/* SPD EEPROMS locate on I2C bus 1 */
#define SPD_EEPROM_ADDRESS1	0x51	/* CTLR 0 DIMM 0 */
#define SPD_EEPROM_ADDRESS2	0x52	/* CTLR 1 DIMM 0 */

/* These are used when DDR doesn't use SPD.  */
#define CFG_SDRAM_SIZE		256		/* DDR is 256MB */
#define CFG_DDR_CS0_BNDS	0x0000001F
#define CFG_DDR_CS0_CONFIG	0x80010102	/* Enable, no interleaving */
#define CFG_DDR_TIMING_3	0x00000000
#define CFG_DDR_TIMING_0	0x00260802
#define CFG_DDR_TIMING_1	0x3935d322
#define CFG_DDR_TIMING_2	0x14904cc8
#define CFG_DDR_MODE_1		0x00480432
#define CFG_DDR_MODE_2		0x00000000
#define CFG_DDR_INTERVAL	0x06180100
#define CFG_DDR_DATA_INIT	0xdeadbeef
#define CFG_DDR_CLK_CTRL	0x03800000
#define CFG_DDR_OCD_CTRL	0x00000000
#define CFG_DDR_OCD_STATUS	0x00000000
#define CFG_DDR_CONTROL		0xC3008000	/* Type = DDR2 */
#define CFG_DDR_CONTROL2	0x04400010

#define CFG_DDR_ERR_INT_EN	0x0000000d
#define CFG_DDR_ERR_DIS		0x00000000
#define CFG_DDR_SBE		0x00010000

/*
 * FIXME: Not used in fixed_sdram function
 */
#define CFG_DDR_MODE		0x00000022
#define CFG_DDR_CS1_BNDS	0x00000000
#define CFG_DDR_CS2_BNDS	0x00000FFF	/* Not done */
#define CFG_DDR_CS3_BNDS	0x00000FFF	/* Not done */
#define CFG_DDR_CS4_BNDS	0x00000FFF	/* Not done */
#define CFG_DDR_CS5_BNDS	0x00000FFF	/* Not done */

/*
 * Make sure required options are set
 */
#ifndef CONFIG_SPD_EEPROM
#error ("CONFIG_SPD_EEPROM is required")
#endif

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Memory map
 *
 * 0x0000_0000	0x7fff_ffff	DDR			2G Cacheable
 * 0x8000_0000	0xbfff_ffff	PCI Express Mem		1G non-cacheable
 * 0xc000_0000	0xdfff_ffff	PCI			512M non-cacheable
 * 0xe100_0000	0xe3ff_ffff	PCI IO range		4M non-cacheable
 *
 * Localbus cacheable (TBD)
 * 0xXXXX_XXXX	0xXXXX_XXXX	SRAM			YZ M Cacheable
 *
 * Localbus non-cacheable
 * 0xe000_0000	0xe80f_ffff	Promjet/free		128M non-cacheable
 * 0xe800_0000	0xefff_ffff	FLASH			128M non-cacheable
 * 0xffdf_0000	0xffdf_7fff	PIXIS			32K non-cacheable TLB0
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

/*
 * Local Bus Definitions
 */
#define CFG_FLASH_BASE		0xe0000000	/* start of FLASH 128M */

#define CFG_BR0_PRELIM		0xe8001001
#define CFG_OR0_PRELIM		0xf8000ff7

#define CFG_BR1_PRELIM		0xe0001001
#define CFG_OR1_PRELIM		0xf8000ff7

#define CFG_FLASH_BANKS_LIST	{CFG_FLASH_BASE + 0x8000000, CFG_FLASH_BASE}
#define CFG_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS 45 /* count down from 45/5: 9..1 */

#define CFG_MAX_FLASH_BANKS	2		/* number of banks */
#define CFG_MAX_FLASH_SECT	1024		/* sectors per device */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000		/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500		/* Flash Write Timeout (ms) */

#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#define CONFIG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_AMD_CHECK_DQ7

#define CONFIG_BOARD_EARLY_INIT_R	/* call board_early_init_r function */

#define CONFIG_FSL_PIXIS	1	/* use common PIXIS code */
#define PIXIS_BASE	0xffdf0000	/* PIXIS registers */

#define CFG_BR3_PRELIM	(PIXIS_BASE | 0x0801)	/* port size 8bit */
#define CFG_OR3_PRELIM		0xffffeff7	/* 32KB but only 4k mapped */

#define PIXIS_ID		0x0	/* Board ID at offset 0 */
#define PIXIS_VER		0x1	/* Board version at offset 1 */
#define PIXIS_PVER		0x2	/* PIXIS FPGA version at offset 2 */
#define PIXIS_CSR   		0x3	/* PIXIS General control/status register */
#define PIXIS_RST		0x4	/* PIXIS Reset Control register */
#define PIXIS_PWR		0x5	/* PIXIS Power status register */
#define PIXIS_AUX		0x6	/* Auxiliary 1 register */
#define PIXIS_SPD		0x7	/* Register for SYSCLK speed */
#define PIXIS_AUX2		0x8	/* Auxiliary 2 register */
#define PIXIS_VCTL		0x10	/* VELA Control Register */
#define PIXIS_VSTAT		0x11	/* VELA Status Register */
#define PIXIS_VCFGEN0		0x12	/* VELA Config Enable 0 */
#define PIXIS_VCFGEN1		0x13	/* VELA Config Enable 1 */
#define PIXIS_VCORE0	 	0x14	/* VELA VCORE0 Register */
#define PIXIS_VBOOT		0x16	/* VELA VBOOT Register */
#define PIXIS_VSPEED0		0x17	/* VELA VSpeed 0 */
#define PIXIS_VSPEED1		0x18	/* VELA VSpeed 1 */
#define PIXIS_VSPEED2		0x19	/* VELA VSpeed 2 */
#define PIXIS_VSYSCLK0		0x1C	/* VELA SYSCLK0 Register */
#define PIXIS_VSYSCLK1		0x1D	/* VELA SYSCLK1 Register */
#define PIXIS_VSYSCLK2		0x1E	/* VELA SYSCLK2 Register */
#define PIXIS_VDDRCLK0		0x1F	/* VELA DDRCLK0 Register */
#define PIXIS_VDDRCLK1		0x20	/* VELA DDRCLK1 Register */
#define PIXIS_VDDRCLK2		0x21	/* VELA DDRCLK2 Register */
#define PIXIS_VWATCH		0x24    /* Watchdog Register */
#define PIXIS_LED		0x25    /* LED Register */

/* old pixis referenced names */
#define PIXIS_VCLKH		0x19	/* VELA VCLKH register */
#define PIXIS_VCLKL		0x1A	/* VELA VCLKL register */
#define CFG_PIXIS_VBOOT_MASK	0xc0

/* define to use L1 as initial stack */
#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0xffd00000	/* Initial L1 address */
#define CFG_INIT_RAM_END	0x00004000	/* End of used area in RAM */

#define CFG_GBL_DATA_SIZE	128	/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc */

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1	(CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2	(CFG_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/*
 * Pass open firmware flat tree
 */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

#define CFG_64BIT_VSPRINTF	1
#define CFG_64BIT_STRTOUL	1

/* new uImage format support */
#define CONFIG_FIT		1
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */

/* I2C */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_EEPROM_ADDR	0x57
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{{0,0x29}}/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000
#define CFG_I2C2_OFFSET		0x3100

/*
 * I2C2 EEPROM
 */
#define CONFIG_ID_EEPROM
#ifdef CONFIG_ID_EEPROM
#define CFG_I2C_EEPROM_NXID
#endif
#define CFG_I2C_EEPROM_ADDR	0x57
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_BUS_NUM	1

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS	0x00000000
#define CFG_PCI_MEMORY_PHYS	0x00000000
#define CFG_PCI_MEMORY_SIZE	0x80000000

/* controller 3, direct to uli, tgtid 3, Base address 8000 */
#define CFG_PCIE3_MEM_BASE	0x80000000
#define CFG_PCIE3_MEM_PHYS	CFG_PCIE3_MEM_BASE
#define CFG_PCIE3_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCIE3_IO_BASE	0x00000000
#define CFG_PCIE3_IO_PHYS	0xffc00000
#define CFG_PCIE3_IO_SIZE	0x00010000	/* 64k */

/* controller 2, Slot 2, tgtid 2, Base address 9000 */
#define CFG_PCIE2_MEM_BASE	0xa0000000
#define CFG_PCIE2_MEM_PHYS	CFG_PCIE2_MEM_BASE
#define CFG_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCIE2_IO_BASE	0x00000000
#define CFG_PCIE2_IO_PHYS	0xffc10000
#define CFG_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 1, Slot 1, tgtid 1, Base address a000 */
#define CFG_PCIE1_MEM_BASE	0xc0000000
#define CFG_PCIE1_MEM_PHYS	CFG_PCIE1_MEM_BASE
#define CFG_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCIE1_IO_BASE	0x00000000
#define CFG_PCIE1_IO_PHYS	0xffc20000
#define CFG_PCIE1_IO_SIZE	0x00010000	/* 64k */

#if defined(CONFIG_PCI)

/*PCIE video card used*/
#define VIDEO_IO_OFFSET		CFG_PCIE1_IO_PHYS

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
#define CFG_ISA_IO_BASE_ADDRESS VIDEO_IO_OFFSET
#endif

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP
#undef CONFIG_RTL8139

#ifdef CONFIG_RTL8139
/* This macro is used by RTL8139 but not defined in PPC architecture */
#define KSEG1ADDR(x)		(x)
#define _IO_BASE	0x00000000
#endif

#ifndef CONFIG_PCI_PNP
	#define PCI_ENET0_IOADDR	CFG_PCIE3_IO_BASE
	#define PCI_ENET0_MEMADDR	CFG_PCIE3_IO_BASE
	#define PCI_IDSEL_NUMBER	0x11	/* IDSEL = AD11 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_DOS_PARTITION
#define CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CFG_SCSI_MAX_SCSI_ID	4
#define CFG_SCSI_MAX_LUN	1
#define CFG_SCSI_MAX_DEVICE	(CFG_SCSI_MAX_SCSI_ID * CFG_SCSI_MAX_LUN)
#define CFG_SCSI_MAXDEVICE	CFG_SCSI_MAX_DEVICE
#endif /* SCSI */

#endif	/* CONFIG_PCI */


#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_MII_DEFAULT_TSEC	1	/* Allow unregistered phys */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC3"
#define CONFIG_TSEC4	1
#define CONFIG_TSEC4_NAME	"eTSEC4"

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC3_PHY_ADDR		2
#define TSEC4_PHY_ADDR		3

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC4_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"

#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#if CFG_MONITOR_BASE > 0xfff80000
#define CONFIG_ENV_ADDR		0xfff80000
#else
#define CONFIG_ENV_ADDR		(CFG_MONITOR_BASE + 0x70000)
#endif
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_IRQ
#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_NET
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_EXT2
#endif

#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	*/
#define CONFIG_CMDLINE_EDITING		/* Command-line editing */
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
#define CONFIG_ETHADDR	00:E0:0C:02:00:FD
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR	00:E0:0C:02:01:FD
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR	00:E0:0C:02:02:FD
#define CONFIG_HAS_ETH3
#define CONFIG_ETH3ADDR	00:E0:0C:02:03:FD
#endif

#define CONFIG_IPADDR		192.168.1.254

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

#define	CONFIG_EXTRA_ENV_SETTINGS				\
 "netdev=eth0\0"						\
 "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
 "tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
	"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
	"protect on " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
 "consoledev=ttyS0\0"				\
 "ramdiskaddr=2000000\0"			\
 "ramdiskfile=8572ds/ramdisk.uboot\0"		\
 "fdtaddr=c00000\0"				\
 "fdtfile=8572ds/mpc8572ds.dtb\0"		\
 "bdev=sda3\0"

#define CONFIG_HDBOOT				\
 "setenv bootargs root=/dev/$bdev rw "		\
 "console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $loadaddr $bootfile;"			\
 "tftp $fdtaddr $fdtfile;"			\
 "bootm $loadaddr - $fdtaddr"

#define CONFIG_NFSBOOTCOMMAND		\
 "setenv bootargs root=/dev/nfs rw "	\
 "nfsroot=$serverip:$rootpath "		\
 "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
 "console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $loadaddr $bootfile;"		\
 "tftp $fdtaddr $fdtfile;"		\
 "bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND		\
 "setenv bootargs root=/dev/ram rw "	\
 "console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $ramdiskaddr $ramdiskfile;"	\
 "tftp $loadaddr $bootfile;"		\
 "tftp $fdtaddr $fdtfile;"		\
 "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_HDBOOT

#endif	/* __CONFIG_H */
