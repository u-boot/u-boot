/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Kevin Lam <kevin.lam@freescale.com>
 * Joe D'Abbraccio <joe.d'abbraccio@freescale.com>
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

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */
#define CONFIG_MPC83XX		1 /* MPC83XX family */
#define CONFIG_MPC837X		1 /* MPC837X CPU specific */
#define CONFIG_MPC837XERDB	1

#define CONFIG_PCI	1

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_MISC_INIT_R

/*
 * On-board devices
 */
#define CONFIG_TSEC_ENET		/* TSEC Ethernet support */
#define CONFIG_VSC7385_ENET

/*
 * System Clock Setup
 */
#ifdef CONFIG_PCISLAVE
#define CONFIG_83XX_PCICLK	66666667 /* in HZ */
#else
#define CONFIG_83XX_CLKIN	66666667 /* in Hz */
#define CONFIG_83XX_GENERIC_PCI	1
#endif

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	CONFIG_83XX_CLKIN
#endif

/*
 * Hardware Reset Configuration Word
 */
#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_SVCOD_DIV_2 |\
	HRCWL_CSB_TO_CLKIN_5X1 |\
	HRCWL_CORE_TO_CSB_2X1)

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

/* System performance - define the value i.e. CFG_XXX
*/

/* Arbiter Configuration Register */
#define CFG_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CFG_ACR_RPTCNT		3	/* Arbiter repeat count (0-7) */

/* System Priority Control Regsiter */
#define CFG_SPCR_TSECEP		3	/* eTSEC1&2 emergency priority (0-3) */

/* System Clock Configuration Register */
#define CFG_SCCR_TSEC1CM	1		/* eTSEC1 clock mode (0-3) */
#define CFG_SCCR_TSEC2CM	1		/* eTSEC2 clock mode (0-3) */
#define CFG_SCCR_SATACM		SCCR_SATACM_2	/* SATA1-4 clock mode (0-3) */

/*
 * System IO Config
 */
#define CFG_SICRH		0x08200000
#define CFG_SICRL		0x00000000

/*
 * Output Buffer Impedance
 */
#define CFG_OBIR		0x30100000

/*
 * IMMR new address
 */
#define CFG_IMMR		0xE0000000

/*
 * Device configurations
 */

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

#define CONFIG_TSEC2

/* The flash address and size of the VSC7385 firmware image */
#define CONFIG_VSC7385_IMAGE		0xFE7FE000
#define CONFIG_VSC7385_IMAGE_SIZE	8192

#endif

/*
 * DDR Setup
 */
#define CFG_DDR_BASE		0x00000000 /* DDR is system memory */
#define CFG_SDRAM_BASE		CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE	CFG_DDR_BASE
#define CFG_DDR_SDRAM_CLK_CNTL	0x03000000
#define CFG_83XX_DDR_USES_CS0

#define CFG_DDRCDR_VALUE	(DDRCDR_DHC_EN | DDRCDR_ODT | DDRCDR_Q_DRN)

#undef CONFIG_DDR_ECC		/* support DDR ECC function */
#undef CONFIG_DDR_ECC_CMD	/* Use DDR ECC user commands */

#undef CONFIG_NEVER_ASSERT_ODT_TO_CPU	/* Never assert ODT to internal IOs */

/*
 * Manually set up DDR parameters
 */
#define CFG_DDR_SIZE		256		/* MB */
#define CFG_DDR_CS0_BNDS	0x0000000f
#define CFG_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_ODT_WR_ACS \
				| CSCONFIG_ROW_BIT_13 | CSCONFIG_COL_BIT_10)

#define CFG_DDR_TIMING_3	0x00000000
#define CFG_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (6 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00220802 */
				/* 0x00260802 */ /* DDR400 */
#define CFG_DDR_TIMING_1	((3 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (9 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (3 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (7 << TIMING_CFG1_CASLAT_SHIFT) \
				| (13 << TIMING_CFG1_REFREC_SHIFT) \
				| (3 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x3935d322 */
				/* 0x3937d322 */
#define CFG_DDR_TIMING_2	0x02984cc8

#define CFG_DDR_INTERVAL	((1545 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (256 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x06090100 */

#if defined(CONFIG_DDR_2T_TIMING)
#define CFG_DDR_SDRAM_CFG		(SDRAM_CFG_SREN \
				| 3 << SDRAM_CFG_SDRAM_TYPE_SHIFT \
				| SDRAM_CFG_2T_EN \
				| SDRAM_CFG_DBW_32)
#else
#define CFG_DDR_SDRAM_CFG		(SDRAM_CFG_SREN \
				| 3 << SDRAM_CFG_SDRAM_TYPE_SHIFT)
				/* 0x43000000 */
#endif
#define CFG_DDR_SDRAM_CFG2	0x00001000 /* 1 posted refresh */
#define CFG_DDR_MODE		((0x0440 << SDRAM_MODE_ESD_SHIFT) \
				| (0x0442 << SDRAM_MODE_SD_SHIFT))
				/* 0x04400442 */ /* DDR400 */
#define CFG_DDR_MODE2		0x00000000;

/*
 * Memory test
 */
#undef CFG_DRAM_TEST		/* memory test, takes time */
#define CFG_MEMTEST_START	0x00040000 /* memtest region */
#define CFG_MEMTEST_END		0x0ef70010

/*
 * The reserved memory
 */
#define CFG_MONITOR_BASE	TEXT_BASE /* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef	CFG_RAMBOOT
#endif

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
#define CFG_FLASH_SIZE		8 /* max FLASH size is 32M */

#define CFG_FLASH_EMPTY_INFO			/* display empty sectors */
#define CFG_FLASH_USE_BUFFER_WRITE		/* buffer up multiple bytes */

#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE /* Window base at flash base */
#define CFG_LBLAWAR0_PRELIM	0x80000016	/* 8 MB window size */

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | /* Flash Base address */ \
				(2 << BR_PS_SHIFT) | /* 16 bit port size */ \
				BR_V) /* valid */
#define CFG_OR0_PRELIM		(0xFF800000		/* 8 MByte */ \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_9 \
				| OR_GPCM_EHTR \
				| OR_GPCM_EAD)
				/* 0xFF806FF7	TODO SLOW 8 MB flash size */

#define CFG_MAX_FLASH_BANKS	1 /* number of banks */
#define CFG_MAX_FLASH_SECT	256 /* max sectors per device */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

/*
 * NAND Flash on the Local Bus
 */
#define CFG_NAND_BASE		0xE0600000	/* 0xE0600000 */
#define CFG_BR1_PRELIM		(CFG_NAND_BASE | \
				 (2 << BR_DECC_SHIFT) |	/* Use HW ECC */ \
				 BR_PS_8 |		/* Port Size = 8 bit */ \
				 BR_MS_FCM |		/* MSEL = FCM */ \
				 BR_V)			/* valid */
#define CFG_OR1_PRELIM		(0xFFFF8000 |		/* length 32K */ \
				 OR_FCM_CSCT | \
				 OR_FCM_CST | \
				 OR_FCM_CHT | \
				 OR_FCM_SCY_1 | \
				 OR_FCM_TRLX | \
				 OR_FCM_EHTR)
#define CFG_LBLAWBAR1_PRELIM	CFG_NAND_BASE
#define CFG_LBLAWAR1_PRELIM	0x8000000E	/* 32KB  */

/* Vitesse 7385 */

#define CFG_VSC7385_BASE	0xF0000000

#ifdef CONFIG_VSC7385_ENET

#define CFG_BR2_PRELIM		0xf0000801		/* Base address */
#define CFG_OR2_PRELIM		0xfffe09ff		/* 128K bytes*/
#define CFG_LBLAWBAR2_PRELIM	CFG_VSC7385_BASE	/* Access Base */
#define CFG_LBLAWAR2_PRELIM	0x80000010		/* Access Size 128K */

#endif

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CFG_NS16550_COM1	(CFG_IMMR+0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR+0x4600)

/* SERDES */
#define CONFIG_FSL_SERDES
#define CONFIG_FSL_SERDES1	0xe3000
#define CONFIG_FSL_SERDES2	0xe3100

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS 1

/* I2C */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
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
#define CFG_PCI_IO_BASE		0x00000000
#define CFG_PCI_IO_PHYS		0xE0300000
#define CFG_PCI_IO_SIZE		0x100000 /* 1M */

#define CFG_PCI_SLV_MEM_LOCAL	CFG_SDRAM_BASE
#define CFG_PCI_SLV_MEM_BUS	0x00000000
#define CFG_PCI_SLV_MEM_SIZE	0x80000000

#ifdef CONFIG_PCI
#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CFG_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */
#endif	/* CONFIG_PCI */

/*
 * TSEC
 */
#ifdef CONFIG_TSEC_ENET

#define CONFIG_NET_MULTI
#define CONFIG_GMII			/* MII PHY management */

#define CONFIG_TSEC1

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME		"TSEC0"
#define CFG_TSEC1_OFFSET		0x24000
#define TSEC1_PHY_ADDR			2
#define TSEC1_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC1_PHYIDX			0
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME		"TSEC1"
#define CFG_TSEC2_OFFSET		0x25000
#define TSEC2_PHY_ADDR			0x1c
#define TSEC2_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_PHYIDX			0
#endif

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME			"TSEC0"

#endif

/*
 * SATA
 */
#define CONFIG_LIBATA
#define CONFIG_FSL_SATA

#define CFG_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CFG_SATA1_OFFSET	0x18000
#define CFG_SATA1		(CFG_IMMR + CFG_SATA1_OFFSET)
#define CFG_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CFG_SATA2_OFFSET	0x19000
#define CFG_SATA2		(CFG_IMMR + CFG_SATA2_OFFSET)
#define CFG_SATA2_FLAGS		FLAGS_DMA

#ifdef CONFIG_FSL_SATA
#define CONFIG_LBA48
#define CONFIG_CMD_SATA
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT2
#endif

/*
 * Environment
 */
#ifndef CFG_RAMBOOT
	#define CFG_ENV_IS_IN_FLASH	1
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE+CFG_MONITOR_LEN)
	#define CFG_ENV_SECT_SIZE	0x10000	/* 64K (one sector) for env */
	#define CFG_ENV_SIZE		0x4000
#else
	#define CFG_NO_FLASH		1	/* Flash is not usable now */
	#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-0x1000)
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

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

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

/* L2 Switch: cache-inhibit and guarded */
#define CFG_IBAT3L	(CFG_VSC7385_BASE | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT3U	(CFG_VSC7385_BASE | BATU_BL_128K | BATU_VS | BATU_VP)
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

#ifdef CONFIG_HAS_ETH0
#define CONFIG_ETHADDR		00:04:9f:ef:04:01
#endif

#ifdef CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:04:9f:ef:04:02
#endif

#define CONFIG_HAS_FSL_DR_USB

#define CONFIG_IPADDR		10.0.0.2
#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_GATEWAYIP	10.0.0.1
#define CONFIG_NETMASK		255.0.0.0
#define CONFIG_NETDEV		eth1

#define CONFIG_HOSTNAME		mpc837x_rdb
#define CONFIG_ROOTPATH		/nfsroot
#define CONFIG_RAMDISKFILE	rootfs.ext2.gz.uboot
#define CONFIG_BOOTFILE		uImage
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */
#define CONFIG_FDTFILE		mpc8379_rdb.dtb

#define CONFIG_LOADADDR		500000	/* default location for tftp and bootm */
#define CONFIG_BOOTDELAY	-1	/* -1 disables auto-boot */
#define CONFIG_BAUDRATE		115200

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" MK_STR(CONFIG_NETDEV) "\0"				\
	"uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
	"tftpflash=tftp $loadaddr $uboot;"				\
		"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
		"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
		"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
		"protect on " MK_STR(TEXT_BASE) " +$filesize; "		\
		"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
	"fdtaddr=400000\0"						\
	"fdtfile=" MK_STR(CONFIG_FDTFILE) "\0"				\
	"ramdiskaddr=1000000\0"						\
	"ramdiskfile=" MK_STR(CONFIG_RAMDISKFILE) "\0"			\
	"console=ttyS0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath "	\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv rootdev /dev/nfs;"					\
	"run setbootargs;"						\
	"run setipargs;"						\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv rootdev /dev/ram;"					\
	"run setbootargs;"						\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#undef MK_STR
#undef XMK_STR

#endif	/* __CONFIG_H */
