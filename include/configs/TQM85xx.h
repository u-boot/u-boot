/*
 * (C) Copyright 2007
 * Thomas Waehner, TQ-System GmbH, thomas.waehner@tqs.de.
 *
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Wolfgang Denk <wd@denx.de>
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003 Motorola,Inc.
 * Xianghua Xiao <X.Xiao@motorola.com>
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
 * TQM85xx (8560/40/55/41/48) board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE			*/
#define CONFIG_E500		1	/* BOOKE e500 family		*/
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41		*/

#if defined(CONFIG_TQM8548_BE)
#define CONFIG_SYS_TEXT_BASE	0xfff80000
#else
#define CONFIG_SYS_TEXT_BASE	0xfffc0000
#endif

#if defined(CONFIG_TQM8548_AG) || defined(CONFIG_TQM8548_BE)
#define CONFIG_TQM8548
#endif

#define CONFIG_PCI
#ifndef CONFIG_TQM8548_AG
#define CONFIG_PCI1			/* PCI/PCI-X controller		*/
#endif
#ifdef CONFIG_TQM8548
#define CONFIG_PCIE1			/* PCI Express interface	*/
#endif

#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code	*/
#define CONFIG_PCIX_CHECK		/* PCIX olny works at 66 MHz	*/
#define CONFIG_FSL_PCIE_RESET	1	/* need PCIe reset errata	*/

#define CONFIG_TSEC_ENET		/* tsec ethernet support	*/

#define CONFIG_MISC_INIT_R	1	/* Call misc_init_r		*/

 /*
 * Configuration for big NOR Flashes
 *
 * Define CONFIG_TQM_BIGFLASH for boards with more than 128 MiB NOR Flash.
 * Please be aware, that this changes the whole memory map (new CCSRBAR
 * address, etc). You have to use an adapted Linux kernel or FDT blob
 * if this option is set.
 */
#undef CONFIG_TQM_BIGFLASH

/*
 * NAND flash support (disabled by default)
 *
 * Warning: NAND support will likely increase the U-Boot image size
 * to more than 256 KB. Please adjust CONFIG_SYS_TEXT_BASE if necessary.
 */
#ifdef CONFIG_TQM8548_BE
#define CONFIG_NAND
#endif

/*
 * MPC8540 and MPC8548 don't have CPM module
 */
#if !defined(CONFIG_MPC8540) && !defined(CONFIG_MPC8548)
#define CONFIG_CPM2		1	/* has CPM2			*/
#endif

#define CONFIG_FSL_LAW		1	/* Use common FSL init code	*/

#if defined(CONFIG_TQM8548_AG) || defined(CONFIG_TQM8548_BE)
#define	CONFIG_CAN_DRIVER		/* CAN Driver support		*/
#endif

/*
 * sysclk for MPC85xx
 *
 * Two valid values are:
 *    33333333
 *    66666666
 *
 * Most PCI cards are still 33Mhz, so in the presence of PCI, 33MHz
 * is likely the desired value here, so that is now the default.
 * The board, however, can run at 66MHz.  In any event, this value
 * must match the settings of some switches.  Details can be found
 * in the README.mpc85xxads.
 */

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	33333333
#endif

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache		*/
#define CONFIG_BTB			/* toggle branch predition	*/

#define CONFIG_SYS_INIT_DBCR DBCR_IDM		/* Enable Debug Exceptions	*/

#undef	CONFIG_SYS_DRAM_TEST			/* memory test, takes time	*/
#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END		0x10000000

#ifdef CONFIG_TQM_BIGFLASH
#define CONFIG_SYS_CCSRBAR	 	0xA0000000
#else
#define CONFIG_SYS_CCSRBAR		0xE0000000
#endif
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory	*/

#if defined(CONFIG_TQM_BIGFLASH) || \
	(!defined(CONFIG_TQM8548_AG) && !defined(CONFIG_TQM8548_BE))
#define CONFIG_SYS_PPC_DDR_WIMGE (MAS2_I | MAS2_G)
#define CONFIG_SYS_DDR_EARLY_SIZE_MB	(512)
#else
#define CONFIG_SYS_PPC_DDR_WIMGE (0)
#define CONFIG_SYS_DDR_EARLY_SIZE_MB	(2 * 1024)
#endif

#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#ifdef CONFIG_TQM8548_AG
#define CONFIG_VERY_BIG_RAM
#endif

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

#if defined(CONFIG_TQM8540) || defined(CONFIG_TQM8560)
/* TQM8540 & 8560 need DLL-override */
#define CONFIG_SYS_FSL_ERRATUM_DDR_MSYNC_IN	/* possible DLL fix needed */
#define CONFIG_DDR_DEFAULT_CL	25		/* CAS latency 2,5	*/
#endif /* CONFIG_TQM8540 || CONFIG_TQM8560 */

#if defined(CONFIG_TQM8541) || defined(CONFIG_TQM8555) || \
    defined(CONFIG_TQM8548)
#define CONFIG_DDR_DEFAULT_CL	30		/* CAS latency 3	*/
#endif /* CONFIG_TQM8541 || CONFIG_TQM8555 || CONFIG_TQM8548 */

/*
 * Flash on the Local Bus
 */
#ifdef CONFIG_TQM_BIGFLASH
#define CONFIG_SYS_FLASH0		0xE0000000
#define CONFIG_SYS_FLASH1		0xC0000000
#else /* !CONFIG_TQM_BIGFLASH */
#define CONFIG_SYS_FLASH0		0xFC000000
#define CONFIG_SYS_FLASH1		0xF8000000
#endif /* CONFIG_TQM_BIGFLASH */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH1, CONFIG_SYS_FLASH0 }

#define CONFIG_SYS_LBC_FLASH_BASE	CONFIG_SYS_FLASH1	/* Localbus flash start	*/
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_LBC_FLASH_BASE  /* start of FLASH	*/

/* Default ORx timings are for <= 41.7 MHz Local Bus Clock.
 *
 * Note: According to timing specifications external addr latch delay
 * (EAD, bit #0) must be set if Local Bus Clock is > 83 MHz.
 *
 * For other Local Bus Clocks see following table:
 *
 * Clock/MHz   CONFIG_SYS_ORx_PRELIM
 * 166         0x.....CA5
 * 133         0x.....C85
 * 100         0x.....C65
 *  83         0x.....FA2
 *  66         0x.....C82
 *  50         0x.....C60
 *  42         0x.....040
 *  33         0x.....030
 *  25         0x.....020
 *
 */
#ifdef CONFIG_TQM_BIGFLASH
#define CONFIG_SYS_BR0_PRELIM		0xE0001801	/* port size 32bit	*/
#define CONFIG_SYS_OR0_PRELIM		0xE0000040	/* 512MB Flash		*/
#define CONFIG_SYS_BR1_PRELIM		0xC0001801	/* port size 32bit	*/
#define CONFIG_SYS_OR1_PRELIM		0xE0000040	/* 512MB Flash		*/
#else /* !CONFIG_TQM_BIGFLASH */
#define CONFIG_SYS_BR0_PRELIM		0xfc001801	/* port size 32bit	*/
#define CONFIG_SYS_OR0_PRELIM		0xfc000040	/* 64MB Flash		*/
#define CONFIG_SYS_BR1_PRELIM		0xf8001801	/* port size 32bit	*/
#define CONFIG_SYS_OR1_PRELIM		0xfc000040	/* 64MB Flash		*/
#endif /* CONFIG_TQM_BIGFLASH */

#define CONFIG_SYS_FLASH_CFI			/* flash is CFI compat.		*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector	*/
#define CONFIG_SYS_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash*/
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1 /* speed up output to Flash	*/

#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* number of banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* sectors per device		*/
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms)	*/

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor	*/

/*
 * Note: when changing the Local Bus clock divider you have to
 * change the timing values in CONFIG_SYS_ORx_PRELIM.
 *
 * LCRR[00:03] CLKDIV: System (CCB) clock divider. Valid values are 2, 4, 8.
 * LCRR[16:17] EADC  : External address delay cycles. It should be set to 2
 *                     for Local Bus Clock > 83.3 MHz.
 */
#define CONFIG_SYS_LBC_LCRR		0x00030008	/* LB clock ratio reg	*/
#define CONFIG_SYS_LBC_LBCR		0x00000000	/* LB config reg	*/
#define CONFIG_SYS_LBC_LSRT		0x20000000	/* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR		0x20000000	/* LB refresh timer presc.*/

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_CCSRBAR \
				 + 0x04010000)	/* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size used area in RAM	*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(~CONFIG_SYS_TEXT_BASE + 1)/* Reserved for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(384 * 1024)	/* Reserved for malloc	*/

/* Serial Port */
#if defined(CONFIG_TQM8560)

#define CONFIG_CONS_ON_SCC	/* define if console on SCC		*/
#undef	CONFIG_CONS_NONE	/* define if console on something else	*/
#define CONFIG_CONS_INDEX	1 /* which serial channel for console	*/

#else /* !CONFIG_TQM8560 */

#define CONFIG_CONS_INDEX     1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* PS/2 Keyboard */
#define CONFIG_PS2KBD			/* AT-PS/2 Keyboard		*/
#define CONFIG_PS2MULT			/* .. on PS/2 Multiplexer	*/
#define CONFIG_PS2SERIAL	2	/* .. on DUART2			*/
#define CONFIG_PS2MULT_DELAY	(CONFIG_SYS_HZ/2)	/* Initial delay	*/
#define CONFIG_BOARD_EARLY_INIT_R	1

#endif /* CONFIG_TQM8560 */

#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_AUTO_COMPLETE	1	/* add autocompletion support */
#define CONFIG_SYS_HUSH_PARSER		1	/* Use the HUSH parser		*/
#ifdef	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/* CAN */
#define CONFIG_SYS_CAN_BASE		(CONFIG_SYS_CCSRBAR \
				 + 0x03000000)	/* CAN base address     */
#ifdef CONFIG_CAN_DRIVER
#define CONFIG_SYS_CAN_OR_AM		0xFFFF8000	/* 32 KiB address mask  */
#define CONFIG_SYS_OR2_CAN		(CONFIG_SYS_CAN_OR_AM | OR_UPM_BI)
#define CONFIG_SYS_BR2_CAN		((CONFIG_SYS_CAN_BASE & BR_BA) | \
				 BR_PS_8 | BR_MS_UPMC | BR_V)
#endif /* CONFIG_CAN_DRIVER */

/*
 * I2C
 */
#define CONFIG_FSL_I2C			/* Use FSL common I2C driver	*/
#define CONFIG_HARD_I2C			/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_NOPROBES	{0x48}	/* Don't probe these addrs	*/
#define CONFIG_SYS_I2C_OFFSET		0x3000

/* I2C RTC */
#define CONFIG_RTC_DS1337		/* Use ds1337 rtc via i2c	*/
#define CONFIG_SYS_I2C_RTC_ADDR	0x68	/* at address 0x68		*/

/* I2C EEPROM */
/*
 * EEPROM configuration for onboard EEPROM M24C32 (M24C64 should work also).
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* =32 Bytes per write	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20
#define CONFIG_SYS_I2C_MULTI_EEPROMS		1	/* more than one eeprom	*/

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75		1		/* ON Semi's LM75	*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3

#ifndef CONFIG_PCIE1
/* RapidIO MMU */
#ifdef CONFIG_TQM_BIGFLASH
#define CONFIG_SYS_RIO_MEM_BASE	0xb0000000	/* base address		*/
#define CONFIG_SYS_RIO_MEM_SIZE	0x10000000	/* 256M			*/
#else /* !CONFIG_TQM_BIGFLASH */
#define CONFIG_SYS_RIO_MEM_BASE	0xc0000000	/* base address		*/
#define CONFIG_SYS_RIO_MEM_SIZE	0x20000000	/* 512M			*/
#endif /* CONFIG_TQM_BIGFLASH */
#define CONFIG_SYS_RIO_MEM_PHYS	CONFIG_SYS_RIO_MEM_BASE
#endif /* CONFIG_PCIE1 */

/* NAND FLASH */
#ifdef CONFIG_NAND

#define CONFIG_NAND_FSL_UPM	1

#define	CONFIG_MTD_NAND_ECC_JFFS2	1	/* use JFFS2 ECC	*/

/* address distance between chip selects */
#define	CONFIG_SYS_NAND_SELECT_DEVICE	1
#define	CONFIG_SYS_NAND_CS_DIST	0x200

#define CONFIG_SYS_NAND_SIZE		0x8000
#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_CCSRBAR + 0x03010000)

#define CONFIG_SYS_MAX_NAND_DEVICE	1	/* Max number of NAND devices	*/
#define CONFIG_SYS_NAND_MAX_CHIPS	2	/* Number of chips per device	*/

/* CS3 for NAND Flash */
#define CONFIG_SYS_BR3_PRELIM		((CONFIG_SYS_NAND_BASE & BR_BA) | \
					 BR_PS_8 | BR_MS_UPMB | BR_V)
#define CONFIG_SYS_OR3_PRELIM		(P2SZ_TO_AM(CONFIG_SYS_NAND_SIZE) | OR_UPM_BI)

#define NAND_BIG_DELAY_US		25	/* max tR for Samsung devices	*/

#endif /* CONFIG_NAND */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BUS
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M			*/
#define CONFIG_SYS_PCI1_IO_BUS	(CONFIG_SYS_CCSRBAR + 0x02000000)
#define CONFIG_SYS_PCI1_IO_PHYS	CONFIG_SYS_PCI1_IO_BUS
#define CONFIG_SYS_PCI1_IO_SIZE	0x1000000	/*  16M			*/

#ifdef CONFIG_PCIE1
/*
 * General PCI express
 * Addresses are mapped 1-1.
 */
#ifdef CONFIG_TQM_BIGFLASH
#define CONFIG_SYS_PCIE1_MEM_BUS	0xb0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x10000000      /* 512M                 */
#define CONFIG_SYS_PCIE1_IO_BUS		0xaf000000
#else /* !CONFIG_TQM_BIGFLASH */
#define CONFIG_SYS_PCIE1_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000      /* 512M                 */
#define CONFIG_SYS_PCIE1_IO_BUS		0xef000000
#endif /* CONFIG_TQM_BIGFLASH */
#define CONFIG_SYS_PCIE1_MEM_PHYS	CONFIG_SYS_PCIE1_MEM_BUS
#define CONFIG_SYS_PCIE1_IO_PHYS	CONFIG_SYS_PCIE1_IO_BUS
#define CONFIG_SYS_PCIE1_IO_SIZE	0x1000000       /* 16M                  */
#endif /* CONFIG_PCIE1 */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/

#define CONFIG_EEPRO100
#undef CONFIG_TULIP

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup	*/
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1057	/* Motorola			*/

#endif /* CONFIG_PCI */


#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR		2
#define TSEC2_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define FEC_PHY_ADDR		3
#define FEC_PHYIDX		0
#define FEC_FLAGS		0
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2

#ifdef CONFIG_TQM8548
/*
 * TQM8548 has 4 ethernet ports. 4 ETSEC's.
 *
 * On the STK85xx Starterkit the ETSEC3/4 ports are on an
 * additional adapter (AIO) between module and Starterkit.
 */
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"TSEC2"
#define CONFIG_TSEC4	1
#define CONFIG_TSEC4_NAME	"TSEC3"
#define TSEC3_PHY_ADDR		4
#define TSEC4_PHY_ADDR		5
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC4_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define CONFIG_HAS_ETH3
#define CONFIG_HAS_ETH4
#endif	/* CONFIG_TQM8548 */

/* Options are TSEC[0-1], FEC */
#define CONFIG_ETHPRIME		"TSEC0"

#if defined(CONFIG_TQM8540)
/*
 * TQM8540 has 3 ethernet ports. 2 TSEC's and one FEC.
 * The FEC port is connected on the same signals as the FCC3 port
 * of the TQM8560 to the baseboard (STK85xx Starterkit).
 *
 * On the STK85xx Starterkit the X47/X50 jumper has to be set to
 * a - d (X50.2 - 3) to enable the FEC port.
 */
#define CONFIG_MPC85XX_FEC	1
#define CONFIG_MPC85XX_FEC_NAME	"FEC"
#endif

#if defined(CONFIG_TQM8541) || defined(CONFIG_TQM8555)
/*
 * TQM8541/55 have 4 ethernet ports. 2 TSEC's and 2 FCC's. Only one FCC port
 * can be used at once, since only one FCC port is available on the STK85xx
 * Starterkit.
 *
 * To use this port you have to configure U-Boot to use the FCC port 1...2
 * and set the X47/X50 jumper to:
 * FCC1: a - b (X47.2 - X50.2)
 * FCC2: a - c (X50.2 - 1)
 */
#define CONFIG_ETHER_ON_FCC
#define	CONFIG_ETHER_INDEX    1	/* FCC channel for ethernet	*/
#endif

#if defined(CONFIG_TQM8560)
/*
 * TQM8560 has 5 ethernet ports. 2 TSEC's and 3 FCC's. Only one FCC port
 * can be used at once, since only one FCC port is available on the STK85xx
 * Starterkit.
 *
 * To use this port you have to configure U-Boot to use the FCC port 1...3
 * and set the X47/X50 jumper to:
 * FCC1: a - b (X47.2 - X50.2)
 * FCC2: a - c (X50.2 - 1)
 * FCC3: a - d (X50.2 - 3)
 */
#define CONFIG_ETHER_ON_FCC
#define	CONFIG_ETHER_INDEX    3	/* FCC channel for ethernet	*/
#endif

#if defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 1)
#define CONFIG_ETHER_ON_FCC1
#define CONFIG_SYS_CMXFCR_MASK1	(CMXFCR_FC1 | CMXFCR_RF1CS_MSK | \
				 CMXFCR_TF1CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE1	(CMXFCR_RF1CS_CLK11 | CMXFCR_TF1CS_CLK12)
#define CONFIG_SYS_CPMFCR_RAMTYPE	0
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)
#endif

#if defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 2)
#define CONFIG_ETHER_ON_FCC2
#define CONFIG_SYS_CMXFCR_MASK2	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | \
				 CMXFCR_TF2CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE2	(CMXFCR_RF2CS_CLK16 | CMXFCR_TF2CS_CLK13)
#define CONFIG_SYS_CPMFCR_RAMTYPE	0
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)
#endif

#if defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 3)
#define CONFIG_ETHER_ON_FCC3
#define CONFIG_SYS_CMXFCR_MASK3	(CMXFCR_FC3 | CMXFCR_RF3CS_MSK | \
				 CMXFCR_TF3CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE3	(CMXFCR_RF3CS_CLK15 | CMXFCR_TF3CS_CLK14)
#define CONFIG_SYS_CPMFCR_RAMTYPE	0
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)
#endif

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1

#define CONFIG_ENV_SECT_SIZE	0x40000 /* 256K (one sector) for env	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define	CONFIG_TIMESTAMP	/* Print image info with ts	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

#ifdef CONFIG_NAND
/*
 * Use NAND-FLash as JFFS2 device
 */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_JFFS2

#define	CONFIG_JFFS2_NAND	1

#ifdef CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nand0=TQM85xx-nand"
#define MTDPARTS_DEFAULT	"mtdparts=TQM85xx-nand:-"
#else
#define CONFIG_JFFS2_DEV 	"nand0"	/* NAND device jffs2 lives on	*/
#define CONFIG_JFFS2_PART_OFFSET 0	/* start of jffs2 partition	*/
#define CONFIG_JFFS2_PART_SIZE	0x200000 /* size of jffs2 partition	*/
#endif /* CONFIG_CMD_MTDPARTS */

#endif /* CONFIG_NAND */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP
#ifndef CONFIG_TQM8548_AG
#define CONFIG_CMD_DATE
#endif
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_DTT
#define CONFIG_CMD_MII
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif

#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
			 sizeof(CONFIG_SYS_PROMPT) + 16)   /* Print Buf Size	*/
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux	*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port*/
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use	*/
#endif

#define CONFIG_LOADADDR	 200000		/* default addr for tftp & bootm*/

#define CONFIG_BOOTDELAY 5		/* -1 disables auto-boot	*/

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs	*/


/*
 * Setup some board specific values for the default environment variables
 */
#ifdef CONFIG_CPM2
#define CONFIG_ENV_CONSDEV		"consdev=ttyCPM0\0"
#else
#define CONFIG_ENV_CONSDEV		"consdev=ttyS0\0"
#endif
#define CONFIG_ENV_FDT_FILE	"fdt_file="MK_STR(CONFIG_HOSTNAME)"/" \
				MK_STR(CONFIG_HOSTNAME)".dtb\0"
#define CONFIG_ENV_BOOTFILE	"bootfile="MK_STR(CONFIG_HOSTNAME)"/uImage\0"
#define CONFIG_ENV_UBOOT		"uboot="MK_STR(CONFIG_HOSTNAME)"/u-boot.bin\0" \
				"uboot_addr="MK_STR(CONFIG_SYS_TEXT_BASE)"\0"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_ENV_BOOTFILE						\
	CONFIG_ENV_FDT_FILE						\
	CONFIG_ENV_CONSDEV						\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=$serverip:$rootpath\0"				\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs $bootargs "				\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask"		\
		":$hostname:$netdev:off panic=1\0"			\
	"addcons=setenv bootargs $bootargs "				\
		"console=$consdev,$baudrate\0"				\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm $kernel_addr - $fdt_addr\0"			\
	"flash_self=run ramargs addip addcons;"				\
		"bootm $kernel_addr $ramdisk_addr $fdt_addr\0"		\
	"net_nfs=tftp $kernel_addr_r $bootfile;"       			\
		"tftp $fdt_addr_r $fdt_file;"				\
		"run nfsargs addip addcons;"				\
		"bootm $kernel_addr_r - $fdt_addr_r\0"    		\
	"rootpath=/opt/eldk/ppc_85xx\0"					\
	"fdt_addr_r=900000\0"						\
	"kernel_addr_r=1000000\0"      					\
	"fdt_addr=ffec0000\0"						\
	"kernel_addr=ffd00000\0"					\
	"ramdisk_addr=ff800000\0"					\
	CONFIG_ENV_UBOOT						\
	"load=tftp 100000 $uboot\0"					\
	"update=protect off $uboot_addr +$filesize;"			\
		"erase $uboot_addr +$filesize;"				\
		"cp.b 100000 $uboot_addr $filesize"			\
	"upd=run load update\0"						\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#endif /* __CONFIG_H */
