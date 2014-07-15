/*
 * WindRiver SBC8349 U-Boot configuration file.
 * Copyright (c) 2006, 2007 Wind River Systems, Inc.
 *
 * Paul Gortmaker <paul.gortmaker@windriver.com>
 * Based on the MPC8349EMDS config.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * sbc8349 board configuration file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */
#define CONFIG_MPC834x		1	/* MPC834x family */
#define CONFIG_MPC8349		1	/* MPC8349 specific */
#define CONFIG_SBC8349		1	/* WRS SBC8349 board specific */

#define	CONFIG_SYS_TEXT_BASE	0xFF800000

/* Don't enable PCI2 on sbc834x - it doesn't exist physically. */
#undef CONFIG_MPC83XX_PCI2		/* support for 2nd PCI controller */

/*
 * The default if PCI isn't enabled, or if no PCI clk setting is given
 * is 66MHz; this is what the board defaults to when the PCI slot is
 * physically empty.  The board will automatically (i.e w/o jumpers)
 * clock down to 33MHz if you insert a 33MHz PCI card.
 */
#ifdef CONFIG_PCI_33M
#define CONFIG_83XX_CLKIN	33000000	/* in Hz */
#else	/* 66M */
#define CONFIG_83XX_CLKIN	66000000	/* in Hz */
#endif

#ifndef CONFIG_SYS_CLK_FREQ
#ifdef CONFIG_PCI_33M
#define CONFIG_SYS_CLK_FREQ	33000000
#define HRCWL_CSB_TO_CLKIN	HRCWL_CSB_TO_CLKIN_8X1
#else	/* 66M */
#define CONFIG_SYS_CLK_FREQ	66000000
#define HRCWL_CSB_TO_CLKIN	HRCWL_CSB_TO_CLKIN_4X1
#endif
#endif

#undef CONFIG_BOARD_EARLY_INIT_F		/* call board_pre_init */

#define CONFIG_SYS_IMMR		0xE0000000

#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * DDR Setup
 */
#undef CONFIG_DDR_ECC			/* only for ECC DDR module */
#undef CONFIG_DDR_ECC_CMD		/* use DDR ECC user commands */
#define CONFIG_SPD_EEPROM		/* use SPD EEPROM for DDR setup*/
#define CONFIG_SYS_83XX_DDR_USES_CS0	/* WRS; Fsl board uses CS2/CS3 */

/*
 * 32-bit data path mode.
 *
 * Please note that using this mode for devices with the real density of 64-bit
 * effectively reduces the amount of available memory due to the effect of
 * wrapping around while translating address to row/columns, for example in the
 * 256MB module the upper 128MB get aliased with contents of the lower
 * 128MB); normally this define should be used for devices with real 32-bit
 * data path.
 */
#undef CONFIG_DDR_32BIT

#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
				DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075)
#define CONFIG_DDR_2T_TIMING

#if defined(CONFIG_SPD_EEPROM)
/*
 * Determine DDR configuration from I2C interface.
 */
#define SPD_EEPROM_ADDRESS	0x52		/* DDR DIMM */

#else
/*
 * Manually set up DDR parameters
 * NB: manual DDR setup untested on sbc834x
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#define CONFIG_SYS_DDR_CS2_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)
#define CONFIG_SYS_DDR_TIMING_1	0x36332321
#define CONFIG_SYS_DDR_TIMING_2	0x00000800	/* P9-45,may need tuning */
#define CONFIG_SYS_DDR_CONTROL	0xc2000000	/* unbuffered,no DYN_PWR */
#define CONFIG_SYS_DDR_INTERVAL	0x04060100	/* autocharge,no open page */

#if defined(CONFIG_DDR_32BIT)
/* set burst length to 8 for 32-bit data path */
				/* DLL,normal,seq,4/2.5, 8 burst len */
#define CONFIG_SYS_DDR_MODE	0x00000023
#else
/* the default burst length is 4 - for 64-bit data path */
				/* DLL,normal,seq,4/2.5, 4 burst len */
#define CONFIG_SYS_DDR_MODE	0x00000022
#endif
#endif

/*
 * SDRAM on the Local Bus
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xF0000000	/* Localbus SDRAM */
#define CONFIG_SYS_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER		/* use the CFI driver */
#define CONFIG_SYS_FLASH_BASE		0xFF800000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		8		/* flash size in MB */
/* #define CONFIG_SYS_FLASH_USE_BUFFER_WRITE */

#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE \
					| BR_PS_16	/* 16 bit port */ \
					| BR_MS_GPCM	/* MSEL = GPCM */ \
					| BR_V)		/* valid */

#define CONFIG_SYS_OR0_PRELIM		(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) \
					| OR_GPCM_XAM \
					| OR_GPCM_CSNT \
					| OR_GPCM_ACS_DIV2 \
					| OR_GPCM_XACS \
					| OR_GPCM_SCY_15 \
					| OR_GPCM_TRLX_SET \
					| OR_GPCM_EHTR_SET \
					| OR_GPCM_EAD)
					/* 0xFF806FF7 */

					/* window base at flash base */
#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_8MB)

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef  CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
					/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000
					/* Size of used area in RAM*/
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)	/* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)	/* Reserved for malloc */

/*
 * Local Bus LCRR and LBCR regs
 *    LCRR:  DLL bypass, Clock divider is 4
 * External Local Bus rate is
 *    CLKIN * HRCWL_CSB_TO_CLKIN / HRCWL_LCL_BUS_TO_SCB_CLK / LCRR_CLKDIV
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_4
#define CONFIG_SYS_LBC_LBCR	0x00000000

#undef CONFIG_SYS_LB_SDRAM	/* if board has SDRAM on local bus */

#ifdef CONFIG_SYS_LB_SDRAM
/* Local bus BR2, OR2 definition for SDRAM if soldered on the board*/
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
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = F0001861
 */

#define CONFIG_SYS_BR2_PRELIM		(CONFIG_SYS_LBC_SDRAM_BASE \
					| BR_PS_32 \
					| BR_MS_SDRAM \
					| BR_V)
					/* 0xF0001861 */
#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_LBC_SDRAM_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM	(LBLAWAR_EN | LBLAWAR_64MB)

/*
 * The SDRAM size in MB, CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR2, need:
 *    64MB mask for AM, OR2[0:7] = 1111 1100
 *                 XAM, OR2[17:18] = 11
 *    9 columns OR2[19-21] = 010
 *    13 rows   OR2[23-25] = 100
 *    EAD set for extra time OR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1100 0000 0000 0110 1001 0000 0001 = FC006901
 */

#define CONFIG_SYS_OR2_PRELIM	(MEG_TO_AM(CONFIG_SYS_LBC_SDRAM_SIZE) \
			| OR_SDRAM_XAM \
			| ((9 - OR_SDRAM_MIN_COLS) << OR_SDRAM_COLS_SHIFT) \
			| ((13 - OR_SDRAM_MIN_ROWS) << OR_SDRAM_ROWS_SHIFT) \
			| OR_SDRAM_EAD)
			/* 0xFC006901 */

				/* LB sdram refresh timer, about 6us */
#define CONFIG_SYS_LBC_LSRT	0x32000000
				/* LB refresh timer prescal, 266MHz/32 */
#define CONFIG_SYS_LBC_MRTPR	0x20000000

#define CONFIG_SYS_LBC_LSDMR_COMMON	(LSDMR_RFEN \
					| LSDMR_BSMA1516 \
					| LSDMR_RFCR8 \
					| LSDMR_PRETOACT6 \
					| LSDMR_ACTTORW3 \
					| LSDMR_BL8 \
					| LSDMR_WRC3 \
					| LSDMR_CL3)

/*
 * SDRAM Controller configuration sequence.
 */
#define CONFIG_SYS_LBC_LSDMR_1	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_PCHALL)
#define CONFIG_SYS_LBC_LSDMR_2	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_ARFRSH)
#define CONFIG_SYS_LBC_LSDMR_3	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_ARFRSH)
#define CONFIG_SYS_LBC_LSDMR_4	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_MRW)
#define CONFIG_SYS_LBC_LSDMR_5	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_NORMAL)
#endif

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX     1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE    1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1        (CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2        (CONFIG_SYS_IMMR+0x4600)

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support   */
/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69}, {1, 0x69} }
/* could also use CONFIG_I2C_MULTI_BUS and CONFIG_SYS_SPD_BUS_NUM... */

/* TSEC */
#define CONFIG_SYS_TSEC1_OFFSET 0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET 0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC2_OFFSET)

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00100000	/* 1M */

#define CONFIG_SYS_PCI2_MEM_BASE	0xA0000000
#define CONFIG_SYS_PCI2_MEM_PHYS	CONFIG_SYS_PCI2_MEM_BASE
#define CONFIG_SYS_PCI2_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_MMIO_BASE	0xB0000000
#define CONFIG_SYS_PCI2_MMIO_PHYS	CONFIG_SYS_PCI2_MMIO_BASE
#define CONFIG_SYS_PCI2_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_IO_BASE		0x00000000
#define CONFIG_SYS_PCI2_IO_PHYS		0xE2100000
#define CONFIG_SYS_PCI2_IO_SIZE		0x00100000	/* 1M */

#if defined(CONFIG_PCI)

#define PCI_64BIT
#define PCI_ONE_PCI1
#if defined(PCI_64BIT)
#undef PCI_ALL_PCI1
#undef PCI_TWO_PCI1
#undef PCI_ONE_PCI1
#endif

#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
	#define PCI_ENET0_IOADDR	0xFIXME
	#define PCI_ENET0_MEMADDR	0xFIXME
	#define PCI_IDSEL_NUMBER	0xFIXME
#endif

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */

#endif	/* CONFIG_PCI */

/*
 * TSEC configuration
 */
#define CONFIG_TSEC_ENET		/* TSEC ethernet support */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define CONFIG_PHY_BCM5421S	1
#define TSEC1_PHY_ADDR		0x19
#define TSEC2_PHY_ADDR		0x1a
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"TSEC0"

#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
	#define CONFIG_ENV_IS_IN_FLASH	1
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x40000)
	#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
	#define CONFIG_ENV_SIZE		0x2000

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

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
#include <config_cmd_default.h>

#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_SYS_RAMBOOT)
    #undef CONFIG_CMD_SAVEENV
    #undef CONFIG_CMD_LOADS
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

#if defined(CONFIG_CMD_KGDB)
	#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#else
	#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
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
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

#define CONFIG_SYS_RCWH_PCIHOST 0x80000000 /* PCIHOST  */

#if 1 /*528/264*/
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN |\
	HRCWL_VCO_1X2 |\
	HRCWL_CORE_TO_CSB_2X1)
#elif 0 /*396/132*/
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN |\
	HRCWL_VCO_1X4 |\
	HRCWL_CORE_TO_CSB_3X1)
#elif 0 /*264/132*/
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN |\
	HRCWL_VCO_1X4 |\
	HRCWL_CORE_TO_CSB_2X1)
#elif 0 /*132/132*/
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN |\
	HRCWL_VCO_1X4 |\
	HRCWL_CORE_TO_CSB_1X1)
#elif 0 /*264/264 */
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN |\
	HRCWL_VCO_1X4 |\
	HRCWL_CORE_TO_CSB_1X1)
#endif

#if defined(PCI_64BIT)
#define CONFIG_SYS_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_64_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_DISABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII)
#else
#define CONFIG_SYS_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_32_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII)
#endif

/* System IO Config */
#define CONFIG_SYS_SICRH 0
#define CONFIG_SYS_SICRL SICRL_LDP_A

#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK \
				| HID0_ENABLE_INSTRUCTION_CACHE)

/* #define CONFIG_SYS_HID0_FINAL	(\
	HID0_ENABLE_INSTRUCTION_CACHE |\
	HID0_ENABLE_M_BIT |\
	HID0_ENABLE_ADDRESS_BROADCAST) */


#define CONFIG_SYS_HID2 HID2_HBE

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR @ 0x00000000 */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* PCI @ 0x80000000 */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#else
#define CONFIG_SYS_IBAT1L	(0)
#define CONFIG_SYS_IBAT1U	(0)
#define CONFIG_SYS_IBAT2L	(0)
#define CONFIG_SYS_IBAT2U	(0)
#endif

#ifdef CONFIG_MPC83XX_PCI2
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_PCI2_MEM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_PCI2_MEM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_PCI2_MMIO_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT4U	(CONFIG_SYS_PCI2_MMIO_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#else
#define CONFIG_SYS_IBAT3L	(0)
#define CONFIG_SYS_IBAT3U	(0)
#define CONFIG_SYS_IBAT4L	(0)
#define CONFIG_SYS_IBAT4U	(0)
#endif

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_IMMR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_IMMR \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* LBC SDRAM @ 0xF0000000, stack in DCACHE 0xFDF00000 & FLASH @ 0xFE000000 */
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_LBC_SDRAM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_LBC_SDRAM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)

#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT2L	CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT3L	CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U
#define CONFIG_SYS_DBAT4L	CONFIG_SYS_IBAT4L
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U
#define CONFIG_SYS_DBAT5L	CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

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

#define CONFIG_HOSTNAME		SBC8349
#define CONFIG_ROOTPATH		"/tftpboot/rootfs"
#define CONFIG_BOOTFILE		"uImage"

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		800000

#define CONFIG_BOOTDELAY	6	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE	 115200

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=sbc8349\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"	\
		"bootm\0"						\
	"load=tftp 100000 /tftpboot/sbc8349/u-boot.bin\0"		\
	"update=protect off ff800000 ff83ffff; "			\
		"era ff800000 ff83ffff; cp.b 100000 ff800000 ${filesize}\0" \
	"upd=run load update\0"						\
	"fdtaddr=780000\0"						\
	"fdtfile=sbc8349.dtb\0"						\
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

#define CONFIG_BOOTCOMMAND	"run flash_self"

#endif	/* __CONFIG_H */
