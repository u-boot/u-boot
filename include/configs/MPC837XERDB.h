/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Kevin Lam <kevin.lam@freescale.com>
 * Joe D'Abbraccio <joe.d'abbraccio@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */
#define CONFIG_MPC837x		1 /* MPC837x CPU specific */
#define CONFIG_MPC837XERDB	1
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SYS_GENERIC_BOARD

#define	CONFIG_SYS_TEXT_BASE	0xFE000000

#define CONFIG_PCI	1

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_MISC_INIT_R
#define CONFIG_HWCONFIG

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
#define CONFIG_PCIE
#endif

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	CONFIG_83XX_CLKIN
#endif

/*
 * Hardware Reset Configuration Word
 */
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_SVCOD_DIV_2 |\
	HRCWL_CSB_TO_CLKIN_5X1 |\
	HRCWL_CORE_TO_CSB_2X1)

#ifdef CONFIG_PCISLAVE
#define CONFIG_SYS_HRCW_HIGH (\
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
#define CONFIG_SYS_HRCW_HIGH (\
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

/* System performance - define the value i.e. CONFIG_SYS_XXX
*/

/* Arbiter Configuration Register */
#define CONFIG_SYS_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CONFIG_SYS_ACR_RPTCNT	3	/* Arbiter repeat count (0-7) */

/* System Priority Control Regsiter */
#define CONFIG_SYS_SPCR_TSECEP	3	/* eTSEC1&2 emergency priority (0-3) */

/* System Clock Configuration Register */
#define CONFIG_SYS_SCCR_TSEC1CM	1		/* eTSEC1 clock mode (0-3) */
#define CONFIG_SYS_SCCR_TSEC2CM	1		/* eTSEC2 clock mode (0-3) */
#define CONFIG_SYS_SCCR_SATACM	SCCR_SATACM_2	/* SATA1-4 clock mode (0-3) */

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRH		0x08200000
#define CONFIG_SYS_SICRL		0x00000000

/*
 * Output Buffer Impedance
 */
#define CONFIG_SYS_OBIR		0x30100000

/*
 * IMMR new address
 */
#define CONFIG_SYS_IMMR		0xE0000000

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
#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	0x03000000
#define CONFIG_SYS_83XX_DDR_USES_CS0

#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_DHC_EN | DDRCDR_ODT | DDRCDR_Q_DRN)

#undef CONFIG_DDR_ECC		/* support DDR ECC function */
#undef CONFIG_DDR_ECC_CMD	/* Use DDR ECC user commands */

#undef CONFIG_NEVER_ASSERT_ODT_TO_CPU	/* Never assert ODT to internal IOs */

/*
 * Manually set up DDR parameters
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#define CONFIG_SYS_DDR_CS0_BNDS		0x0000000f
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_ODT_WR_ONLY_CURRENT \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)

#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (6 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00260802 */ /* DDR400 */
#define CONFIG_SYS_DDR_TIMING_1	((3 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (9 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (3 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (7 << TIMING_CFG1_CASLAT_SHIFT) \
				| (13 << TIMING_CFG1_REFREC_SHIFT) \
				| (3 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x3937d322 */
#define CONFIG_SYS_DDR_TIMING_2	((0 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (5 << TIMING_CFG2_CPO_SHIFT) \
				| (3 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (3 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (8 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x02984cc8 */

#define CONFIG_SYS_DDR_INTERVAL	((1024 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (0 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x06090100 */

#if defined(CONFIG_DDR_2T_TIMING)
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN \
					| SDRAM_CFG_SDRAM_TYPE_DDR2 \
					| SDRAM_CFG_32_BE \
					| SDRAM_CFG_2T_EN)
					/* 0x43088000 */
#else
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN \
					| SDRAM_CFG_SDRAM_TYPE_DDR2)
					/* 0x43000000 */
#endif
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00001000 /* 1 posted refresh */
#define CONFIG_SYS_DDR_MODE		((0x0406 << SDRAM_MODE_ESD_SHIFT) \
					| (0x0442 << SDRAM_MODE_SD_SHIFT))
					/* 0x04400442 */ /* DDR400 */
#define CONFIG_SYS_DDR_MODE2		0x00000000

/*
 * Memory test
 */
#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00040000 /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x0ef70010

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE /* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef	CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN	(384 * 1024) /* Reserve 384 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024) /* Reserved for malloc */

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * Local Bus Configuration & Clock Setup
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_8
#define CONFIG_SYS_LBC_LBCR		0x00000000
#define CONFIG_FSL_ELBC		1

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER	/* use the CFI driver */
#define CONFIG_SYS_FLASH_BASE		0xFE000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE		8 /* max FLASH size is 32M */

#define CONFIG_SYS_FLASH_PROTECTION	1	/* Use h/w Flash protection. */
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* display empty sectors */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* buffer up multiple bytes */

					/* Window base at flash base */
#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	0x80000016	/* 8 MB window size */

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE \
				| BR_PS_16	/* 16 bit port */ \
				| BR_MS_GPCM	/* MSEL = GPCM */ \
				| BR_V)		/* valid */
#define CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_9 \
				| OR_GPCM_EHTR_SET \
				| OR_GPCM_EAD)
				/* 0xFF800191 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256 /* max sectors per device */

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

/*
 * NAND Flash on the Local Bus
 */
#define CONFIG_SYS_NAND_BASE	0xE0600000
#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_NAND_BASE \
				| BR_DECC_CHK_GEN	/* Use HW ECC */ \
				| BR_PS_8		/* 8 bit port */ \
				| BR_MS_FCM		/* MSEL = FCM */ \
				| BR_V)			/* valid */
#define CONFIG_SYS_OR1_PRELIM	(OR_AM_32KB \
				| OR_FCM_CSCT \
				| OR_FCM_CST \
				| OR_FCM_CHT \
				| OR_FCM_SCY_1 \
				| OR_FCM_TRLX \
				| OR_FCM_EHTR)
#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_32KB)

/* Vitesse 7385 */

#define CONFIG_SYS_VSC7385_BASE	0xF0000000

#ifdef CONFIG_VSC7385_ENET

#define CONFIG_SYS_BR2_PRELIM		(CONFIG_SYS_VSC7385_BASE \
					| BR_PS_8 \
					| BR_MS_GPCM \
					| BR_V)
					/* 0xF0000801 */
#define CONFIG_SYS_OR2_PRELIM		(OR_AM_128KB \
					| OR_GPCM_CSNT \
					| OR_GPCM_XACS \
					| OR_GPCM_SCY_15 \
					| OR_GPCM_SETA \
					| OR_GPCM_TRLX_SET \
					| OR_GPCM_EHTR_SET \
					| OR_GPCM_EAD)
					/* 0xfffe09ff */

					/* Access Base */
#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_VSC7385_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM	(LBLAWAR_EN | LBLAWAR_128KB)

#endif

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/* SERDES */
#define CONFIG_FSL_SERDES
#define CONFIG_FSL_SERDES1	0xe3000
#define CONFIG_FSL_SERDES2	0xe3100

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS 1

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x51} }

/*
 * Config on-board RTC
 */
#define CONFIG_RTC_DS1374	/* use ds1374 rtc via i2c */
#define CONFIG_SYS_I2C_RTC_ADDR	0x68 /* at address 0x68 */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI_MEM_BASE		0x80000000
#define CONFIG_SYS_PCI_MEM_PHYS		CONFIG_SYS_PCI_MEM_BASE
#define CONFIG_SYS_PCI_MEM_SIZE		0x10000000 /* 256M */
#define CONFIG_SYS_PCI_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI_MMIO_PHYS	CONFIG_SYS_PCI_MMIO_BASE
#define CONFIG_SYS_PCI_MMIO_SIZE	0x10000000 /* 256M */
#define CONFIG_SYS_PCI_IO_BASE		0x00000000
#define CONFIG_SYS_PCI_IO_PHYS		0xE0300000
#define CONFIG_SYS_PCI_IO_SIZE		0x100000 /* 1M */

#define CONFIG_SYS_PCI_SLV_MEM_LOCAL	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_PCI_SLV_MEM_BUS	0x00000000
#define CONFIG_SYS_PCI_SLV_MEM_SIZE	0x80000000

#define CONFIG_SYS_PCIE1_BASE		0xA0000000
#define CONFIG_SYS_PCIE1_CFG_BASE	0xA0000000
#define CONFIG_SYS_PCIE1_CFG_SIZE	0x08000000
#define CONFIG_SYS_PCIE1_MEM_BASE	0xA8000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xA8000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x10000000
#define CONFIG_SYS_PCIE1_IO_BASE	0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xB8000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000

#define CONFIG_SYS_PCIE2_BASE		0xC0000000
#define CONFIG_SYS_PCIE2_CFG_BASE	0xC0000000
#define CONFIG_SYS_PCIE2_CFG_SIZE	0x08000000
#define CONFIG_SYS_PCIE2_MEM_BASE	0xC8000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xC8000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x10000000
#define CONFIG_SYS_PCIE2_IO_BASE	0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xD8000000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00800000

#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */
#endif	/* CONFIG_PCI */

/*
 * TSEC
 */
#ifdef CONFIG_TSEC_ENET

#define CONFIG_GMII			/* MII PHY management */

#define CONFIG_TSEC1

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME		"TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET		0x24000
#define TSEC1_PHY_ADDR			2
#define TSEC1_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC1_PHYIDX			0
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME		"TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET		0x25000
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

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1_OFFSET	0x18000
#define CONFIG_SYS_SATA1	(CONFIG_SYS_IMMR + CONFIG_SYS_SATA1_OFFSET)
#define CONFIG_SYS_SATA1_FLAGS	FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2_OFFSET	0x19000
#define CONFIG_SYS_SATA2	(CONFIG_SYS_IMMR + CONFIG_SYS_SATA2_OFFSET)
#define CONFIG_SYS_SATA2_FLAGS	FLAGS_DMA

#ifdef CONFIG_FSL_SATA
#define CONFIG_LBA48
#define CONFIG_CMD_SATA
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT2
#endif

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
	#define CONFIG_ENV_IS_IN_FLASH	1
	#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE+CONFIG_SYS_MONITOR_LEN)
	#define CONFIG_ENV_SECT_SIZE	0x10000	/* 64K (one sector) for env */
	#define CONFIG_ENV_SIZE		0x4000
#else
	#define CONFIG_SYS_NO_FLASH	1	/* Flash is not usable now */
	#define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-0x1000)
	#define CONFIG_ENV_SIZE		0x2000
#endif

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
#define CONFIG_CMD_DATE

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_SYS_RAMBOOT)
#undef CONFIG_CMD_SAVEENV
#undef CONFIG_CMD_LOADS
#endif

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support */

#undef CONFIG_WATCHDOG		/* watchdog disabled */

#define CONFIG_MMC     1

#ifdef CONFIG_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_ESDHC_PIN_MUX
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC83xx_ESDHC_ADDR
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x2000000 /* default load address */

#if defined(CONFIG_CMD_KGDB)
	#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size */
#else
	#define CONFIG_SYS_CBSIZE	256 /* Console I/O Buffer Size */
#endif

				/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
				/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */

/*
 * Core HID Setup
 */
#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK \
				| HID0_ENABLE_INSTRUCTION_CACHE)
#define CONFIG_SYS_HID2		HID2_HBE

/*
 * MMU Setup
 */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR: cache cacheable */
#define CONFIG_SYS_SDRAM_LOWER		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_SDRAM_UPPER		(CONFIG_SYS_SDRAM_BASE + 0x10000000)

#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_LOWER \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_LOWER \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U

#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_SDRAM_UPPER \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_SDRAM_UPPER \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U

/* IMMRBAR, PCI IO and NAND: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_IMMR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_IMMR \
				| BATU_BL_8M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT2L	CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U

/* L2 Switch: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_VSC7385_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_VSC7385_BASE \
				| BATU_BL_128K \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT3L	CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_FLASH_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT4U	(CONFIG_SYS_FLASH_BASE \
				| BATU_BL_32M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT4L	(CONFIG_SYS_FLASH_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U

/* Stack in dcache: cacheable, no memory coherence */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_INIT_RAM_ADDR \
				| BATU_BL_128K \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT5L	CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

#ifdef CONFIG_PCI
/* PCI MEM space: cacheable */
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_PCI_MEM_PHYS \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_PCI_MEM_PHYS \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
/* PCI MMIO space: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT7L	(CONFIG_SYS_PCI_MMIO_PHYS \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT7U	(CONFIG_SYS_PCI_MMIO_PHYS \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U
#else
#define CONFIG_SYS_IBAT6L	(0)
#define CONFIG_SYS_IBAT6U	(0)
#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

#define CONFIG_NETDEV		"eth1"

#define CONFIG_HOSTNAME		mpc837x_rdb
#define CONFIG_ROOTPATH		"/nfsroot"
#define CONFIG_RAMDISKFILE	"rootfs.ext2.gz.uboot"
#define CONFIG_BOOTFILE		"uImage"
				/* U-Boot image on TFTP server */
#define CONFIG_UBOOTPATH	"u-boot.bin"
#define CONFIG_FDTFILE		"mpc8379_rdb.dtb"

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		800000
#define CONFIG_BOOTDELAY	6	/* -1 disables auto-boot */
#define CONFIG_BAUDRATE		115200

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" CONFIG_NETDEV "\0"				\
	"uboot=" CONFIG_UBOOTPATH "\0"					\
	"tftpflash=tftp $loadaddr $uboot;"				\
		"protect off " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" +$filesize; "	\
		"erase " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize; "	\
		"protect on " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize\0"	\
	"fdtaddr=780000\0"						\
	"fdtfile=" CONFIG_FDTFILE "\0"					\
	"ramdiskaddr=1000000\0"						\
	"ramdiskfile=" CONFIG_RAMDISKFILE "\0"				\
	"console=ttyS0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath "	\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"	\
							"$netdev:off "	\
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

#endif	/* __CONFIG_H */
