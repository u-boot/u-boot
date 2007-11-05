/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
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
#define CONFIG_QE		1 /* Has QE */
#define CONFIG_MPC83XX		1 /* MPC83XX family */
#define CONFIG_MPC8360		1 /* MPC8360 CPU specific */
#define CONFIG_MPC8360EMDS	1 /* MPC8360EMDS board specific */
#undef CONFIG_PQ_MDS_PIB /* POWERQUICC MDS Platform IO Board */
#undef CONFIG_PQ_MDS_PIB_ATM /* QOC3 ATM card */

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
 */
#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_4X1 |\
	HRCWL_VCO_1X2 |\
	HRCWL_CE_PLL_VCO_DIV_4 |\
	HRCWL_CE_PLL_DIV_1X1 |\
	HRCWL_CE_TO_PLL_1X6 |\
	HRCWL_CORE_TO_CSB_2X1)

#ifdef CONFIG_PCISLAVE
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_AGENT |\
	HRCWH_PCI1_ARBITER_DISABLE |\
	HRCWH_PCICKDRV_DISABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0XFFF00100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT)
#else
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCICKDRV_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT)
#endif

/*
 * System IO Config
 */
#define CFG_SICRH		0x00000000
#define CFG_SICRL		0x40000000

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
#define CFG_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
				DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

#define CFG_83XX_DDR_USES_CS0

#define CONFIG_DDR_ECC		/* support DDR ECC function */
#define CONFIG_DDR_ECC_CMD	/* Use DDR ECC user commands */

/*
 * DDRCDR - DDR Control Driver Register
 */
#define CFG_DDRCDR_VALUE	0x80080001

#define CONFIG_SPD_EEPROM	/* Use SPD EEPROM for DDR setup */
#if defined(CONFIG_SPD_EEPROM)
/*
 * Determine DDR configuration from I2C interface.
 */
#define SPD_EEPROM_ADDRESS	0x52 /* DDR SODIMM */
#else
/*
 * Manually set up DDR parameters
 */
#define CFG_DDR_SIZE		256 /* MB */
#if defined(CONFIG_DDR_II)
#define CFG_DDRCDR		0x80080001
#define CFG_DDR_CS0_BNDS	0x0000000f
#define CFG_DDR_CS0_CONFIG	0x80330102
#define CFG_DDR_TIMING_0	0x00220802
#define CFG_DDR_TIMING_1	0x38357322
#define CFG_DDR_TIMING_2	0x2f9048c8
#define CFG_DDR_TIMING_3	0x00000000
#define CFG_DDR_CLK_CNTL	0x02000000
#define CFG_DDR_MODE		0x47d00432
#define CFG_DDR_MODE2		0x8000c000
#define CFG_DDR_INTERVAL	0x03cf0080
#define CFG_DDR_SDRAM_CFG	0x43000000
#define CFG_DDR_SDRAM_CFG2	0x00401000
#else
#define CFG_DDR_CONFIG (CSCONFIG_EN | CSCONFIG_ROW_BIT_13 | CSCONFIG_COL_BIT_9)
#define CFG_DDR_TIMING_1	0x37344321 /* tCL-tRCD-tRP-tRAS=2.5-3-3-7 */
#define CFG_DDR_TIMING_2	0x00000800 /* may need tuning */
#define CFG_DDR_CONTROL		0x42008000 /* Self refresh,2T timing */
#define CFG_DDR_MODE		0x20000162 /* DLL,normal,seq,4/2.5 */
#define CFG_DDR_INTERVAL	0x045b0100 /* page mode */
#endif
#endif

/*
 * Memory test
 */
#undef CFG_DRAM_TEST		/* memory test, takes time */
#define CFG_MEMTEST_START	0x00000000 /* memtest region */
#define CFG_MEMTEST_END		0x00100000

/*
 * The reserved memory
 */

#define CFG_MONITOR_BASE	TEXT_BASE /* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef	CFG_RAMBOOT
#endif

/* CFG_MONITOR_LEN must be a multiple of CFG_ENV_SECT_SIZE */
#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(128 * 1024) /* Reserved for malloc */

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
#define CFG_LCRR		(LCRR_DBYP | LCRR_CLKDIV_4)
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

#define CFG_BR0_PRELIM	(CFG_FLASH_BASE | /* Flash Base address */ \
			(2 << BR_PS_SHIFT) | /* 16 bit port size */ \
			BR_V)	/* valid */
#define CFG_OR0_PRELIM		((~(CFG_FLASH_SIZE - 1) << 20) | OR_UPM_XAM | \
				OR_GPCM_CSNT | OR_GPCM_ACS_0b11 | OR_GPCM_XACS | OR_GPCM_SCY_15 | \
				OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

#define CFG_MAX_FLASH_BANKS	1 /* number of banks */
#define CFG_MAX_FLASH_SECT	256 /* max sectors per device */

#undef	CFG_FLASH_CHECKSUM

/*
 * BCSR on the Local Bus
 */
#define CFG_BCSR		0xF8000000
#define CFG_LBLAWBAR1_PRELIM	CFG_BCSR /* Access window base at BCSR base */
#define CFG_LBLAWAR1_PRELIM	0x8000000F /* Access window size 64K */

#define CFG_BR1_PRELIM		(CFG_BCSR|0x00000801) /* Port size=8bit, MSEL=GPCM */
#define CFG_OR1_PRELIM		0xFFFFE9f7 /* length 32K */

/*
 * SDRAM on the Local Bus
 */
#define CFG_LBC_SDRAM_BASE	0xF0000000	/* SDRAM base address */
#define CFG_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

#define CFG_LB_SDRAM		/* if board has SRDAM on local bus */

#ifdef CFG_LB_SDRAM
#define CFG_LBLAWBAR2_PRELIM	CFG_LBC_SDRAM_BASE
#define CFG_LBLAWAR2_PRELIM	0x80000019 /* 64MB */

/*local bus BR2, OR2 definition for SDRAM if soldered on the EPB board */
/*
 * Base Register 2 and Option Register 2 configure SDRAM.
 * The SDRAM base address, CFG_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR2, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f0001861
 *
 * CFG_LBC_SDRAM_BASE should be masked and OR'ed into
 * the top 17 bits of BR2.
 */

#define CFG_BR2_PRELIM	0xf0001861 /*Port size=32bit, MSEL=SDRAM */

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

#define CFG_OR2_PRELIM	0xfc006901

#define CFG_LBC_LSRT	0x32000000 /* LB sdram refresh timer, about 6us */
#define CFG_LBC_MRTPR	0x20000000 /* LB refresh timer prescal, 266MHz/32 */

/*
 * LSDMR masks
 */
#define CFG_LBC_LSDMR_OP_NORMAL (0 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ARFRSH (1 << (31 - 4))
#define CFG_LBC_LSDMR_OP_SRFRSH (2 << (31 - 4))
#define CFG_LBC_LSDMR_OP_MRW	(3 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PRECH	(4 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PCHALL (5 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ACTBNK (6 << (31 - 4))
#define CFG_LBC_LSDMR_OP_RWINV	(7 << (31 - 4))

#define CFG_LBC_LSDMR_COMMON	0x0063b723

/*
 * SDRAM Controller configuration sequence.
 */
#define CFG_LBC_LSDMR_1		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_PCHALL)
#define CFG_LBC_LSDMR_2		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_ARFRSH)
#define CFG_LBC_LSDMR_3		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_ARFRSH)
#define CFG_LBC_LSDMR_4		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_MRW)
#define CFG_LBC_LSDMR_5		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_NORMAL)

#endif

/*
 * Windows to access PIB via local bus
 */
#define CFG_LBLAWBAR3_PRELIM	0xf8010000 /* windows base 0xf8010000 */
#define CFG_LBLAWAR3_PRELIM	0x8000000e /* windows size 32KB */

/*
 * CS4 on Local Bus, to PIB
 */
#define CFG_BR4_PRELIM	0xf8010801 /* CS4 base address at 0xf8010000 */
#define CFG_OR4_PRELIM	0xffffe9f7 /* size 32KB, port size 8bit, GPCM */

/*
 * CS5 on Local Bus, to PIB
 */
#define CFG_BR5_PRELIM	0xf8008801 /* CS5 base address at 0xf8008000 */
#define CFG_OR5_PRELIM	0xffffe9f7 /* size 32KB, port size 8bit, GPCM */

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1	(CFG_IMMR+0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR+0x4600)

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#undef  CONFIG_OF_FLAT_TREE
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_HAS_BD_T	1
#define CONFIG_OF_HAS_UBOOT_ENV	1

#define OF_CPU			"PowerPC,8360@0"
#define OF_SOC			"soc8360@e0000000"
#define OF_QE			"qe@e0100000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc8360@e0000000/serial@4500"

/* I2C */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_FSL_I2C
#define CFG_I2C_SPEED	400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE	0x7F
#define CFG_I2C_NOPROBES	{0x52} /* Don't probe these addrs */
#define CFG_I2C_OFFSET	0x3000
#define CFG_I2C2_OFFSET 0x3100

/*
 * Config on-board RTC
 */
#define CONFIG_RTC_DS1374		/* use ds1374 rtc via i2c */
#define CFG_I2C_RTC_ADDR	0x68	/* at address 0x68 */

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

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CFG_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */

#endif	/* CONFIG_PCI */


#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME		"Freescale GETH"
#define CONFIG_PHY_MODE_NEED_CHANGE

#define CONFIG_UEC_ETH1		/* GETH1 */

#ifdef CONFIG_UEC_ETH1
#define CFG_UEC1_UCC_NUM	0	/* UCC1 */
#define CFG_UEC1_RX_CLK		QE_CLK_NONE
#define CFG_UEC1_TX_CLK		QE_CLK9
#define CFG_UEC1_ETH_TYPE	GIGA_ETH
#define CFG_UEC1_PHY_ADDR	0
#define CFG_UEC1_INTERFACE_MODE ENET_1000_GMII
#endif

#define CONFIG_UEC_ETH2		/* GETH2 */

#ifdef CONFIG_UEC_ETH2
#define CFG_UEC2_UCC_NUM	1	/* UCC2 */
#define CFG_UEC2_RX_CLK		QE_CLK_NONE
#define CFG_UEC2_TX_CLK		QE_CLK4
#define CFG_UEC2_ETH_TYPE	GIGA_ETH
#define CFG_UEC2_PHY_ADDR	1
#define CFG_UEC2_INTERFACE_MODE ENET_1000_GMII
#endif

/*
 * Environment
 */

#ifndef CFG_RAMBOOT
	#define CFG_ENV_IS_IN_FLASH	1
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
	#define CFG_ENV_SECT_SIZE	0x20000
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
#define CONFIG_CMD_ASKENV

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#endif


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
 * Cache Config
 */
#define CFG_DCACHE_SIZE		32768
#define CFG_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5 /*log base 2 of the above value */
#endif

/*
 * MMU Setup
 */

/* DDR: cache cacheable */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U

/* IMMRBAR & PCI IO: cache-inhibit and guarded */
#define CFG_IBAT1L	(CFG_IMMR | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT1U	(CFG_IMMR | BATU_BL_4M | BATU_VS | BATU_VP)
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U

/* BCSR: cache-inhibit and guarded */
#define CFG_IBAT2L	(CFG_BCSR | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT2U	(CFG_BCSR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CFG_IBAT3L	(CFG_FLASH_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT3U	(CFG_FLASH_BASE | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_DBAT3L	(CFG_FLASH_BASE | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U	CFG_IBAT3U

/* Local bus SDRAM: cacheable */
#define CFG_IBAT4L	(CFG_LBC_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT4U	(CFG_LBC_SDRAM_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CFG_DBAT4L	CFG_IBAT4L
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

#if defined(CONFIG_UEC_ETH)
#define CONFIG_ETHADDR	00:04:9f:ef:01:01
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR 00:04:9f:ef:01:02
#endif

#define CONFIG_BAUDRATE 115200

#define CONFIG_LOADADDR 200000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY 6	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_EXTRA_ENV_SETTINGS					\
   "netdev=eth0\0"							\
   "consoledev=ttyS0\0"							\
   "ramdiskaddr=1000000\0"						\
   "ramdiskfile=ramfs.83xx\0"						\
   "fdtaddr=400000\0"							\
   "fdtfile=mpc8360emds.dtb\0"						\
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
