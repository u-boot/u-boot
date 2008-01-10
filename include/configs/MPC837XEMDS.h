/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Dave Liu <daveliu@freescale.com>
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

#ifndef __CONFIG_H
#define __CONFIG_H

#undef DEBUG

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */
#define CONFIG_MPC83XX		1 /* MPC83XX family */
#define CONFIG_MPC837X		1 /* MPC837X CPU specific */
#define CONFIG_MPC837XEMDS	1 /* MPC837XEMDS board specific */

/*
 * System Clock Setup
 */
#ifdef CONFIG_PCISLAVE
#define CONFIG_83XX_PCICLK	66000000 /* in HZ */
#else
#define CONFIG_83XX_CLKIN	66000000 /* in Hz */
#endif

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	66000000
#endif

/*
 * Hardware Reset Configuration Word
 * if CLKIN is 66MHz, then
 * CSB = 396MHz, CORE = 594MHz, DDRC = 396MHz, LBC = 396MHz
 */
#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_SVCOD_DIV_2 |\
	HRCWL_CSB_TO_CLKIN_6X1 |\
	HRCWL_CORE_TO_CSB_1_5X1)

#ifdef CONFIG_PCISLAVE
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_AGENT |\
	HRCWH_PCI1_ARBITER_DISABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0XFFF00100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_RL_EXT_LEGACY |\
	HRCWH_TSEC1M_IN_RGMII |\
	HRCWH_TSEC2M_IN_RGMII |\
	HRCWH_BIG_ENDIAN |\
	HRCWH_LDP_CLEAR)
#else
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_RL_EXT_LEGACY |\
	HRCWH_TSEC1M_IN_RGMII |\
	HRCWH_TSEC2M_IN_RGMII |\
	HRCWH_BIG_ENDIAN |\
	HRCWH_LDP_CLEAR)
#endif

/*
 * eTSEC Clock Config
 */
#define CFG_SCCR_TSEC1CM	1	/* CSB:eTSEC1 = 1:1 */
#define CFG_SCCR_TSEC2CM	1	/* CSB:eTSEC2 = 1:1 */

/*
 * System IO Config
 */
#define CFG_SICRH		0x00000000
#define CFG_SICRL		0x00000000

/*
 * Output Buffer Impedance
 */
#define CFG_OBIR		0x31100000

#define CONFIG_BOARD_EARLY_INIT_F /* call board_pre_init */
#define CONFIG_BOARD_EARLY_INIT_R

/*
 * IMMR new address
 */
#define CFG_IMMR		0xE0000000

/*
 * DDR Setup
 */
#define CFG_DDR_BASE		0x00000000 /* DDR is system memory */
#define CFG_SDRAM_BASE		CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE	CFG_DDR_BASE
#define CFG_DDR_SDRAM_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
#define CFG_83XX_DDR_USES_CS0
#define CFG_DDRCDR_VALUE	0x80080001 /* ODT 150ohm on SoC */

#undef CONFIG_DDR_ECC		/* support DDR ECC function */
#undef CONFIG_DDR_ECC_CMD	/* Use DDR ECC user commands */

#define CONFIG_SPD_EEPROM	/* Use SPD EEPROM for DDR setup */
#define CONFIG_NEVER_ASSERT_ODT_TO_CPU /* Never assert ODT to internal IOs */

#if defined(CONFIG_SPD_EEPROM)
#define SPD_EEPROM_ADDRESS	0x51 /* I2C address of DDR SODIMM SPD */
#else
/*
 * Manually set up DDR parameters
 * WHITE ELECTRONIC DESGGNS - W3HG64M72EEU403PD4 SO-DIMM
 * consist of nine chips from SAMSUNG K4T51083QE-ZC(L)D5
 */
#define CFG_DDR_SIZE		512 /* MB */
#define CFG_DDR_CS0_BNDS	0x0000001f
#define CFG_DDR_CS0_CONFIG	( CSCONFIG_EN \
				| 0x00010000  /* ODT_WR to CSn */ \
				| CSCONFIG_ROW_BIT_14 | CSCONFIG_COL_BIT_10 )
				/* 0x80010202 */
#define CFG_DDR_TIMING_3	0x00000000
#define CFG_DDR_TIMING_0	( ( 0 << TIMING_CFG0_RWT_SHIFT ) \
				| ( 0 << TIMING_CFG0_WRT_SHIFT ) \
				| ( 0 << TIMING_CFG0_RRT_SHIFT ) \
				| ( 0 << TIMING_CFG0_WWT_SHIFT ) \
				| ( 6 << TIMING_CFG0_ACT_PD_EXIT_SHIFT ) \
				| ( 2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT ) \
				| ( 8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT ) \
				| ( 2 << TIMING_CFG0_MRS_CYC_SHIFT ) )
				/* 0x00620802 */
#define CFG_DDR_TIMING_1	( ( 3 << TIMING_CFG1_PRETOACT_SHIFT ) \
				| ( 9 << TIMING_CFG1_ACTTOPRE_SHIFT ) \
				| ( 3 << TIMING_CFG1_ACTTORW_SHIFT ) \
				| ( 5 << TIMING_CFG1_CASLAT_SHIFT ) \
				| (13 << TIMING_CFG1_REFREC_SHIFT ) \
				| ( 3 << TIMING_CFG1_WRREC_SHIFT ) \
				| ( 2 << TIMING_CFG1_ACTTOACT_SHIFT ) \
				| ( 2 << TIMING_CFG1_WRTORD_SHIFT ) )
				/* 0x3935d322 */
#define CFG_DDR_TIMING_2	( ( 2 << TIMING_CFG2_ADD_LAT_SHIFT ) \
				| ( 6 << TIMING_CFG2_CPO_SHIFT ) \
				| ( 2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT ) \
				| ( 4 << TIMING_CFG2_RD_TO_PRE_SHIFT ) \
				| ( 2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT ) \
				| ( 3 << TIMING_CFG2_CKE_PLS_SHIFT ) \
				| ( 8 << TIMING_CFG2_FOUR_ACT_SHIFT) )
				/* 0x231088c8 */
#define CFG_DDR_INTERVAL	( ( 0x03E0 << SDRAM_INTERVAL_REFINT_SHIFT ) \
				| ( 0x0100 << SDRAM_INTERVAL_BSTOPRE_SHIFT ) )
				/* 0x03E00100 */
#define CFG_DDR_SDRAM_CFG	0x43000000
#define CFG_DDR_SDRAM_CFG2	0x00001000 /* 1 posted refresh */
#define CFG_DDR_MODE		( ( 0x0450 << SDRAM_MODE_ESD_SHIFT ) \
				| ( 0x1432 << SDRAM_MODE_SD_SHIFT ) )
				/* ODT 150ohm CL=3, AL=2 on SDRAM */
#define CFG_DDR_MODE2		0x00000000
#endif

/*
 * Memory test
 */
#undef CFG_DRAM_TEST		/* memory test, takes time */
#define CFG_MEMTEST_START	0x00040000 /* memtest region */
#define CFG_MEMTEST_END		0x00140000

/*
 * The reserved memory
 */
#define CFG_MONITOR_BASE	TEXT_BASE /* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef CFG_RAMBOOT
#endif

/* CFG_MONITOR_LEN must be a multiple of CFG_ENV_SECT_SIZE */
#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(512 * 1024) /* Reserved for malloc */

/*
 * Initial RAM Base Address Setup
 */
#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CFG_INIT_RAM_END	0x1000 /* End of used area in RAM */
#define CFG_GBL_DATA_SIZE	0x100 /* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

/*
 * Local Bus Configuration & Clock Setup
 */
#define CFG_LCRR		(LCRR_DBYP | LCRR_CLKDIV_8)
#define CFG_LBC_LBCR		0x00000000

/*
 * FLASH on the Local Bus
 */
#define CFG_FLASH_CFI		/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER	/* use the CFI driver */
#define CFG_FLASH_BASE		0xFE000000 /* FLASH base address */
#define CFG_FLASH_SIZE		32 /* max FLASH size is 32M */

#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE /* Window base at flash base */
#define CFG_LBLAWAR0_PRELIM	0x80000018 /* 32MB window size */

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | /* Flash Base address */ \
				(2 << BR_PS_SHIFT) | /* 16 bit port size */ \
				BR_V) /* valid */
#define CFG_OR0_PRELIM		((~(CFG_FLASH_SIZE - 1) << 20) | OR_UPM_XAM | \
				OR_GPCM_CSNT | OR_GPCM_ACS_0b11 | OR_GPCM_XACS | OR_GPCM_SCY_15 | \
				OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

#define CFG_MAX_FLASH_BANKS	1 /* number of banks */
#define CFG_MAX_FLASH_SECT	256 /* max sectors per device */

#undef CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000 /* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500 /* Flash Write Timeout (ms) */

/*
 * BCSR on the Local Bus
 */
#define CFG_BCSR		0xF8000000
#define CFG_LBLAWBAR1_PRELIM	CFG_BCSR /* Access window base at BCSR base */
#define CFG_LBLAWAR1_PRELIM	0x8000000E /* Access window size 32K */

#define CFG_BR1_PRELIM		(CFG_BCSR | 0x00000801) /* Port size=8bit, MSEL=GPCM */
#define CFG_OR1_PRELIM		0xFFFFE9f7 /* length 32K */

/*
 * NAND Flash on the Local Bus
 */
#define CFG_NAND_BASE		0xE0600000	/* 0xE0600000 */
#define CFG_BR3_PRELIM		( CFG_NAND_BASE \
				| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
				| BR_PS_8		/* Port Size = 8 bit */ \
				| BR_MS_FCM		/* MSEL = FCM */ \
				| BR_V )		/* valid */
#define CFG_OR3_PRELIM		( 0xFFFF8000		/* length 32K */ \
				| OR_FCM_CSCT \
				| OR_FCM_CST \
				| OR_FCM_CHT \
				| OR_FCM_SCY_1 \
				| OR_FCM_TRLX \
				| OR_FCM_EHTR )
				/* 0xFFFF8396 */

#define CFG_LBLAWBAR3_PRELIM	CFG_NAND_BASE
#define CFG_LBLAWAR3_PRELIM	0x8000000E	/* 32KB  */

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CFG_NS16550_COM1	(CFG_IMMR+0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/* I2C */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_FSL_I2C
#define CFG_I2C_SPEED		400000 /* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{0x51} /* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000
#define CFG_I2C2_OFFSET		0x3100

/*
 * Config on-board RTC
 */
#define CONFIG_RTC_DS1374	/* use ds1374 rtc via i2c */
#define CFG_I2C_RTC_ADDR	0x68 /* at address 0x68 */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CFG_PCI_MEM_BASE	0x80000000
#define CFG_PCI_MEM_PHYS	CFG_PCI_MEM_BASE
#define CFG_PCI_MEM_SIZE	0x10000000 /* 256M */
#define CFG_PCI_MMIO_BASE	0x90000000
#define CFG_PCI_MMIO_PHYS	CFG_PCI_MMIO_BASE
#define CFG_PCI_MMIO_SIZE	0x10000000 /* 256M */
#define CFG_PCI_IO_BASE		0xE0300000
#define CFG_PCI_IO_PHYS		0xE0300000
#define CFG_PCI_IO_SIZE		0x100000 /* 1M */

#define CFG_PCI_SLV_MEM_LOCAL	CFG_SDRAM_BASE
#define CFG_PCI_SLV_MEM_BUS	0x00000000
#define CFG_PCI_SLV_MEM_SIZE	0x80000000

#ifdef CONFIG_PCI
#define CONFIG_83XX_GENERIC_PCI	1 /* Use generic PCI setup */
#define CONFIG_PQ_MDS_PIB	1 /* PQ MDS Platform IO Board */

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CFG_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */
#endif /* CONFIG_PCI */

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

/*
 * TSEC
 */
#define CONFIG_TSEC_ENET	/* TSEC ethernet support */
#define CFG_TSEC1_OFFSET	0x24000
#define CFG_TSEC1		(CFG_IMMR+CFG_TSEC1_OFFSET)
#define CFG_TSEC2_OFFSET	0x25000
#define CFG_TSEC2		(CFG_IMMR+CFG_TSEC2_OFFSET)

/*
 * TSEC ethernet configuration
 */
#define CONFIG_MII		1 /* MII PHY management */
#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC1"
#define TSEC1_PHY_ADDR		2
#define TSEC2_PHY_ADDR		3
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"eTSEC1"

/*
 * Environment
 */
#ifndef CFG_RAMBOOT
	#define CFG_ENV_IS_IN_FLASH	1
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
	#define CFG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
	#define CFG_ENV_SIZE		0x2000
#else
	#define CFG_NO_FLASH		1	/* Flash is not usable now */
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
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#endif

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */

#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		/* undef to save memory */
#define CFG_LOAD_ADDR		0x2000000 /* default load address */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
	#define CFG_CBSIZE	1024 /* Console I/O Buffer Size */
#else
	#define CFG_CBSIZE	256 /* Console I/O Buffer Size */
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
#define CFG_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*
 * Core HID Setup
 */
#define CFG_HID0_INIT		0x000000000
#define CFG_HID0_FINAL		HID0_ENABLE_MACHINE_CHECK
#define CFG_HID2		HID2_HBE

/*
 * MMU Setup
 */

/* DDR: cache cacheable */
#define CFG_SDRAM_LOWER		CFG_SDRAM_BASE
#define CFG_SDRAM_UPPER		(CFG_SDRAM_BASE + 0x10000000)

#define CFG_IBAT0L	(CFG_SDRAM_LOWER | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_LOWER | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U

#define CFG_IBAT1L	(CFG_SDRAM_UPPER | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_SDRAM_UPPER | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U

/* IMMRBAR, PCI IO and NAND: cache-inhibit and guarded */
#define CFG_IBAT2L	(CFG_IMMR | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT2U	(CFG_IMMR | BATU_BL_8M | BATU_VS | BATU_VP)
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U

/* BCSR: cache-inhibit and guarded */
#define CFG_IBAT3L	(CFG_BCSR | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT3U	(CFG_BCSR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_DBAT3L	CFG_IBAT3L
#define CFG_DBAT3U	CFG_IBAT3U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CFG_IBAT4L	(CFG_FLASH_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT4U	(CFG_FLASH_BASE | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_DBAT4L	(CFG_FLASH_BASE | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT4U	CFG_IBAT4U

/* Stack in dcache: cacheable, no memory coherence */
#define CFG_IBAT5L	(CFG_INIT_RAM_ADDR | BATL_PP_10)
#define CFG_IBAT5U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_DBAT5L	CFG_IBAT5L
#define CFG_DBAT5U	CFG_IBAT5U

#ifdef CONFIG_PCI
/* PCI MEM space: cacheable */
#define CFG_IBAT6L	(CFG_PCI_MEM_PHYS | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT6U	(CFG_PCI_MEM_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
/* PCI MMIO space: cache-inhibit and guarded */
#define CFG_IBAT7L	(CFG_PCI_MMIO_PHYS | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT7U	(CFG_PCI_MMIO_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U
#else
#define CFG_IBAT6L	(0)
#define CFG_IBAT6U	(0)
#define CFG_IBAT7L	(0)
#define CFG_IBAT7U	(0)
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01 /* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02 /* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR		00:E0:0C:00:83:79
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:E0:0C:00:83:78
#endif

#define CONFIG_BAUDRATE 115200

#define CONFIG_LOADADDR 200000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY 6	/* -1 disables auto-boot */
#undef CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_EXTRA_ENV_SETTINGS					\
   "netdev=eth0\0"							\
   "consoledev=ttyS0\0"							\
   "ramdiskaddr=1000000\0"						\
   "ramdiskfile=ramfs.83xx\0"						\
   "fdtaddr=400000\0"							\
   "fdtfile=mpc837xemds.dtb\0"						\
   ""

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr $ramdiskaddr $fdtaddr"


#define CONFIG_BOOTCOMMAND CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
