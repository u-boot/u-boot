/*
 * Copyright 2009 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * P1 P2 RDB board configuration file
 * This file is intended to address a set of Low End and Ultra Low End
 * Freescale SOCs of QorIQ series(RDB platforms).
 * Currently only P2020RDB
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_MK_P1011RDB
#define CONFIG_P1011
#endif
#ifdef CONFIG_MK_P1020RDB
#define CONFIG_P1020
#endif
#ifdef CONFIG_MK_P2010RDB
#define CONFIG_P2010
#endif
#ifdef CONFIG_MK_P2020RDB
#define CONFIG_P2020
#endif

#ifdef CONFIG_MK_NAND
#define CONFIG_NAND_U_BOOT		1
#define CONFIG_RAMBOOT_NAND		1
#define CONFIG_RAMBOOT_TEXT_BASE	0xf8f82000
#endif

#ifdef CONFIG_MK_SDCARD
#define CONFIG_RAMBOOT_SDCARD		1
#define CONFIG_RAMBOOT_TEXT_BASE	0xf8f80000
#endif

#ifdef CONFIG_MK_SPIFLASH
#define CONFIG_RAMBOOT_SPIFLASH		1
#define CONFIG_RAMBOOT_TEXT_BASE	0xf8f80000
#endif

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48/P1020/P2020,etc*/
#define CONFIG_FSL_ELBC		1	/* Enable eLBC Support */
#define CONFIG_PCI		1	/* Enable PCI/PCIE */
#define CONFIG_PCIE1		1	/* PCIE controler 1 (slot 1) */
#define CONFIG_PCIE2		1	/* PCIE controler 2 (slot 2) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_FSL_PCIE_RESET	1	/* need PCIe reset errata */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */
#define CONFIG_FSL_LAW		1	/* Use common FSL init code */
#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_E1000		1	/*  E1000 pci Ethernet card*/
#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif
#define CONFIG_DDR_CLK_FREQ	66666666 /* DDRCLK on P1_P2 RDB */
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0) /*sysclk for P1_P2 RDB */

#if defined(CONFIG_P2020) || defined(CONFIG_P1020)
#define CONFIG_MP
#endif

#define CONFIG_HWCONFIG

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

#define CONFIG_ADDR_STREAMING		/* toggle addr streaming */

#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x1fffffff
#define CONFIG_PANIC_HANG	/* do not reset board on panic */

 /*
  * Config the L2 Cache as L2 SRAM
  */
#define CONFIG_SYS_INIT_L2_ADDR		0xf8f80000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	0xff8f80000ull
#else
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#endif
#define CONFIG_SYS_L2_SIZE		(512 << 10)
#define CONFIG_SYS_INIT_L2_END		(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_CCSRBAR		0xffe00000	/* relocated CCSRBAR */
#define CONFIG_SYS_CCSRBAR_PHYS	CONFIG_SYS_CCSRBAR	/* physical addr of */
							/* CCSRBAR */
#define CONFIG_SYS_IMMR		CONFIG_SYS_CCSRBAR	/* PQII uses */
							/* CONFIG_SYS_IMMR */

#if defined(CONFIG_RAMBOOT_NAND) && !defined(CONFIG_NAND_SPL)
#define CONFIG_SYS_CCSRBAR_DEFAULT	CONFIG_SYS_CCSRBAR
#else
#define CONFIG_SYS_CCSRBAR_DEFAULT	0xff700000      /* CCSRBAR Default */
#endif

/* DDR Setup */
#define CONFIG_FSL_DDR2
#undef CONFIG_FSL_DDR_INTERACTIVE
#undef CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#undef CONFIG_DDR_DLL

#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_SDRAM_SIZE	1024 	/* DDR size on P1_P2 RDBs */
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1

#define CONFIG_SYS_DDR_ERR_INT_EN	0x0000000d
#define CONFIG_SYS_DDR_ERR_DIS		0x00000000
#define CONFIG_SYS_DDR_SBE		0x00FF0000

/*
 * Memory map
 *
 * 0x0000_0000	0x3fff_ffff	DDR			1G cacheablen
 * 0xa000_0000	0xbfff_ffff	PCI Express Mem		1G non-cacheable
 * 0xffc2_0000	0xffc5_ffff	PCI IO range		256K non-cacheable
 *
 * Localbus cacheable (TBD)
 * 0xXXXX_XXXX	0xXXXX_XXXX	SRAM			YZ M Cacheable
 *
 * Localbus non-cacheable
 * 0xef00_0000	0xefff_ffff	FLASH			16M non-cacheable
 * 0xffa0_0000	0xffaf_ffff	NAND			1M non-cacheable
 * 0xffb0_0000	0xffbf_ffff	VSC7385 switch		1M non-cacheable
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

/*
 * Local Bus Definitions
 */
#define CONFIG_SYS_FLASH_BASE		0xef000000	/* start of FLASH 16M */

#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE

#define CONFIG_FLASH_BR_PRELIM	(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | \
					BR_PS_16 | BR_V)
#define CONFIG_FLASH_OR_PRELIM		0xff000ff7

#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS 45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#if defined(CONFIG_SYS_SPL) || defined(CONFIG_RAMBOOT_NAND) \
	|| defined(CONFIG_RAMBOOT_SDCARD) || defined(CONFIG_RAMBOOT_SPIFLASH)
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_AMD_CHECK_DQ7

#define CONFIG_BOARD_EARLY_INIT_R	/* call board_early_init_r function */
#define CONFIG_HWCONFIG

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR      0xffd00000	/* stack in RAM */
#define CONFIG_SYS_INIT_RAM_END	0x00004000	/* End of used area in RAM */

#define CONFIG_SYS_GBL_DATA_SIZE	128	/* num bytes initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END \
						- CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon*/
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc*/

#ifndef CONFIG_NAND_SPL
#define CONFIG_SYS_NAND_BASE		0xffa00000
#else
#define CONFIG_SYS_NAND_BASE		0xfff00000
#endif
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_NAND_BASE_LIST	{CONFIG_SYS_NAND_BASE}
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS			1
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_CMD_NAND			1
#define CONFIG_NAND_FSL_ELBC		1
#define CONFIG_SYS_NAND_BLOCK_SIZE	(16 * 1024)

/* NAND boot: 4K NAND loader config */
#define CONFIG_SYS_NAND_SPL_SIZE	0x1000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	((512 << 10) - 0x2000)
#define CONFIG_SYS_NAND_U_BOOT_DST	(CONFIG_SYS_INIT_L2_ADDR)
#define CONFIG_SYS_NAND_U_BOOT_START	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_NAND_SPL_SIZE)
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(0)
#define CONFIG_SYS_NAND_U_BOOT_RELOC	(CONFIG_SYS_INIT_L2_END - 0x2000)
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP		((CONFIG_SYS_INIT_L2_END - 1) & ~0xF)

/* NAND flash config */
#define CONFIG_NAND_BR_PRELIM	(CONFIG_SYS_NAND_BASE_PHYS \
				| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
				| BR_PS_8	/* Port Size = 8 bit */ \
				| BR_MS_FCM		/* MSEL = FCM */ \
				| BR_V)			/* valid */

#define CONFIG_NAND_OR_PRELIM	(0xFFF80000		/* length 32K */ \
				| OR_FCM_CSCT \
				| OR_FCM_CST \
				| OR_FCM_CHT \
				| OR_FCM_SCY_1 \
				| OR_FCM_TRLX \
				| OR_FCM_EHTR)

#ifdef CONFIG_RAMBOOT_NAND
#define CONFIG_SYS_BR0_PRELIM  CONFIG_NAND_BR_PRELIM  /* NAND Base Address */
#define CONFIG_SYS_OR0_PRELIM  CONFIG_NAND_OR_PRELIM  /* NAND Options */
#define CONFIG_SYS_BR1_PRELIM  CONFIG_FLASH_BR_PRELIM  /* NOR Base Address */
#define CONFIG_SYS_OR1_PRELIM  CONFIG_FLASH_OR_PRELIM  /* NOR Options */
#else
#define CONFIG_SYS_BR0_PRELIM  CONFIG_FLASH_BR_PRELIM  /* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM  CONFIG_FLASH_OR_PRELIM  /* NOR Options */
#define CONFIG_SYS_BR1_PRELIM  CONFIG_NAND_BR_PRELIM  /* NAND Base Address */
#define CONFIG_SYS_OR1_PRELIM  CONFIG_NAND_OR_PRELIM  /* NAND Options */
#endif

#define CONFIG_SYS_VSC7385_BASE	0xffb00000

#define CONFIG_SYS_VSC7385_BASE_PHYS	CONFIG_SYS_VSC7385_BASE

#define CONFIG_SYS_BR2_PRELIM	(CONFIG_SYS_VSC7385_BASE | BR_PS_8 | BR_V)
#define CONFIG_SYS_OR2_PRELIM	(OR_AM_128KB | OR_GPCM_CSNT | OR_GPCM_XACS | \
				OR_GPCM_SCY_15 | OR_GPCM_SETA | OR_GPCM_TRLX | \
				OR_GPCM_EHTR | OR_GPCM_EAD)

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#ifdef CONFIG_NAND_SPL
#define CONFIG_NS16550_MIN_FUNCTIONS
#endif

#define CONFIG_SERIAL_MULTI	1 /* Enable both serial ports */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	/* determine from environment */

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif

/*
 * Pass open firmware flat tree
 */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/* new uImage format support */
#define CONFIG_FIT		1
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */

/* I2C */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address*/
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_NOPROBES	{{0,0x29}}	/* Don't probe these addrs */
#define CONFIG_SYS_I2C_OFFSET		0x3000
#define CONFIG_SYS_I2C2_OFFSET		0x3100

/*
 * I2C2 EEPROM
 */
#define CONFIG_ID_EEPROM
#ifdef CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#endif
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_BUS_NUM	1

#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR                0x68
/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 2, Slot 2, tgtid 2, Base address 9000 */
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc20000
#define CONFIG_SYS_PCIE2_IO_BUS	0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc20000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 1, Slot 1, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc30000
#define CONFIG_SYS_PCIE1_IO_BUS	0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc30000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

#if defined(CONFIG_PCI)
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


#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_DOS_PARTITION

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

#define TSEC1_PHY_ADDR		2
#define TSEC2_PHY_ADDR		0
#define TSEC3_PHY_ADDR		1

#define CONFIG_VSC7385_ENET

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET
/* The size of the VSC7385 firmware image */
#define CONFIG_VSC7385_IMAGE_SIZE	8192
#endif

#define CONFIG_ETHPRIME		"eTSEC1"

#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

/* TBI PHY configuration for SGMII mode */
#define CONFIG_TSEC_TBICR_SETTINGS ( \
		TBICR_PHY_RESET \
		| TBICR_ANEG_ENABLE \
		| TBICR_FULL_DUPLEX \
		| TBICR_SPEED1_SET \
		)

#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#if defined(CONFIG_SYS_RAMBOOT)
#if defined(CONFIG_RAMBOOT_NAND)
	#define CONFIG_ENV_IS_IN_NAND	1
	#define CONFIG_ENV_SIZE		CONFIG_SYS_NAND_BLOCK_SIZE
	#define CONFIG_ENV_OFFSET	((512 * 1024) + CONFIG_SYS_NAND_BLOCK_SIZE)
#elif defined(CONFIG_RAMBOOT_SDCARD) || defined(CONFIG_RAMBOOT_SPIFLASH)
	#define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
	#define CONFIG_ENV_SIZE		0x2000
#endif
#else
	#define CONFIG_ENV_IS_IN_FLASH	1
	#if CONFIG_SYS_MONITOR_BASE > 0xfff80000
	#define CONFIG_ENV_ADDR		0xfff80000
	#else
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
	#endif
	#define CONFIG_ENV_SIZE		0x2000
	#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#endif

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#define CONFIG_MMC	1

#ifdef CONFIG_MMC
#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */
#define CONFIG_CMD_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_FSL_ESDHC
#define CONFIG_GENERIC_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#ifdef CONFIG_P2020
#define CONFIG_SYS_FSL_ESDHC_USE_PIO /* P2020 eSDHC DMA is not functional*/
#endif
#endif

#define CONFIG_USB_EHCI

#ifdef CONFIG_USB_EHCI
#define CONFIG_CMD_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#define CONFIG_USB_STORAGE
#endif

#if defined(CONFIG_MMC) || defined(CONFIG_USB_EHCI)
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

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
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
						/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(16 << 20)/* Initial Memory map for Linux*/

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

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#endif

#define CONFIG_HOSTNAME		P2020RDB
#define CONFIG_ROOTPATH		/opt/nfsroot
#define CONFIG_BOOTFILE		uImage
#define CONFIG_UBOOTPATH	u-boot.bin/* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"netdev=eth0\0"						\
	"uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
	"loadaddr=1000000\0"			\
	"tftpflash=tftpboot $loadaddr $uboot; "			\
		"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
		"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
		"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
		"protect on " MK_STR(TEXT_BASE) " +$filesize; "		\
		"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
	"consoledev=ttyS0\0"				\
	"ramdiskaddr=2000000\0"			\
	"ramdiskfile=rootfs.ext2.gz.uboot\0"		\
	"fdtaddr=c00000\0"				\
	"fdtfile=p2020rdb.dtb\0"		\
	"bdev=sda1\0"	\
	"jffs2nor=mtdblock3\0"	\
	"norbootaddr=ef080000\0"	\
	"norfdtaddr=ef040000\0"	\
	"jffs2nand=mtdblock9\0"	\
	"nandbootaddr=100000\0"	\
	"nandfdtaddr=80000\0"		\
	"nandimgsize=400000\0"		\
	"nandfdtsize=80000\0"		\
	"usb_phy_type=ulpi\0"		\
	"vscfw_addr=ef000000\0"	\
	"othbootargs=ramdisk_size=600000\0" \
	"usbfatboot=setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"usb start;"			\
	"fatload usb 0:2 $loadaddr $bootfile;"		\
	"fatload usb 0:2 $fdtaddr $fdtfile;"	\
	"fatload usb 0:2 $ramdiskaddr $ramdiskfile;"	\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\
	"usbext2boot=setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"usb start;"			\
	"ext2load usb 0:4 $loadaddr $bootfile;"		\
	"ext2load usb 0:4 $fdtaddr $fdtfile;"	\
	"ext2load usb 0:4 $ramdiskaddr $ramdiskfile;"	\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\
	"norboot=setenv bootargs root=/dev/$jffs2nor rw "	\
	"console=$consoledev,$baudrate rootfstype=jffs2 $othbootargs;"	\
	"bootm $norbootaddr - $norfdtaddr\0"		\
	"nandboot=setenv bootargs root=/dev/$jffs2nand rw rootfstype=jffs2 " \
	"console=$consoledev,$baudrate $othbootargs;"	\
	"nand read 2000000 $nandbootaddr $nandimgsize;"	\
	"nand read 3000000 $nandfdtaddr $nandfdtsize;"	\
	"bootm 2000000 - 3000000;\0"

#define CONFIG_NFSBOOTCOMMAND		\
	"setenv bootargs root=/dev/nfs rw "	\
	"nfsroot=$serverip:$rootpath "		\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftp $loadaddr $bootfile;"		\
	"tftp $fdtaddr $fdtfile;"		\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_HDBOOT			\
	"setenv bootargs root=/dev/$bdev rw rootdelay=30 "	\
	"console=$consoledev,$baudrate $othbootargs;"	\
	"usb start;"			\
	"ext2load usb 0:1 $loadaddr /boot/$bootfile;"		\
	"ext2load usb 0:1 $fdtaddr /boot/$fdtfile;"	\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND		\
	"setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"tftp $ramdiskaddr $ramdiskfile;"	\
	"tftp $loadaddr $bootfile;"		\
	"tftp $fdtaddr $fdtfile;"		\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_HDBOOT

#endif	/* __CONFIG_H */
