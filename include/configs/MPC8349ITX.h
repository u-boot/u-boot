/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
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

#if (CONFIG_SYS_TEXT_BASE == 0xFE000000)
#define CONFIG_SYS_LOWBOOT
#endif

/*
 * High Level Configuration Options
 */
#define CONFIG_MPC83xx		1
#define CONFIG_MPC834x		/* MPC834x family (8343, 8347, 8349) */
#define CONFIG_MPC8349		/* MPC8349 specific */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFEF00000
#endif

#define CONFIG_SYS_IMMR	0xE0000000	/* The IMMR is relocated to here */

#define CONFIG_MISC_INIT_F
#define CONFIG_MISC_INIT_R

/*
 * On-board devices
 */

#ifdef CONFIG_MPC8349ITX
/* The CF card interface on the back of the board */
#define CONFIG_COMPACT_FLASH
#define CONFIG_VSC7385_ENET	/* VSC7385 ethernet support */
#define CONFIG_SATA_SIL3114	/* SIL3114 SATA controller */
#define CONFIG_SYS_USB_HOST	/* use the EHCI USB controller */
#endif

#define CONFIG_PCI
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C
#define CONFIG_TSEC_ENET		/* TSEC Ethernet support */

/*
 * Device configurations
 */

/* I2C */
#ifdef CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

#define CONFIG_SYS_SPD_BUS_NUM		1	/* The I2C bus for SPD */
#define CONFIG_SYS_RTC_BUS_NUM		1	/* The I2C bus for RTC */

#define CONFIG_SYS_I2C_8574_ADDR1	0x20	/* I2C1, PCF8574 */
#define CONFIG_SYS_I2C_8574_ADDR2	0x21	/* I2C1, PCF8574 */
#define CONFIG_SYS_I2C_8574A_ADDR1	0x38	/* I2C1, PCF8574A */
#define CONFIG_SYS_I2C_8574A_ADDR2	0x39	/* I2C1, PCF8574A */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* I2C0, Board EEPROM */
#define CONFIG_SYS_I2C_RTC_ADDR		0x68	/* I2C1, DS1339 RTC*/
#define SPD_EEPROM_ADDRESS		0x51	/* I2C1, DDR */

/* Don't probe these addresses: */
#define CONFIG_SYS_I2C_NOPROBES	{ {1, CONFIG_SYS_I2C_8574_ADDR1}, \
				 {1, CONFIG_SYS_I2C_8574_ADDR2}, \
				 {1, CONFIG_SYS_I2C_8574A_ADDR1}, \
				 {1, CONFIG_SYS_I2C_8574A_ADDR2} }
/* Bit definitions for the 8574[A] I2C expander */
				/* Board revision, 00=0.0, 01=0.1, 10=1.0 */
#define I2C_8574_REVISION	0x03
#define I2C_8574_CF		0x08	/* 1=Compact flash absent, 0=present */
#define I2C_8574_MPCICLKRN	0x10	/* MiniPCI Clk Run */
#define I2C_8574_PCI66		0x20	/* 0=33MHz PCI, 1=66MHz PCI */
#define I2C_8574_FLASHSIDE	0x40	/* 0=Reset vector from U4, 1=from U7*/

#endif

/* Compact Flash */
#ifdef CONFIG_COMPACT_FLASH

#define CONFIG_SYS_IDE_MAXBUS		1
#define CONFIG_SYS_IDE_MAXDEVICE	1

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_CF_BASE
#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000
#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0200
#define CONFIG_SYS_ATA_STRIDE		2

/* If a CF card is not inserted, time out quickly */
#define ATA_RESET_TIME	1

#endif

/*
 * SATA
 */
#ifdef CONFIG_SATA_SIL3114

#define CONFIG_SYS_SATA_MAX_DEVICE      4
#define CONFIG_LIBATA
#define CONFIG_LBA48

#endif

#ifdef CONFIG_SYS_USB_HOST
/*
 * Support USB
 */
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL

/* Current USB implementation supports the only USB controller,
 * so we have to choose between the MPH or the DR ones */
#if 1
#define CONFIG_HAS_FSL_MPH_USB
#else
#define CONFIG_HAS_FSL_DR_USB
#endif

#endif

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_83XX_DDR_USES_CS0
#define CONFIG_SYS_MEMTEST_START	0x1000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x2000

#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN \
					| DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075)

#define CONFIG_VERY_BIG_RAM
#define CONFIG_MAX_MEM_MAPPED   ((phys_size_t)256 << 20)

#ifdef CONFIG_SYS_I2C
#define CONFIG_SPD_EEPROM		/* use SPD EEPROM for DDR setup*/
#endif

/* No SPD? Then manually set up DDR parameters */
#ifndef CONFIG_SPD_EEPROM
    #define CONFIG_SYS_DDR_SIZE		256	/* Mb */
    #define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)

    #define CONFIG_SYS_DDR_TIMING_1	0x26242321
    #define CONFIG_SYS_DDR_TIMING_2	0x00000800  /* P9-45, may need tuning */
#endif

/*
 *Flash on the Local Bus
 */

#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER		/* use the CFI driver */
#define CONFIG_SYS_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_EMPTY_INFO
/* 127 64KB sectors + 8 8KB sectors per device */
#define CONFIG_SYS_MAX_FLASH_SECT	135
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

/* The ITX has two flash chips, but the ITX-GP has only one.  To support both
boards, we say we have two, but don't display a message if we find only one. */
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* number of banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	\
		{CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE + 0x800000}
#define CONFIG_SYS_FLASH_SIZE		16	/* FLASH size in MB */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* Use h/w Flash protection. */

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

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE \
				| BR_PS_16 \
				| BR_MS_GPCM \
				| BR_V)
#define CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) \
				| OR_UPM_XAM \
				| OR_GPCM_CSNT \
				| OR_GPCM_ACS_DIV2 \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_15 \
				| OR_GPCM_TRLX_SET \
				| OR_GPCM_EHTR_SET \
				| OR_GPCM_EAD)
#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_16MB)

/* Vitesse 7385 */

#define CONFIG_SYS_VSC7385_BASE	0xF8000000

#ifdef CONFIG_VSC7385_ENET

#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_VSC7385_BASE \
				| BR_PS_8 \
				| BR_MS_GPCM \
				| BR_V)
#define CONFIG_SYS_OR1_PRELIM	(OR_AM_128KB \
				| OR_GPCM_CSNT \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_15 \
				| OR_GPCM_SETA \
				| OR_GPCM_TRLX_SET \
				| OR_GPCM_EHTR_SET \
				| OR_GPCM_EAD)

#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_VSC7385_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_128KB)

#endif

/* LED */

#define CONFIG_SYS_LED_BASE	0xF9000000
#define CONFIG_SYS_BR2_PRELIM	(CONFIG_SYS_LED_BASE \
				| BR_PS_8 \
				| BR_MS_GPCM \
				| BR_V)
#define CONFIG_SYS_OR2_PRELIM	(OR_AM_2MB \
				| OR_GPCM_CSNT \
				| OR_GPCM_ACS_DIV2 \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_9 \
				| OR_GPCM_TRLX_SET \
				| OR_GPCM_EHTR_SET \
				| OR_GPCM_EAD)

/* Compact Flash */

#ifdef CONFIG_COMPACT_FLASH

#define CONFIG_SYS_CF_BASE	0xF0000000

#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_CF_BASE \
				| BR_PS_16 \
				| BR_MS_UPMA \
				| BR_V)
#define CONFIG_SYS_OR3_PRELIM	(OR_UPM_AM | OR_UPM_BI)

#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_CF_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_64KB)

#endif

/*
 * U-Boot memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef	CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000	/* Initial RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN	(384 * 1024) /* Reserve 384 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024) /* Reserved for malloc */

/*
 * Local Bus LCRR and LBCR regs
 *    LCRR:  DLL bypass, Clock divider is 4
 * External Local Bus rate is
 *    CLKIN * HRCWL_CSB_TO_CLKIN / HRCWL_LCL_BUS_TO_SCB_CLK / LCRR_CLKDIV
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_4
#define CONFIG_SYS_LBC_LBCR	0x00000000

				/* LB sdram refresh timer, about 6us */
#define CONFIG_SYS_LBC_LSRT	0x32000000
				/* LB refresh timer prescal, 266MHz/32*/
#define CONFIG_SYS_LBC_MRTPR	0x20000000

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_CONSOLE		ttyS0
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * PCI
 */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE

#define CONFIG_MPC83XX_PCI2

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	\
			(CONFIG_SYS_PCI1_MEM_BASE + CONFIG_SYS_PCI1_MEM_SIZE)
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x01000000	/* 16M */

#ifdef CONFIG_MPC83XX_PCI2
#define CONFIG_SYS_PCI2_MEM_BASE	\
			(CONFIG_SYS_PCI1_MMIO_BASE + CONFIG_SYS_PCI1_MMIO_SIZE)
#define CONFIG_SYS_PCI2_MEM_PHYS	CONFIG_SYS_PCI2_MEM_BASE
#define CONFIG_SYS_PCI2_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_MMIO_BASE	\
			(CONFIG_SYS_PCI2_MEM_BASE + CONFIG_SYS_PCI2_MEM_SIZE)
#define CONFIG_SYS_PCI2_MMIO_PHYS	CONFIG_SYS_PCI2_MMIO_BASE
#define CONFIG_SYS_PCI2_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_IO_BASE		0x00000000
#define CONFIG_SYS_PCI2_IO_PHYS		\
			(CONFIG_SYS_PCI1_IO_PHYS + CONFIG_SYS_PCI1_IO_SIZE)
#define CONFIG_SYS_PCI2_IO_SIZE		0x01000000	/* 16M */
#endif

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#ifndef CONFIG_PCI_PNP
    #define PCI_ENET0_IOADDR	0x00000000
    #define PCI_ENET0_MEMADDR	CONFIG_SYS_PCI2_MEM_BASE
    #define PCI_IDSEL_NUMBER	0x0f	/* IDSEL = AD15 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif

#define CONFIG_PCI_66M
#ifdef CONFIG_PCI_66M
#define CONFIG_83XX_CLKIN	66666666	/* in Hz */
#else
#define CONFIG_83XX_CLKIN	33333333	/* in Hz */
#endif

/* TSEC */

#ifdef CONFIG_TSEC_ENET

#define CONFIG_MII
#define CONFIG_PHY_GIGE		/* In case CONFIG_CMD_MII is specified */

#define CONFIG_TSEC1

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME  "TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR		0x1c	/* VSC8201 uses address 0x1c */
#define TSEC1_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME  "TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET	0x25000

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

#ifndef CONFIG_SYS_RAMBOOT
  #define CONFIG_ENV_IS_IN_FLASH
  #define CONFIG_ENV_ADDR	\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
  #define CONFIG_ENV_SECT_SIZE	0x10000 /* 64K (one sector) for environment */
  #define CONFIG_ENV_SIZE	0x2000
#else
  #define CONFIG_SYS_NO_FLASH	/* Flash is not usable now */
  #undef  CONFIG_FLASH_CFI_DRIVER
  #define CONFIG_ENV_IS_NOWHERE	/* Store ENV in memory only */
  #define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - 0x1000)
  #define CONFIG_ENV_SIZE	0x2000
#endif

#define CONFIG_LOADS_ECHO	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

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
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SDRAM

#if defined(CONFIG_COMPACT_FLASH) || defined(CONFIG_SATA_SIL3114) \
				|| defined(CONFIG_USB_STORAGE)
	#define CONFIG_DOS_PARTITION
	#define CONFIG_CMD_FAT
	#define CONFIG_SUPPORT_VFAT
#endif

#ifdef CONFIG_COMPACT_FLASH
	#define CONFIG_CMD_IDE
#endif

#ifdef CONFIG_SATA_SIL3114
	#define CONFIG_CMD_SATA
#endif

#if defined(CONFIG_SATA_SIL3114) || defined(CONFIG_USB_STORAGE)
	#define CONFIG_CMD_EXT2
#endif

#ifdef CONFIG_PCI
	#define CONFIG_CMD_PCI
#endif

#ifdef CONFIG_SYS_I2C
	#define CONFIG_CMD_I2C
#endif

/* Watchdog */
#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_CMDLINE_EDITING		/* Command-line editing */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support */
#define CONFIG_SYS_HUSH_PARSER		/* Use the HUSH parser */

#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_LOADADDR	800000	/* default location for tftp and bootm */

#ifdef CONFIG_MPC8349ITX
#define CONFIG_SYS_PROMPT "MPC8349E-mITX> "	/* Monitor Command Prompt */
#else
#define CONFIG_SYS_PROMPT "MPC8349E-mITX-GP> "	/* Monitor Command Prompt */
#endif

#if defined(CONFIG_CMD_KGDB)
	#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#else
	#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
#endif

				/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
				/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_SYS_HZ		1000	/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_4X1 |\
	HRCWL_VCO_1X2 |\
	HRCWL_CORE_TO_CSB_2X1)

#ifdef CONFIG_SYS_LOWBOOT
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
#else
#define CONFIG_SYS_HRCW_HIGH (\
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
	HRCWH_TSEC2M_IN_GMII)
#endif

/*
 * System performance
 */
#define CONFIG_SYS_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CONFIG_SYS_ACR_RPTCNT	3	/* Arbiter repeat count (0-7) */
#define CONFIG_SYS_SPCR_TSEC1EP	3	/* TSEC1 emergency priority (0-3) */
#define CONFIG_SYS_SPCR_TSEC2EP	3	/* TSEC2 emergency priority (0-3) */
#define CONFIG_SYS_SCCR_TSEC1CM	1	/* TSEC1 clock mode (0-3) */
#define CONFIG_SYS_SCCR_TSEC2CM	1	/* TSEC2 & I2C0 clock mode (0-3) */
#define CONFIG_SYS_SCCR_USBMPHCM 3	/* USB MPH controller's clock */
#define CONFIG_SYS_SCCR_USBDRCM	0	/* USB DR controller's clock */

/*
 * System IO Config
 */
/* Needed for gigabit to work on TSEC 1 */
#define CONFIG_SYS_SICRH SICRH_TSOBI1
				/* USB DR as device + USB MPH as host */
#define CONFIG_SYS_SICRL	(SICRL_LDP_A | SICRL_USB1)

#define CONFIG_SYS_HID0_INIT	0x00000000
#define CONFIG_SYS_HID0_FINAL	HID0_ENABLE_INSTRUCTION_CACHE

#define CONFIG_SYS_HID2	HID2_HBE
#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR  */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* PCI  */
#ifdef CONFIG_PCI
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
#define CONFIG_SYS_IBAT1L	0
#define CONFIG_SYS_IBAT1U	0
#define CONFIG_SYS_IBAT2L	0
#define CONFIG_SYS_IBAT2U	0
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
#define CONFIG_SYS_IBAT3L	0
#define CONFIG_SYS_IBAT3U	0
#define CONFIG_SYS_IBAT4L	0
#define CONFIG_SYS_IBAT4U	0
#endif

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 & BCSR @ 0xE2400000 */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_IMMR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_IMMR \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* SDRAM @ 0xF0000000, stack in DCACHE 0xFDF00000 & FLASH @ 0xFE000000 */
#define CONFIG_SYS_IBAT6L	(0xF0000000 \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT6U	(0xF0000000 \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

#define CONFIG_SYS_IBAT7L	0
#define CONFIG_SYS_IBAT7U	0

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
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif


/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_NETDEV		"eth0"

#ifdef CONFIG_MPC8349ITX
#define CONFIG_HOSTNAME		"mpc8349emitx"
#else
#define CONFIG_HOSTNAME		"mpc8349emitxgp"
#endif

/* Default path and filenames */
#define CONFIG_ROOTPATH		"/nfsroot/rootfs"
#define CONFIG_BOOTFILE		"uImage"
				/* U-Boot image on TFTP server */
#define CONFIG_UBOOTPATH	"u-boot.bin"

#ifdef CONFIG_MPC8349ITX
#define CONFIG_FDTFILE		"mpc8349emitx.dtb"
#else
#define CONFIG_FDTFILE		"mpc8349emitxgp.dtb"
#endif

#define CONFIG_BOOTDELAY	6

#define CONFIG_BOOTARGS \
	"root=/dev/nfs rw" \
	" nfsroot=" __stringify(CONFIG_SERVERIP) ":" CONFIG_ROOTPATH	\
	" ip=" __stringify(CONFIG_IPADDR) ":"		\
		__stringify(CONFIG_SERVERIP) ":"	\
		__stringify(CONFIG_GATEWAYIP) ":"	\
		__stringify(CONFIG_NETMASK) ":"		\
		CONFIG_HOSTNAME ":" CONFIG_NETDEV ":off"		\
	" console=" __stringify(CONFIG_CONSOLE) "," __stringify(CONFIG_BAUDRATE)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=" __stringify(CONFIG_CONSOLE) "\0"			\
	"netdev=" CONFIG_NETDEV "\0"					\
	"uboot=" CONFIG_UBOOTPATH "\0"					\
	"tftpflash=tftpboot $loadaddr $uboot; "				\
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
	"fdtfile=" CONFIG_FDTFILE "\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath"	\
	" ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "\
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

#endif
