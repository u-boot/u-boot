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
 MPC8349E-mITX board configuration file

 Memory map:

 0x0000_0000-0x0FFF_FFFF DDR SDRAM (256 MB)
 0x8000_0000-0x9FFF_FFFF PCI1 memory space (512 MB)
 0xA000_0000-0xBFFF_FFFF PCI2 memory space (512 MB)
 0xE000_0000-0xEFFF_FFFF IMMR (1 MB)
 0xE200_0000-0xE2FF_FFFF PCI1 I/O space (16 MB)
 0xE300_0000-0xE3FF_FFFF PCI2 I/O space (16 MB)
 0xF000_0000-0xF000_FFFF Compact Flash
 0xF001_0000-0xF001_FFFF Local bus expansion slot
 0xF800_0000-0xF801_FFFF GBE L2 Switch VSC7385
 0xFF00_0000-0xFF7F_FFFF Alternative bank of Flash memory (8MB)
 0xFF80_0000-0xFFFF_FFFF Boot Flash (8 MB)

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

#undef DEBUG

/*
 * High Level Configuration Options
 */
#define CONFIG_MPC834X		/* MPC834x family (8343, 8347, 8349) */
#define CONFIG_MPC8349		/* MPC8349 specific */

#define CONFIG_PCI

#define CONFIG_COMPACT_FLASH	/* The CF card interface on the back of the board */
#define CONFIG_RTC_DS1337

/* I2C */
#define CONFIG_HARD_I2C

#ifdef CONFIG_HARD_I2C

#define CONFIG_MISC_INIT_F
#define CONFIG_MISC_INIT_R

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

#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#define PCI_66M
#ifdef PCI_66M
#define CONFIG_83XX_CLKIN	66666666	/* in Hz */
#else
#define CONFIG_83XX_CLKIN	33333333	/* in Hz */
#endif

#ifndef CONFIG_SYS_CLK_FREQ
#ifdef PCI_66M
#define CONFIG_SYS_CLK_FREQ	66666666
#else
#define CONFIG_SYS_CLK_FREQ	33333333
#endif
#endif

#define CFG_IMMR		0xE0000000	/* The IMMR is relocated to here */

#undef CFG_DRAM_TEST				/* memory test, takes time */
#define CFG_MEMTEST_START	0x00003000	/* memtest region */
#define CFG_MEMTEST_END		0x07100000	/* only has 128M */

/*
 * DDR Setup
 */
#undef CONFIG_DDR_ECC			/* only for ECC DDR module */
#undef CONFIG_DDR_ECC_CMD		/* use DDR ECC user commands */
#define CONFIG_SPD_EEPROM		/* use SPD EEPROM for DDR setup*/

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

#define CFG_DDR_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE CFG_DDR_BASE
#undef	CONFIG_DDR_2T_TIMING
#define CFG_83XX_DDR_USES_CS0

#ifndef CONFIG_SPD_EEPROM
/*
 * Manually set up DDR parameters
 */
    #define CFG_DDR_SIZE	256		/* Mb */
    #define CFG_DDR_CONFIG	(CSCONFIG_EN | CSCONFIG_ROW_BIT_13 | CSCONFIG_COL_BIT_10)

    #define CFG_DDR_TIMING_1	0x26242321
    #define CFG_DDR_TIMING_2	0x00000800  /* P9-45, may need tuning */
#endif

/* FLASH on the Local Bus */
#define CFG_FLASH_CFI				/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER			/* use the CFI driver */
#define CFG_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CFG_FLASH_SIZE		16		/* FLASH size in MB */
#define CFG_FLASH_EMPTY_INFO

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | BR_PS_16 | BR_V)
#define CFG_OR0_PRELIM		((~(CFG_FLASH_SIZE - 1) << 20) | OR_UPM_XAM | \
				OR_GPCM_CSNT | OR_GPCM_ACS_0b11 | OR_GPCM_XACS | OR_GPCM_SCY_15 | \
				OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)
#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE	/* Window base at flash base */
#define CFG_LBLAWAR0_PRELIM	0x80000017	/* 16Mb window bytes */

/* VSC7385 on the Local Bus */
#define CFG_VSC7385_BASE	0xF8000000	/* start of VSC7385   */

#define CFG_BR1_PRELIM		(CFG_VSC7385_BASE | BR_PS_8 | BR_V)
#define CFG_OR1_PRELIM		(0xFFFE0000 /* 128KB */ | \
				OR_GPCM_CSNT | OR_GPCM_XACS | OR_GPCM_SCY_15 | \
				OR_GPCM_SETA | OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

#define CFG_LBLAWBAR1_PRELIM	CFG_VSC7385_BASE	/* Access window base at VSC7385 base */
#define CFG_LBLAWAR1_PRELIM	0x80000010		/* Access window size 128K */

#define CFG_MAX_FLASH_BANKS	2		/* number of banks */
#define CFG_MAX_FLASH_SECT	135		/* sectors per device */

#define CFG_FLASH_BANKS_LIST {CFG_FLASH_BASE, CFG_FLASH_BASE + 0x800000}

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CFG_LED_BASE		0xF9000000  /* start of LED and Board ID */
#define CFG_BR2_PRELIM		(CFG_LED_BASE | BR_PS_8 | BR_V)
#define CFG_OR2_PRELIM		(0xFFE00000 /* 2MB */ | \
				OR_GPCM_CSNT | OR_GPCM_ACS_0b11 | OR_GPCM_XACS | \
				OR_GPCM_SCY_9 | \
				OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

#ifdef CONFIG_COMPACT_FLASH

#define CFG_CF_BASE		0xF0000000

#define CFG_BR3_PRELIM		(CFG_CF_BASE | BR_PS_16 | BR_MS_UPMA | BR_V)
#define CFG_OR3_PRELIM		(OR_UPM_AM | OR_UPM_BI)

#define CFG_LBLAWBAR2_PRELIM	CFG_CF_BASE	/* Window base at flash base + LED & Board ID */
#define CFG_LBLAWAR2_PRELIM	0x8000000F	/* 64K bytes */

#undef CONFIG_IDE_RESET
#undef CONFIG_IDE_PREINIT

#define CFG_IDE_MAXBUS		1
#define CFG_IDE_MAXDEVICE	1

#define CFG_ATA_IDE0_OFFSET	0x0000
#define CFG_ATA_BASE_ADDR	CFG_CF_BASE
#define CFG_ATA_DATA_OFFSET	0x0000
#define CFG_ATA_REG_OFFSET	0
#define CFG_ATA_ALT_OFFSET	0x0200
#define CFG_ATA_STRIDE		2

#define ATA_RESET_TIME	1	/* If a CF card is not inserted, time out quickly */

#endif

#define CONFIG_DOS_PARTITION

#define CFG_MID_FLASH_JUMP	0x7F000000
#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */


#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef	CFG_RAMBOOT
#endif

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0xFD000000		/* Initial RAM address */
#define CFG_INIT_RAM_END	0x1000	     /* End of used area in RAM*/

#define CFG_GBL_DATA_SIZE	0x100	  /* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

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

#undef CFG_LB_SDRAM	/* if board has SRDAM on local bus */

#ifdef CFG_LB_SDRAM
/*local bus BR2, OR2 definition for SDRAM if soldered on the ADS board*/
/*
 * Base Register 2 and Option Register 2 configure SDRAM.
 * The SDRAM base address, CFG_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR2, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = F0001861
 */

#define CFG_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#define CFG_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

#define CFG_LBLAWBAR2_PRELIM	0xF0000000
#define CFG_LBLAWAR2_PRELIM	0x80000019 /* 64M */

#define CFG_BR2_PRELIM		(CFG_LBC_SDRAM_BASE | BR_PS_32 | BR_MS_SDRAM | BR_V)
#define CFG_OR2_PRELIM		(0xFC000000 /* 64 MB */ | \
				 OR_SDRAM_XAM | \
				 ((9 - 7) << OR_SDRAM_COLS_SHIFT) | \
				 ((13 - 9) << OR_SDRAM_ROWS_SHIFT) | \
				 OR_SDRAM_EAD)

#define CFG_LBC_LSRT	0x32000000    /* LB sdram refresh timer, about 6us */
#define CFG_LBC_MRTPR	0x20000000    /* LB refresh timer prescal, 266MHz/32*/

/*
 * LSDMR masks
 */
#define CFG_LBC_LSDMR_RFEN	(1 << (31 -  1))
#define CFG_LBC_LSDMR_BSMA1516	(3 << (31 - 10))
#define CFG_LBC_LSDMR_BSMA1617	(4 << (31 - 10))
#define CFG_LBC_LSDMR_RFCR5	(3 << (31 - 16))
#define CFG_LBC_LSDMR_RFCR8	(5 << (31 - 16))
#define CFG_LBC_LSDMR_RFCR16	(7 << (31 - 16))
#define CFG_LBC_LSDMR_PRETOACT3 (3 << (31 - 19))
#define CFG_LBC_LSDMR_PRETOACT6 (5 << (31 - 19))
#define CFG_LBC_LSDMR_PRETOACT7 (7 << (31 - 19))
#define CFG_LBC_LSDMR_ACTTORW3	(3 << (31 - 22))
#define CFG_LBC_LSDMR_ACTTORW7	(7 << (31 - 22))
#define CFG_LBC_LSDMR_ACTTORW6	(6 << (31 - 22))
#define CFG_LBC_LSDMR_BL8	(1 << (31 - 23))
#define CFG_LBC_LSDMR_WRC2	(2 << (31 - 27))
#define CFG_LBC_LSDMR_WRC3	(3 << (31 - 27))
#define CFG_LBC_LSDMR_WRC4	(0 << (31 - 27))
#define CFG_LBC_LSDMR_BUFCMD	(1 << (31 - 29))
#define CFG_LBC_LSDMR_CL3	(3 << (31 - 31))

#define CFG_LBC_LSDMR_OP_NORMAL (0 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ARFRSH (1 << (31 - 4))
#define CFG_LBC_LSDMR_OP_SRFRSH (2 << (31 - 4))
#define CFG_LBC_LSDMR_OP_MRW	(3 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PRECH	(4 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PCHALL (5 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ACTBNK (6 << (31 - 4))
#define CFG_LBC_LSDMR_OP_RWINV	(7 << (31 - 4))

#define CFG_LBC_LSDMR_COMMON	( CFG_LBC_LSDMR_RFEN		\
				| CFG_LBC_LSDMR_BSMA1516	\
				| CFG_LBC_LSDMR_RFCR8		\
				| CFG_LBC_LSDMR_PRETOACT6	\
				| CFG_LBC_LSDMR_ACTTORW3	\
				| CFG_LBC_LSDMR_BL8		\
				| CFG_LBC_LSDMR_WRC3		\
				| CFG_LBC_LSDMR_CL3		\
				)

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

#define CFG_NS16550_COM1	(CFG_IMMR + 0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR + 0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_FLAT_TREE	1
#define CONFIG_OF_BOARD_SETUP	1

/* maximum size of the flat tree (8K) */
#define OF_FLAT_TREE_MAX_SIZE	8192

#define OF_CPU			"PowerPC,8349@0"
#define OF_SOC			"soc8349@e0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc8349@e0000000/serial@4500"

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

/* TSEC */

#ifdef CONFIG_TSEC_ENET

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI
#endif

#define CONFIG_MII
#define CONFIG_PHY_GIGE		/* In case CFG_CMD_MII is specified */

#define CONFIG_MPC83XX_TSEC1

#ifdef CONFIG_MPC83XX_TSEC1
#define CONFIG_MPC83XX_TSEC1_NAME  "TSEC0"
#define CFG_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR		0x1c	/* VSC8201 uses address 0x1c */
#define TSEC1_PHYIDX		0
#endif

#ifdef CONFIG_MPC83XX_TSEC2
#define CONFIG_MPC83XX_TSEC2_NAME  "TSEC1"
#define CFG_TSEC2_OFFSET	0x25000
#define CONFIG_UNKNOWN_TSEC	/* TSEC2 is proprietary */
#define TSEC2_PHY_ADDR		4
#define TSEC2_PHYIDX		0
#endif

#define CONFIG_ETHPRIME		"Freescale TSEC"

#endif


/*
 * Environment
 */
#ifndef CFG_RAMBOOT
  #define CFG_ENV_IS_IN_FLASH
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x40000)
  #define CFG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
  #define CFG_ENV_SIZE		0x2000
#else
  #define CFG_NO_FLASH		/* Flash is not usable now */
  #define CFG_ENV_IS_NOWHERE	/* Store ENV in memory only */
  #define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
  #define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	/* allow baudrate change */

/* CONFIG_COMMANDS */

#ifdef CONFIG_COMPACT_FLASH
#define CONFIG_COMMANDS_CF	(CFG_CMD_IDE | CFG_CMD_FAT)
#else
#define CONFIG_COMMANDS_CF	0
#endif

#ifdef CONFIG_PCI
#define CONFIG_COMMANDS_PCI	CFG_CMD_PCI
#else
#define CONFIG_COMMANDS_PCI	0
#endif

#ifdef CONFIG_HARD_I2C
#define CONFIG_COMMANDS_I2C	CFG_CMD_I2C
#else
#define CONFIG_COMMANDS_I2C	0
#endif

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL | \
				CONFIG_COMMANDS_CF	| \
				CFG_CMD_NET	| \
				CFG_CMD_PING	| \
				CONFIG_COMMANDS_I2C	| \
				CONFIG_COMMANDS_PCI	| \
				CFG_CMD_SDRAM	| \
				CFG_CMD_DATE	| \
				CFG_CMD_CACHE	| \
				CFG_CMD_IRQ)
#include <cmd_confdefs.h>

/* Watchdog */

#undef CONFIG_WATCHDOG		/* watchdog disabled */
#ifdef CONFIG_WATCHDOG
#define CFG_WATCHDOG_VALUE	0xFFFFFFC3
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory */
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"MPC8349E-mITX> "		/* Monitor Command Prompt */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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

/* Cache Configuration */
#define CFG_DCACHE_SIZE		32768
#define CFG_CACHELINE_SIZE	32
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log2 of the above value */
#endif

#define CFG_RCWH_PCIHOST 0x80000000 /* PCIHOST	*/

#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_4X1 |\
	HRCWL_VCO_1X2 |\
	HRCWL_CORE_TO_CSB_2X1)

#ifdef PCI_64BIT
#define CFG_HRCW_HIGH (\
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
	HRCWH_TSEC2M_IN_GMII )
#else
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_32_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_DISABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0XFFF00100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII )
#endif

/* System performance */
#define CFG_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CFG_ACR_RPTCNT		3	/* Arbiter repeat count (0-7) */
#define CFG_SPCR_TSEC1EP	3	/* TSEC1 emergency priority (0-3) */
#define CFG_SPCR_TSEC2EP	3	/* TSEC2 emergency priority (0-3) */
#define CFG_SCCR_TSEC1CM	1	/* TSEC1 clock mode (0-3) */
#define CFG_SCCR_TSEC2CM	1	/* TSEC2 & I2C0 clock mode (0-3) */
#define CFG_ACR_RPTCNT		3	/* Arbiter repeat count */

/* System IO Config */
#define CFG_SICRH SICRH_TSOBI1	/* Needed for gigabit to work on TSEC 1 */
#define CFG_SICRL (SICRL_LDP_A | SICRL_USB1)

#define CFG_HID0_INIT 0x000000000

#define CFG_HID0_FINAL CFG_HID0_INIT

#define CFG_HID2	HID2_HBE

/* DDR @ 0x00000000 */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* PCI @ 0x80000000 */
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

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif


/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#ifdef CONFIG_MPC83XX_TSEC1
#define CONFIG_ETHADDR		00:E0:0C:00:8C:01
#endif

#ifdef CONFIG_MPC83XX_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:E0:0C:00:8C:02
#endif

#if 1
#define CONFIG_IPADDR		10.82.19.159
#define CONFIG_SERVERIP		10.82.48.106
#define CONFIG_GATEWAYIP	10.82.19.254
#define CONFIG_NETMASK		255.255.252.0
#define CONFIG_NETDEV		eth0

#define CONFIG_HOSTNAME		mpc8349emitx
#define CONFIG_ROOTPATH		/nfsroot0/u/timur/itx-ltib/rootfs
#define CONFIG_BOOTFILE		timur/uImage

#define CONFIG_UBOOTPATH	timur/u-boot.bin
#else
#define CONFIG_IPADDR		192.168.1.253
#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.252.0
#define CONFIG_NETDEV		eth0

#define CONFIG_HOSTNAME		mpc8349emitx
#define CONFIG_ROOTPATH		/nfsroot/rootfs
#define CONFIG_BOOTFILE		uImage

#define CONFIG_UBOOTPATH	u-boot.bin
#endif

#define CONFIG_UBOOTSTART	fe700000
#define CONFIG_UBOOTEND		fe77ffff

#define CONFIG_LOADADDR		200000	/* default location for tftp and bootm */

#define CONFIG_BAUDRATE		115200

#undef CONFIG_BOOTCOMMAND
#ifdef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTDELAY	6
#else
#define CONFIG_BOOTDELAY	-1	/* -1 disables auto-boot */
#endif

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

#define CONFIG_BOOTARGS \
	"root=/dev/nfs rw" \
	" nfsroot=" MK_STR(CONFIG_SERVERIP) ":" MK_STR(CONFIG_ROOTPATH) \
	" ip=" MK_STR(CONFIG_IPADDR) ":" MK_STR(CONFIG_SERVERIP) ":" \
		MK_STR(CONFIG_GATEWAYIP) ":" MK_STR(CONFIG_NETMASK) ":" \
		MK_STR(CONFIG_HOSTNAME) ":" MK_STR(CONFIG_NETDEV) ":off" \
	" console=ttyS0," MK_STR(CONFIG_BAUDRATE)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" MK_STR(CONFIG_NETDEV) "\0" \
	"tftpflash=tftpboot $loadaddr " MK_STR(CONFIG_UBOOTPATH) "; " \
		"erase " MK_STR(CONFIG_UBOOTSTART) " " MK_STR(CONFIG_UBOOTEND) "; " \
		"cp.b $loadaddr " MK_STR(CONFIG_UBOOTSTART) " $filesize; " \
		"cmp.b $loadaddr " MK_STR(CONFIG_UBOOTSTART) " $filesize\0" \
	"tftpupdate=tftpboot $loadaddr " MK_STR(CONFIG_UBOOTPATH) "; " \
		"protect off FEF00000 FEF7FFFF; " \
		"erase FEF00000 FEF7FFFF; " \
		"cp.b $loadaddr FEF00000 $filesize; " \
		"protect on FEF00000 FEF7FFFF; " \
		"cmp.b $loadaddr FEF00000 $filesize\0" \
	"tftplinux=tftpboot $loadaddr $bootfile; bootm\0" \
	"copyuboot=erase " MK_STR(CONFIG_UBOOTSTART) " " MK_STR(CONFIG_UBOOTEND) "; " \
		"cp.b fef00000 " MK_STR(CONFIG_UBOOTSTART) " 80000\0"	\
	"fdtaddr=400000\0"						\
	"fdtfile=mpc8349emitx.dtb\0"					\
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


#undef MK_STR
#undef XMK_STR

#endif
