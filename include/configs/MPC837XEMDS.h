/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Dave Liu <daveliu@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_DISPLAY_BOARDINFO

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */
#define CONFIG_MPC837x		1 /* MPC837x CPU specific */
#define CONFIG_MPC837XEMDS	1 /* MPC837XEMDS board specific */

#define	CONFIG_SYS_TEXT_BASE	0xFE000000

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
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_SVCOD_DIV_2 |\
	HRCWL_CSB_TO_CLKIN_6X1 |\
	HRCWL_CORE_TO_CSB_1_5X1)

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

/* Arbiter Configuration Register */
#define CONFIG_SYS_ACR_PIPE_DEP	3	/* Arbiter pipeline depth is 4 */
#define CONFIG_SYS_ACR_RPTCNT	3	/* Arbiter repeat count is 4 */

/* System Priority Control Register */
#define CONFIG_SYS_SPCR_TSECEP	3 /* eTSEC1/2 emergency has highest priority */

/*
 * IP blocks clock configuration
 */
#define CONFIG_SYS_SCCR_TSEC1CM	1	/* CSB:eTSEC1 = 1:1 */
#define CONFIG_SYS_SCCR_TSEC2CM	1	/* CSB:eTSEC2 = 1:1 */
#define CONFIG_SYS_SCCR_SATACM	SCCR_SATACM_2	/* CSB:SATA[0:3] = 2:1 */

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRH		0x00000000
#define CONFIG_SYS_SICRL		0x00000000

/*
 * Output Buffer Impedance
 */
#define CONFIG_SYS_OBIR		0x31100000

#define CONFIG_BOARD_EARLY_INIT_F /* call board_pre_init */
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_HWCONFIG

/*
 * IMMR new address
 */
#define CONFIG_SYS_IMMR		0xE0000000

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
#define CONFIG_SYS_83XX_DDR_USES_CS0
#define CONFIG_SYS_DDRCDR_VALUE		(DDRCDR_DHC_EN \
					| DDRCDR_ODT \
					| DDRCDR_Q_DRN)
					/* 0x80080001 */ /* ODT 150ohm on SoC */

#undef CONFIG_DDR_ECC		/* support DDR ECC function */
#undef CONFIG_DDR_ECC_CMD	/* Use DDR ECC user commands */

#define CONFIG_SPD_EEPROM	/* Use SPD EEPROM for DDR setup */
#define CONFIG_NEVER_ASSERT_ODT_TO_CPU /* Never assert ODT to internal IOs */

#if defined(CONFIG_SPD_EEPROM)
#define SPD_EEPROM_ADDRESS	0x51 /* I2C address of DDR SODIMM SPD */
#else
/*
 * Manually set up DDR parameters
 * WHITE ELECTRONIC DESIGNS - W3HG64M72EEU403PD4 SO-DIMM
 * consist of nine chips from SAMSUNG K4T51083QE-ZC(L)D5
 */
#define CONFIG_SYS_DDR_SIZE		512 /* MB */
#define CONFIG_SYS_DDR_CS0_BNDS	0x0000001f
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
			| CSCONFIG_ODT_RD_NEVER  /* ODT_RD to none */ \
			| CSCONFIG_ODT_WR_ONLY_CURRENT  /* ODT_WR to CSn */ \
			| CSCONFIG_ROW_BIT_14 \
			| CSCONFIG_COL_BIT_10)
			/* 0x80010202 */
#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (6 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00620802 */
#define CONFIG_SYS_DDR_TIMING_1	((3 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (9 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (3 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (5 << TIMING_CFG1_CASLAT_SHIFT) \
				| (13 << TIMING_CFG1_REFREC_SHIFT) \
				| (3 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x3935d322 */
#define CONFIG_SYS_DDR_TIMING_2	((1 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (6 << TIMING_CFG2_CPO_SHIFT) \
				| (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (4 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (8 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x131088c8 */
#define CONFIG_SYS_DDR_INTERVAL	((0x03E0 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (0x0100 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x03E00100 */
#define CONFIG_SYS_DDR_SDRAM_CFG	0x43000000
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00001000 /* 1 posted refresh */
#define CONFIG_SYS_DDR_MODE	((0x0448 << SDRAM_MODE_ESD_SHIFT) \
				| (0x1432 << SDRAM_MODE_SD_SHIFT))
				/* ODT 150ohm CL=3, AL=1 on SDRAM */
#define CONFIG_SYS_DDR_MODE2	0x00000000
#endif

/*
 * Memory test
 */
#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00040000 /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00140000

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE /* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
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
#define CONFIG_SYS_FLASH_CFI	/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER	/* use the CFI driver */
#define CONFIG_SYS_FLASH_BASE	0xFE000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE	32 /* max FLASH size is 32M */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* Use h/w Flash protection. */

					/* Window base at flash base */
#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_32MB)

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE \
				| BR_PS_16	/* 16 bit port */ \
				| BR_MS_GPCM	/* MSEL = GPCM */ \
				| BR_V)		/* valid */
#define CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) \
				| OR_UPM_XAM \
				| OR_GPCM_CSNT \
				| OR_GPCM_ACS_DIV2 \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_15 \
				| OR_GPCM_TRLX_SET \
				| OR_GPCM_EHTR_SET \
				| OR_GPCM_EAD)
				/* 0xFE000FF7 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256 /* max sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000 /* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500 /* Flash Write Timeout (ms) */

/*
 * BCSR on the Local Bus
 */
#define CONFIG_SYS_BCSR		0xF8000000
					/* Access window base at BCSR base */
#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_BCSR
#define CONFIG_SYS_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_32KB)

#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_BCSR \
				| BR_PS_8 \
				| BR_MS_GPCM \
				| BR_V)
				/* 0xF8000801 */
#define CONFIG_SYS_OR1_PRELIM	(OR_AM_32KB \
				| OR_GPCM_XAM \
				| OR_GPCM_CSNT \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_15 \
				| OR_GPCM_TRLX_SET \
				| OR_GPCM_EHTR_SET \
				| OR_GPCM_EAD)
				/* 0xFFFFE9F7 */

/*
 * NAND Flash on the Local Bus
 */
#define CONFIG_CMD_NAND		1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_NAND_FSL_ELBC	1

#define CONFIG_SYS_NAND_BASE	0xE0600000
#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_NAND_BASE \
				| BR_DECC_CHK_GEN	/* Use HW ECC */ \
				| BR_PS_8		/* 8 bit port */ \
				| BR_MS_FCM		/* MSEL = FCM */ \
				| BR_V)			/* valid */
#define CONFIG_SYS_OR3_PRELIM	(OR_AM_32KB \
				| OR_FCM_BCTLD \
				| OR_FCM_CST \
				| OR_FCM_CHT \
				| OR_FCM_SCY_1 \
				| OR_FCM_RST \
				| OR_FCM_TRLX \
				| OR_FCM_EHTR)
				/* 0xFFFF919E */

#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_32KB)

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

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
#ifndef __ASSEMBLY__
extern int board_pci_host_broken(void);
#endif
#define CONFIG_PCIE
#define CONFIG_PQ_MDS_PIB	1 /* PQ MDS Platform IO Board */

#define CONFIG_HAS_FSL_DR_USB	1 /* fixup device tree for the DR USB */
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */
#endif /* CONFIG_PCI */

/*
 * TSEC
 */
#define CONFIG_TSEC_ENET	/* TSEC ethernet support */
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC2_OFFSET)

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
#define TSEC1_PHY_ADDR_SGMII	8
#define TSEC2_PHY_ADDR_SGMII	4
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"eTSEC1"

/* SERDES */
#define CONFIG_FSL_SERDES
#define CONFIG_FSL_SERDES1	0xe3000
#define CONFIG_FSL_SERDES2	0xe3100

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
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
	#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
	#define CONFIG_ENV_SIZE		0x2000
#else
	#define CONFIG_SYS_NO_FLASH	1	/* Flash is not usable now */
	#define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
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
#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support   */

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
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR		0x2000000 /* default load address */

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
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK | \
				 HID0_ENABLE_INSTRUCTION_CACHE)
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

/* BCSR: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_BCSR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_BCSR \
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

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_BAUDRATE 115200

#define CONFIG_LOADADDR 800000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY 6	/* -1 disables auto-boot */
#undef CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS0\0"						\
	"ramdiskaddr=1000000\0"						\
	"ramdiskfile=ramfs.83xx\0"					\
	"fdtaddr=780000\0"						\
	"fdtfile=mpc8379_mds.dtb\0"					\
	""

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
		"nfsroot=$serverip:$rootpath "				\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"	\
							"$netdev:off "	\
		"console=$consoledev,$baudrate $othbootargs;"		\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw "				\
		"console=$consoledev,$baudrate $othbootargs;"		\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"


#define CONFIG_BOOTCOMMAND CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
