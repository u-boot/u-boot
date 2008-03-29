/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006. All rights reserved.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 MPC8349E-mITX and MPC8349E-mITX-GP board configuration file

 Memory map:

 0x0000_0000-0x0FFF_FFFF DDR SDRAM (256 MB)
 0x8000_0000-0x9FFF_FFFF PCI1 memory space (512 MB)
 0xA000_0000-0xBFFF_FFFF PCI2 memory space (512 MB)
 0xE000_0000-0xEFFF_FFFF IMMR (1 MB)
 0xE200_0000-0xE2FF_FFFF PCI1 I/O space (16 MB)
 0xE300_0000-0xE3FF_FFFF PCI2 I/O space (16 MB)
 0xF000_0000-0xF000_FFFF Compact Flash (MPC8349E-mITX only)
 0xF001_0000-0xF001_FFFF Local bus expansion slot
 0xF800_0000-0xF801_FFFF Vitesse 7385 Parallel Interface (MPC8349E-mITX only)
 0xFE00_0000-0xFE7F_FFFF First 8MB bank of Flash memory
 0xFE80_0000-0xFEFF_FFFF Second 8MB bank of Flash memory (MPC8349E-mITX only)

 I2C address list:
						Align.	Board
 Bus	Addr	Part No.	Description	Length	Location
 ----------------------------------------------------------------
 I2C0	0x50	M24256-BWMN6P	Board EEPROM	2	U64

 I2C1	0x20	PCF8574		I2C Expander	0	U8
 I2C1	0x21	PCF8574		I2C Expander	0	U10
 I2C1	0x38	PCF8574A	I2C Expander	0	U8
 I2C1	0x39	PCF8574A	I2C Expander	0	U10
 I2C1	0x51	(DDR)		DDR EEPROM	1	U1
 I2C1	0x68	DS1339		RTC		1	U68

 Note that a given board has *either* a pair of 8574s or a pair of 8574As.
*/

#ifndef __CONFIG_H
#define __CONFIG_H

#if (TEXT_BASE == 0xFE000000)
#define CFG_LOWBOOT
#endif

/*
 * High Level Configuration Options
 */
#define CONFIG_MPC834X		/* MPC834x family (8343, 8347, 8349) */
#define CONFIG_MPC8349		/* MPC8349 specific */

#define CFG_IMMR		0xE0000000	/* The IMMR is relocated to here */

#define CONFIG_MISC_INIT_F
#define CONFIG_MISC_INIT_R

/*
 * On-board devices
 */

#ifdef CONFIG_MPC8349ITX
#define CONFIG_COMPACT_FLASH	/* The CF card interface on the back of the board */
#define CONFIG_VSC7385_ENET	/* VSC7385 ethernet support */
#endif

#define CONFIG_PCI
#define CONFIG_RTC_DS1337
#define CONFIG_HARD_I2C
#define CONFIG_TSEC_ENET		/* TSEC Ethernet support */

/*
 * Device configurations
 */

/* I2C */
#ifdef CONFIG_HARD_I2C

#define CONFIG_FSL_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_I2C_OFFSET		0x3000
#define CFG_I2C2_OFFSET		0x3100
#define CFG_SPD_BUS_NUM		1	/* The I2C bus for SPD */

#define CFG_I2C_8574_ADDR1	0x20	/* I2C1, PCF8574 */
#define CFG_I2C_8574_ADDR2	0x21	/* I2C1, PCF8574 */
#define CFG_I2C_8574A_ADDR1	0x38	/* I2C1, PCF8574A */
#define CFG_I2C_8574A_ADDR2	0x39	/* I2C1, PCF8574A */
#define CFG_I2C_EEPROM_ADDR	0x50	/* I2C0, Board EEPROM */
#define CFG_I2C_RTC_ADDR	0x68	/* I2C1, DS1339 RTC*/
#define SPD_EEPROM_ADDRESS	0x51	/* I2C1, DDR */

#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

/* Don't probe these addresses: */
#define CFG_I2C_NOPROBES	{{1, CFG_I2C_8574_ADDR1}, \
				 {1, CFG_I2C_8574_ADDR2}, \
				 {1, CFG_I2C_8574A_ADDR1}, \
				 {1, CFG_I2C_8574A_ADDR2}}
/* Bit definitions for the 8574[A] I2C expander */
#define I2C_8574_REVISION	0x03	/* Board revision, 00=0.0, 01=0.1, 10=1.0 */
#define I2C_8574_CF		0x08	/* 1=Compact flash absent, 0=present */
#define I2C_8574_MPCICLKRN	0x10	/* MiniPCI Clk Run */
#define I2C_8574_PCI66		0x20	/* 0=33MHz PCI, 1=66MHz PCI */
#define I2C_8574_FLASHSIDE	0x40	/* 0=Reset vector from U4, 1=from U7*/

#undef CONFIG_SOFT_I2C

#endif

/* Compact Flash */
#ifdef CONFIG_COMPACT_FLASH

#define CFG_IDE_MAXBUS		1
#define CFG_IDE_MAXDEVICE	1

#define CFG_ATA_IDE0_OFFSET	0x0000
#define CFG_ATA_BASE_ADDR	CFG_CF_BASE
#define CFG_ATA_DATA_OFFSET	0x0000
#define CFG_ATA_REG_OFFSET	0
#define CFG_ATA_ALT_OFFSET	0x0200
#define CFG_ATA_STRIDE		2

#define ATA_RESET_TIME	1	/* If a CF card is not inserted, time out quickly */

#define CONFIG_DOS_PARTITION

#endif

/*
 * DDR Setup
 */
#define CFG_DDR_BASE		0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE 		CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE 	CFG_DDR_BASE
#define CFG_83XX_DDR_USES_CS0
#define CFG_MEMTEST_START	0x1000		/* memtest region */
#define CFG_MEMTEST_END		0x2000

#define CFG_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
				DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075)

#ifdef CONFIG_HARD_I2C
#define CONFIG_SPD_EEPROM		/* use SPD EEPROM for DDR setup*/
#endif

#ifndef CONFIG_SPD_EEPROM	/* No SPD? Then manually set up DDR parameters */
    #define CFG_DDR_SIZE	256		/* Mb */
    #define CFG_DDR_CONFIG	(CSCONFIG_EN | CSCONFIG_ROW_BIT_13 | CSCONFIG_COL_BIT_10)

    #define CFG_DDR_TIMING_1	0x26242321
    #define CFG_DDR_TIMING_2	0x00000800  /* P9-45, may need tuning */
#endif

/*
 *Flash on the Local Bus
 */

#define CFG_FLASH_CFI				/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER			/* use the CFI driver */
#define CFG_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CFG_FLASH_EMPTY_INFO
#define CFG_MAX_FLASH_SECT	135	/* 127 64KB sectors + 8 8KB sectors per device */
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

/* The ITX has two flash chips, but the ITX-GP has only one.  To support both
boards, we say we have two, but don't display a message if we find only one. */
#define CFG_FLASH_QUIET_TEST
#define CFG_MAX_FLASH_BANKS	2		/* number of banks */
#define CFG_FLASH_BANKS_LIST 	{CFG_FLASH_BASE, CFG_FLASH_BASE + 0x800000}
#define CFG_FLASH_SIZE		16		/* FLASH size in MB */
#define CFG_FLASH_SIZE_SHIFT	4		/* log2 of the above value */

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

#define CONFIG_TSEC2

/* The flash address and size of the VSC7385 firmware image */
#define CONFIG_VSC7385_IMAGE		0xFEFFE000
#define CONFIG_VSC7385_IMAGE_SIZE	8192

#endif

/*
 * BRx, ORx, LBLAWBARx, and LBLAWARx
 */

/* Flash */

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | BR_PS_16 | BR_V)
#define CFG_OR0_PRELIM		((~(CFG_FLASH_SIZE - 1) << 20) | OR_UPM_XAM | \
				OR_GPCM_CSNT | OR_GPCM_ACS_0b11 | OR_GPCM_XACS | OR_GPCM_SCY_15 | \
				OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)
#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE
#define CFG_LBLAWAR0_PRELIM	(LBLAWAR_EN | (0x13 + CFG_FLASH_SIZE_SHIFT))

/* Vitesse 7385 */

#define CFG_VSC7385_BASE	0xF8000000

#ifdef CONFIG_VSC7385_ENET

#define CFG_BR1_PRELIM		(CFG_VSC7385_BASE | BR_PS_8 | BR_V)
#define CFG_OR1_PRELIM		(OR_AM_128KB | OR_GPCM_CSNT | OR_GPCM_XACS | \
				OR_GPCM_SCY_15 | OR_GPCM_SETA | OR_GPCM_TRLX | \
				OR_GPCM_EHTR | OR_GPCM_EAD)

#define CFG_LBLAWBAR1_PRELIM	CFG_VSC7385_BASE
#define CFG_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_128KB)

#endif

/* LED */

#define CFG_LED_BASE		0xF9000000
#define CFG_BR2_PRELIM		(CFG_LED_BASE | BR_PS_8 | BR_V)
#define CFG_OR2_PRELIM		(OR_AM_2MB | OR_GPCM_CSNT | OR_GPCM_ACS_0b11 | \
				OR_GPCM_XACS | OR_GPCM_SCY_9 | OR_GPCM_TRLX | \
				OR_GPCM_EHTR | OR_GPCM_EAD)

/* Compact Flash */

#ifdef CONFIG_COMPACT_FLASH

#define CFG_CF_BASE		0xF0000000

#define CFG_BR3_PRELIM		(CFG_CF_BASE | BR_PS_16 | BR_MS_UPMA | BR_V)
#define CFG_OR3_PRELIM		(OR_UPM_AM | OR_UPM_BI)

#define CFG_LBLAWBAR3_PRELIM	CFG_CF_BASE
#define CFG_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_64KB)

#endif

/*
 * U-Boot memory configuration
 */
#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef	CFG_RAMBOOT
#endif

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0xFD000000	/* Initial RAM address */
#define CFG_INIT_RAM_END	0x1000		/* End of used area in RAM*/

#define CFG_GBL_DATA_SIZE	0x100		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/* CFG_MONITOR_LEN must be a multiple of CFG_ENV_SECT_SIZE */
#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(128 * 1024) /* Reserved for malloc */

/*
 * Local Bus LCRR and LBCR regs
 *    LCRR:  DLL bypass, Clock divider is 4
 * External Local Bus rate is
 *    CLKIN * HRCWL_CSB_TO_CLKIN / HRCWL_LCL_BUS_TO_SCB_CLK / LCRR_CLKDIV
 */
#define CFG_LCRR	(LCRR_DBYP | LCRR_CLKDIV_4)
#define CFG_LBC_LBCR	0x00000000

#define CFG_LBC_LSRT	0x32000000    /* LB sdram refresh timer, about 6us */
#define CFG_LBC_MRTPR	0x20000000    /* LB refresh timer prescal, 266MHz/32*/

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
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_CONSOLE		ttyS0
#define CONFIG_BAUDRATE		115200

#define CFG_NS16550_COM1	(CFG_IMMR + 0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR + 0x4600)

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * PCI
 */
#ifdef CONFIG_PCI

#define CONFIG_MPC83XX_PCI2

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCI1_MMIO_BASE	(CFG_PCI1_MEM_BASE + CFG_PCI1_MEM_SIZE)
#define CFG_PCI1_MMIO_PHYS	CFG_PCI1_MMIO_BASE
#define CFG_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xE2000000
#define CFG_PCI1_IO_SIZE	0x01000000	/* 16M */

#ifdef CONFIG_MPC83XX_PCI2
#define CFG_PCI2_MEM_BASE	(CFG_PCI1_MMIO_BASE + CFG_PCI1_MMIO_SIZE)
#define CFG_PCI2_MEM_PHYS	CFG_PCI2_MEM_BASE
#define CFG_PCI2_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCI2_MMIO_BASE	(CFG_PCI2_MEM_BASE + CFG_PCI2_MEM_SIZE)
#define CFG_PCI2_MMIO_PHYS	CFG_PCI2_MMIO_BASE
#define CFG_PCI2_MMIO_SIZE	0x10000000	/* 256M */
#define CFG_PCI2_IO_BASE	0x00000000
#define CFG_PCI2_IO_PHYS	(CFG_PCI1_IO_PHYS + CFG_PCI1_IO_SIZE)
#define CFG_PCI2_IO_SIZE	0x01000000	/* 16M */
#endif

#define _IO_BASE		0x00000000	/* points to PCI I/O space */

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#ifdef CONFIG_RTL8139
/* This macro is used by RTL8139 but not defined in PPC architecture */
#define KSEG1ADDR(x)	    (x)
#endif

#ifndef CONFIG_PCI_PNP
    #define PCI_ENET0_IOADDR	0x00000000
    #define PCI_ENET0_MEMADDR	CFG_PCI2_MEM_BASE
    #define PCI_IDSEL_NUMBER	0x0f	/* IDSEL = AD15 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif

#define PCI_66M
#ifdef PCI_66M
#define CONFIG_83XX_CLKIN	66666666	/* in Hz */
#else
#define CONFIG_83XX_CLKIN	33333333	/* in Hz */
#endif

/* TSEC */

#ifdef CONFIG_TSEC_ENET

#define CONFIG_NET_MULTI
#define CONFIG_MII
#define CONFIG_PHY_GIGE		/* In case CONFIG_CMD_MII is specified */

#define CONFIG_TSEC1

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME  "TSEC0"
#define CFG_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR		0x1c	/* VSC8201 uses address 0x1c */
#define TSEC1_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME  "TSEC1"
#define CFG_TSEC2_OFFSET	0x25000

#define TSEC2_PHY_ADDR		4
#define TSEC2_PHYIDX		0
#define TSEC2_FLAGS		TSEC_GIGABIT
#endif

#define CONFIG_ETHPRIME		"Freescale TSEC"

#endif

/*
 * Environment
 */
#define CONFIG_ENV_OVERWRITE

#ifndef CFG_RAMBOOT
  #define CFG_ENV_IS_IN_FLASH
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
  #define CFG_ENV_SECT_SIZE	0x10000 /* 64K (one sector) for environment */
  #define CFG_ENV_SIZE		0x2000
#else
  #define CFG_NO_FLASH		/* Flash is not usable now */
  #undef  CFG_FLASH_CFI_DRIVER
  #define CFG_ENV_IS_NOWHERE	/* Store ENV in memory only */
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
  #define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	/* allow baudrate change */

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

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_SDRAM

#ifdef CONFIG_COMPACT_FLASH
    #define CONFIG_CMD_IDE
    #define CONFIG_CMD_FAT
#endif

#ifdef CONFIG_PCI
    #define CONFIG_CMD_PCI
#endif

#ifdef CONFIG_HARD_I2C
    #define CONFIG_CMD_I2C
#endif

/* Watchdog */
#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory */
#define CONFIG_CMDLINE_EDITING		/* Command-line editing */
#define CFG_HUSH_PARSER			/* Use the HUSH parser */
#define CFG_PROMPT_HUSH_PS2 "> "

#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_LOADADDR	200000	/* default location for tftp and bootm */

#ifdef CONFIG_MPC8349ITX
#define CFG_PROMPT	"MPC8349E-mITX> "	/* Monitor Command Prompt */
#else
#define CFG_PROMPT	"MPC8349E-mITX-GP> "	/* Monitor Command Prompt */
#endif

#if defined(CONFIG_CMD_KGDB)
    #define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
    #define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE	(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_4X1 |\
	HRCWL_VCO_1X2 |\
	HRCWL_CORE_TO_CSB_2X1)

#ifdef CFG_LOWBOOT
#define CFG_HRCW_HIGH (\
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
	HRCWH_TSEC2M_IN_GMII )
#else
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_32_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0XFFF00100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII )
#endif

/*
 * System performance
 */
#define CFG_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CFG_ACR_RPTCNT		3	/* Arbiter repeat count (0-7) */
#define CFG_SPCR_TSEC1EP	3	/* TSEC1 emergency priority (0-3) */
#define CFG_SPCR_TSEC2EP	3	/* TSEC2 emergency priority (0-3) */
#define CFG_SCCR_TSEC1CM	1	/* TSEC1 clock mode (0-3) */
#define CFG_SCCR_TSEC2CM	1	/* TSEC2 & I2C0 clock mode (0-3) */

/*
 * System IO Config
 */
#define CFG_SICRH SICRH_TSOBI1	/* Needed for gigabit to work on TSEC 1 */
#define CFG_SICRL (SICRL_LDP_A | SICRL_USB1)

#define CFG_HID0_INIT	0x000000000
#define CFG_HID0_FINAL	CFG_HID0_INIT

#define CFG_HID2	HID2_HBE

/* DDR  */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* PCI  */
#ifdef CONFIG_PCI
#define CFG_IBAT1L	(CFG_PCI1_MEM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_PCI1_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT2L	(CFG_PCI1_MMIO_BASE | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT2U	(CFG_PCI1_MMIO_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#else
#define CFG_IBAT1L	0
#define CFG_IBAT1U	0
#define CFG_IBAT2L	0
#define CFG_IBAT2U	0
#endif

#ifdef CONFIG_MPC83XX_PCI2
#define CFG_IBAT3L	(CFG_PCI2_MEM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT3U	(CFG_PCI2_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT4L	(CFG_PCI2_MMIO_BASE | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT4U	(CFG_PCI2_MMIO_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#else
#define CFG_IBAT3L	0
#define CFG_IBAT3U	0
#define CFG_IBAT4L	0
#define CFG_IBAT4U	0
#endif

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 & BCSR @ 0xE2400000 */
#define CFG_IBAT5L	(CFG_IMMR | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT5U	(CFG_IMMR | BATU_BL_256M | BATU_VS | BATU_VP)

/* SDRAM @ 0xF0000000, stack in DCACHE 0xFDF00000 & FLASH @ 0xFE000000 */
#define CFG_IBAT6L	(0xF0000000 | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT6U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT7L	0
#define CFG_IBAT7U	0

#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U
#define CFG_DBAT3L	CFG_IBAT3L
#define CFG_DBAT3U	CFG_IBAT3U
#define CFG_DBAT4L	CFG_IBAT4L
#define CFG_DBAT4U	CFG_IBAT4U
#define CFG_DBAT5L	CFG_IBAT5L
#define CFG_DBAT5U	CFG_IBAT5U
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02	/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif


/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#ifdef CONFIG_HAS_ETH0
#define CONFIG_ETHADDR		00:E0:0C:00:8C:01
#endif

#ifdef CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:E0:0C:00:8C:02
#endif

#define CONFIG_IPADDR		192.168.1.253
#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.252.0
#define CONFIG_NETDEV		eth0

#ifdef CONFIG_MPC8349ITX
#define CONFIG_HOSTNAME		mpc8349emitx
#else
#define CONFIG_HOSTNAME		mpc8349emitxgp
#endif

/* Default path and filenames */
#define CONFIG_ROOTPATH		/nfsroot/rootfs
#define CONFIG_BOOTFILE		uImage
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */

#ifdef CONFIG_MPC8349ITX
#define CONFIG_FDTFILE		mpc8349emitx.dtb
#else
#define CONFIG_FDTFILE		mpc8349emitxgp.dtb
#endif

#define CONFIG_BOOTDELAY	0

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

#define CONFIG_BOOTARGS \
	"root=/dev/nfs rw" \
	" nfsroot=" MK_STR(CONFIG_SERVERIP) ":" MK_STR(CONFIG_ROOTPATH) \
	" ip=" MK_STR(CONFIG_IPADDR) ":" MK_STR(CONFIG_SERVERIP) ":" 	\
		MK_STR(CONFIG_GATEWAYIP) ":" MK_STR(CONFIG_NETMASK) ":" \
		MK_STR(CONFIG_HOSTNAME) ":" MK_STR(CONFIG_NETDEV) ":off" \
	" console=" MK_STR(CONFIG_CONSOLE) "," MK_STR(CONFIG_BAUDRATE)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=" MK_STR(CONFIG_CONSOLE) "\0" 				\
	"netdev=" MK_STR(CONFIG_NETDEV) "\0" 				\
	"uboot=" MK_STR(CONFIG_UBOOTPATH) "\0" 				\
	"tftpflash=tftpboot $loadaddr $uboot; " 			\
		"protect off " MK_STR(TEXT_BASE) " +$filesize; " 	\
		"erase " MK_STR(TEXT_BASE) " +$filesize; " 		\
		"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; " 	\
		"protect on " MK_STR(TEXT_BASE) " +$filesize; " 	\
		"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0" 	\
	"fdtaddr=400000\0"						\
	"fdtfile=" MK_STR(CONFIG_FDTFILE) "\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath"	\
	" ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	" console=$console,$baudrate $othbootargs; "			\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw"				\
	" console=$console,$baudrate $othbootargs; "			\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#undef MK_STR
#undef XMK_STR

#endif
