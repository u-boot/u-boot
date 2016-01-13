/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/************************************************************************
 * canyonlands.h - configuration for Canyonlands (460EX)
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/kconfig.h>

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
/*
 * This config file is used for Canyonlands (460EX) Glacier (460GT)
 * and Arches dual (460GT)
 */
#ifdef CONFIG_CANYONLANDS
#define CONFIG_460EX			/* Specific PPC460EX		*/
#define CONFIG_HOSTNAME		canyonlands
#else
#define CONFIG_460GT			/* Specific PPC460GT		*/
#ifdef CONFIG_GLACIER
#define CONFIG_HOSTNAME		glacier
#else
#define CONFIG_HOSTNAME		arches
#define CONFIG_USE_NETDEV	eth1
#define CONFIG_BD_NUM_CPUS	2
#endif
#endif

#define CONFIG_440

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF80000
#endif

/*
 * Include common defines/options for all AMCC eval boards
 */
#include "amcc-common.h"

#define CONFIG_SYS_CLK_FREQ	66666667	/* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F		/* Call board_early_init_f */
#define CONFIG_BOARD_EARLY_INIT_R		/* Call board_early_init_r */
#define CONFIG_MISC_INIT_R			/* Call misc_init_r */
#define CONFIG_BOARD_TYPES			/* support board types */

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped PCI memory	*/
#define CONFIG_SYS_PCI_BASE		0xd0000000	/* internal PCI regs	*/
#define CONFIG_SYS_PCI_TARGBASE	CONFIG_SYS_PCI_MEMBASE

#define CONFIG_SYS_PCIE_MEMBASE	0xb0000000	/* mapped PCIe memory	*/
#define CONFIG_SYS_PCIE_MEMSIZE	0x08000000	/* smallest incr for PCIe port */
#define CONFIG_SYS_PCIE_BASE		0xc4000000	/* PCIe UTL regs */

#define CONFIG_SYS_PCIE0_CFGBASE	0xc0000000
#define CONFIG_SYS_PCIE1_CFGBASE	0xc1000000
#define CONFIG_SYS_PCIE0_XCFGBASE	0xc3000000
#define CONFIG_SYS_PCIE1_XCFGBASE	0xc3001000

/*
 * BCSR bits as defined in the Canyonlands board user manual.
 */
#define BCSR_USBCTRL_OTG_RST	0x32
#define BCSR_USBCTRL_HOST_RST	0x01
#define BCSR_SELECT_PCIE	0x10

#define	CONFIG_SYS_PCIE0_UTLBASE	0xc08010000ULL	/* 36bit physical addr	*/

/* base address of inbound PCIe window */
#define CONFIG_SYS_PCIE_INBOUND_BASE	0x000000000ULL	/* 36bit physical addr	*/

/* EBC stuff */
#if !defined(CONFIG_ARCHES)
#define CONFIG_SYS_BCSR_BASE		0xE1000000
#define CONFIG_SYS_FLASH_BASE		0xFC000000	/* later mapped to this addr */
#define CONFIG_SYS_FLASH_SIZE		(64 << 20)
#else
#define CONFIG_SYS_FPGA_BASE		0xE1000000
#define CONFIG_SYS_CPLD_ADDR		(CONFIG_SYS_FPGA_BASE + 0x00080000)
#define CONFIG_SYS_CPLD_DATA		(CONFIG_SYS_FPGA_BASE + 0x00080002)
#define CONFIG_SYS_FLASH_BASE		0xFE000000	/* later mapped to this addr  */
#define CONFIG_SYS_FLASH_SIZE		(32 << 20)
#endif

#define CONFIG_SYS_NAND_ADDR		0xE0000000
#define CONFIG_SYS_BOOT_BASE_ADDR	0xFF000000	/* EBC Boot Space: 0xFF000000 */
#define CONFIG_SYS_FLASH_BASE_PHYS_H	0x4
#define CONFIG_SYS_FLASH_BASE_PHYS_L	0xCC000000
#define CONFIG_SYS_FLASH_BASE_PHYS	(((u64)CONFIG_SYS_FLASH_BASE_PHYS_H << 32) |	\
					 (u64)CONFIG_SYS_FLASH_BASE_PHYS_L)

#define CONFIG_SYS_OCM_BASE		0xE3000000	/* OCM: 64k		*/
#define CONFIG_SYS_SRAM_BASE		0xE8000000	/* SRAM: 256k		*/
#define CONFIG_SYS_SRAM_SIZE		(256 << 10)
#define CONFIG_SYS_LOCAL_CONF_REGS	0xEF000000

#define CONFIG_SYS_AHB_BASE		0xE2000000	/* internal AHB peripherals	*/

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in OCM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_BASE	/* OCM			*/
#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/*
 * Define here the location of the environment variables (FLASH).
 */
#define	CONFIG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */
#define CONFIG_SYS_NOR_CS		0	/* NOR chip connected to CSx */
#define CONFIG_SYS_NAND_CS		3	/* NAND chip connected to CSx */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/
#define CONFIG_SYS_FLASH_CFI_AMD_RESET	/* Use AMD (Spansion) reset cmd */

#define CONFIG_SYS_FLASH_BANKS_LIST    {CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000		/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * NAND-FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_NAND_ADDR + CONFIG_SYS_NAND_CS)
#define CONFIG_SYS_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/

/*------------------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------------*/
#if !defined(CONFIG_ARCHES)
/*
 * NAND booting U-Boot version uses a fixed initialization, since the whole
 * I2C SPD DIMM autodetection/calibration doesn't fit into the 4k of boot
 * code.
 */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS	{0x50, 0x51}	/* SPD i2c spd addresses*/
#define CONFIG_DDR_ECC			/* with ECC support		*/
#define CONFIG_DDR_RQDC_FIXED	0x80000038 /* fixed value for RQDC	*/

#else /* defined(CONFIG_ARCHES) */

#define CONFIG_AUTOCALIB	"silent\0"	/* default is non-verbose    */

#define CONFIG_PPC4xx_DDR_AUTOCALIBRATION	/* IBM DDR autocalibration   */
#define DEBUG_PPC4xx_DDR_AUTOCALIBRATION	/* dynamic DDR autocal debug */
#undef CONFIG_PPC4xx_DDR_METHOD_A

/* DDR1/2 SDRAM Device Control Register Data Values */
/* Memory Queue */
#define CONFIG_SYS_SDRAM_R0BAS		0x0000f000
#define CONFIG_SYS_SDRAM_R1BAS		0x00000000
#define CONFIG_SYS_SDRAM_R2BAS		0x00000000
#define CONFIG_SYS_SDRAM_R3BAS		0x00000000
#define CONFIG_SYS_SDRAM_PLBADDULL	0x00000000
#define CONFIG_SYS_SDRAM_PLBADDUHB	0x00000008
#define CONFIG_SYS_SDRAM_CONF1LL	0x00001080
#define CONFIG_SYS_SDRAM_CONF1HB	0x00001080
#define CONFIG_SYS_SDRAM_CONFPATHB	0x10a68000

/* SDRAM Controller */
#define CONFIG_SYS_SDRAM0_MB0CF		0x00000701
#define CONFIG_SYS_SDRAM0_MB1CF		0x00000000
#define CONFIG_SYS_SDRAM0_MB2CF		0x00000000
#define CONFIG_SYS_SDRAM0_MB3CF		0x00000000
#define CONFIG_SYS_SDRAM0_MCOPT1	0x05322000
#define CONFIG_SYS_SDRAM0_MCOPT2	0x00000000
#define CONFIG_SYS_SDRAM0_MODT0		0x01000000
#define CONFIG_SYS_SDRAM0_MODT1		0x00000000
#define CONFIG_SYS_SDRAM0_MODT2		0x00000000
#define CONFIG_SYS_SDRAM0_MODT3		0x00000000
#define CONFIG_SYS_SDRAM0_CODT		0x00800021
#define CONFIG_SYS_SDRAM0_RTR		0x06180000
#define CONFIG_SYS_SDRAM0_INITPLR0	0xb5380000
#define CONFIG_SYS_SDRAM0_INITPLR1	0x82100400
#define CONFIG_SYS_SDRAM0_INITPLR2	0x80820000
#define CONFIG_SYS_SDRAM0_INITPLR3	0x80830000
#define CONFIG_SYS_SDRAM0_INITPLR4	0x80810040
#define CONFIG_SYS_SDRAM0_INITPLR5	0x80800532
#define CONFIG_SYS_SDRAM0_INITPLR6	0x82100400
#define CONFIG_SYS_SDRAM0_INITPLR7	0x8a080000
#define CONFIG_SYS_SDRAM0_INITPLR8	0x8a080000
#define CONFIG_SYS_SDRAM0_INITPLR9	0x8a080000
#define CONFIG_SYS_SDRAM0_INITPLR10	0x8a080000
#define CONFIG_SYS_SDRAM0_INITPLR11	0x80000432
#define CONFIG_SYS_SDRAM0_INITPLR12	0x808103c0
#define CONFIG_SYS_SDRAM0_INITPLR13	0x80810040
#define CONFIG_SYS_SDRAM0_INITPLR14	0x00000000
#define CONFIG_SYS_SDRAM0_INITPLR15	0x00000000
#define CONFIG_SYS_SDRAM0_RQDC		0x80000038
#define CONFIG_SYS_SDRAM0_RFDC		0x00000257
#define CONFIG_SYS_SDRAM0_RDCC		0x40000000
#define CONFIG_SYS_SDRAM0_DLCR		0x03000091
#define CONFIG_SYS_SDRAM0_CLKTR		0x40000000
#define CONFIG_SYS_SDRAM0_WRDTR		0x82000823
#define CONFIG_SYS_SDRAM0_SDTR1		0x80201000
#define CONFIG_SYS_SDRAM0_SDTR2		0x42204243
#define CONFIG_SYS_SDRAM0_SDTR3		0x090c0d1a
#define CONFIG_SYS_SDRAM0_MMODE		0x00000432
#define CONFIG_SYS_SDRAM0_MEMODE	0x00000004
#endif	/* !defined(CONFIG_ARCHES) */

#define CONFIG_SYS_MBYTES_SDRAM	512	/* 512MB			*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000

#define CONFIG_SYS_I2C_EEPROM_ADDR		(0xa8>>1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/* I2C bootstrap EEPROM */
#if defined(CONFIG_ARCHES)
#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR	0x54
#else
#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR	0x52
#endif
#define CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET	0
#define CONFIG_4xx_CONFIG_BLOCKSIZE		16

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75				/* ON Semi's LM75	*/
#define CONFIG_DTT_AD7414			/* use AD7414		*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3

#if defined(CONFIG_ARCHES)
#define CONFIG_SYS_I2C_DTT_ADDR	0x4a		/* AD7414 I2C address	*/
#endif

#if !defined(CONFIG_ARCHES)
/* RTC configuration */
#define CONFIG_RTC_M41T62
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#endif

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_IBM_EMAC4_V4

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1

#if !defined(CONFIG_ARCHES)
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR	1
/* Only Glacier (460GT) has 4 EMAC interfaces */
#ifdef CONFIG_460GT
#define CONFIG_PHY2_ADDR	2
#define CONFIG_PHY3_ADDR	3
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#endif

#else /* defined(CONFIG_ARCHES) */

#define CONFIG_FIXED_PHY	0xFFFFFFFF
#define CONFIG_PHY_ADDR		CONFIG_FIXED_PHY
#define CONFIG_PHY1_ADDR	0
#define CONFIG_PHY2_ADDR	1
#define CONFIG_HAS_ETH2

#define CONFIG_SYS_FIXED_PHY_PORT(devnum, speed, duplex) \
		{devnum, speed, duplex}
#define CONFIG_SYS_FIXED_PHY_PORTS \
		CONFIG_SYS_FIXED_PHY_PORT(0, 1000, FULL)

#define CONFIG_M88E1112_PHY

/*
 * For the GPCS_PHYx_ADDR PHY address, choose some PHY address not
 * used by CONFIG_PHYx_ADDR
 */
#define CONFIG_GPCS_PHY_ADDR    0xA
#define CONFIG_GPCS_PHY1_ADDR   0xB
#define CONFIG_GPCS_PHY2_ADDR   0xC
#endif	/* !defined(CONFIG_ARCHES) */

#define CONFIG_PHY_RESET		/* reset phy upon startup	*/
#define CONFIG_PHY_GIGE			/* Include GbE speed/duplex detection */
#define CONFIG_PHY_DYNAMIC_ANEG

/*-----------------------------------------------------------------------
 * USB-OHCI
 *----------------------------------------------------------------------*/
/* Only Canyonlands (460EX) has USB */
#ifdef CONFIG_460EX
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#undef CONFIG_SYS_OHCI_BE_CONTROLLER		/* 460EX has little endian descriptors	*/
#define CONFIG_SYS_OHCI_SWAP_REG_ACCESS	/* 460EX has little endian register	*/
#define CONFIG_SYS_OHCI_USE_NPS		/* force NoPowerSwitching mode		*/
#define CONFIG_SYS_USB_OHCI_REGS_BASE	(CONFIG_SYS_AHB_BASE | 0xd0000)
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"ppc440"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 15
#define CONFIG_SYS_USB_OHCI_BOARD_INIT
#endif

/*
 * Default environment variables
 */
#if !defined(CONFIG_ARCHES)
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fc000000\0"					\
	"fdt_addr=fc1e0000\0"						\
	"ramdisk_addr=fc200000\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:RP\0"						\
	""
#else /* defined(CONFIG_ARCHES) */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fe000000\0"					\
	"fdt_addr=fe1e0000\0"						\
	"ramdisk_addr=fe200000\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:RP\0"						\
	"ethprime=ppc_4xx_eth1\0"					\
	""
#endif	/* !defined(CONFIG_ARCHES) */

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_CHIP_CONFIG
#if defined(CONFIG_ARCHES)
#define CONFIG_CMD_DTT
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#elif defined(CONFIG_CANYONLANDS)
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SATA
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB
#elif defined(CONFIG_GLACIER)
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DTT
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP
#else
#error "board type not defined"
#endif

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP			/* do pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT		/* let board init pci target    */
#undef	CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1014	/* IBM				*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever			*/

#ifdef CONFIG_460GT
#if defined(CONFIG_ARCHES)
/*-----------------------------------------------------------------------
 * RapidIO I/O and Registers
 *----------------------------------------------------------------------*/
#define CONFIG_RAPIDIO
#define CONFIG_SYS_460GT_SRIO_ERRATA_1

#define SRGPL0_REG_BAR		0x0000000DAA000000ull	/*  16MB */
#define SRGPL0_CFG_BAR		0x0000000DAB000000ull	/*  16MB */
#define SRGPL0_MNT_BAR		0x0000000DAC000000ull	/*  16MB */
#define SRGPL0_MSG_BAR		0x0000000DAD000000ull	/*  16MB */
#define SRGPL0_OUT_BAR		0x0000000DB0000000ull	/* 256MB */

#define CONFIG_SYS_SRGPL0_REG_BAR	0xAA000000		/*  16MB */
#define CONFIG_SYS_SRGPL0_CFG_BAR	0xAB000000		/*  16MB */
#define CONFIG_SYS_SRGPL0_MNT_BAR	0xAC000000		/*  16MB */
#define CONFIG_SYS_SRGPL0_MSG_BAR	0xAD000000		/*  16MB */

#define CONFIG_SYS_I2ODMA_BASE		0xCF000000
#define CONFIG_SYS_I2ODMA_PHYS_ADDR	0x0000000400100000ull

#define CONFIG_PPC4XX_RAPIDIO_PROMISCUOUS_MODE
#undef CONFIG_PPC4XX_RAPIDIO_DEBUG
#undef CONFIG_PPC4XX_RAPIDIO_IN_BAR_USE_OCM
#define CONFIG_PPC4XX_RAPIDIO_USE_HB_PLB
#undef CONFIG_PPC4XX_RAPIDIO_LOOPBACK
#endif /* CONFIG_ARCHES */
#endif /* CONFIG_460GT */

/*
 * SATA driver setup
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SATA_DWC
#define CONFIG_LIBATA
#define SATA_BASE_ADDR		0xe20d1000	/* PPC460EX SATA Base Address */
#define SATA_DMA_REG_ADDR	0xe20d0800	/* PPC460EX SATA Base Address */
#define CONFIG_SYS_SATA_MAX_DEVICE	1	/* SATA MAX DEVICE */
/* Convert sectorsize to wordsize */
#define ATA_SECTOR_WORDS (ATA_SECT_SIZE/2)
#endif

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/*
 * Canyonlands has 64MBytes of NOR FLASH (Spansion 29GL512), but the
 * boot EBC mapping only supports a maximum of 16MBytes
 * (4.ff00.0000 - 4.ffff.ffff).
 * To solve this problem, the FLASH has to get remapped to another
 * EBC address which accepts bigger regions:
 *
 * 0xfc00.0000 -> 4.cc00.0000
 *
 * Arches has 32MBytes of NOR FLASH (Spansion 29GL256), it will be
 * remapped to:
 *
 * 0xfe00.0000 -> 4.ce00.0000
 */

/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CONFIG_SYS_EBC_PB0AP		0x10055e00
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_BOOT_BASE_ADDR | 0x9a000)

#if !defined(CONFIG_ARCHES)
/* Memory Bank 3 (NAND-FLASH) initialization						*/
#define CONFIG_SYS_EBC_PB3AP		0x018003c0
#define CONFIG_SYS_EBC_PB3CR		(CONFIG_SYS_NAND_ADDR | 0x1E000) /* BAS=NAND,BS=1MB,BU=R/W,BW=32bit*/
#endif

#if !defined(CONFIG_ARCHES)
/* Memory Bank 2 (CPLD) initialization						*/
#define CONFIG_SYS_EBC_PB2AP		0x00804240
#define CONFIG_SYS_EBC_PB2CR		(CONFIG_SYS_BCSR_BASE | 0x18000) /* BAS=CPLD,BS=1M,BU=RW,BW=32bit */

#else /* defined(CONFIG_ARCHES) */

/* Memory Bank 1 (FPGA) initialization  */
#define CONFIG_SYS_EBC_PB1AP		0x7f8ffe80
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_FPGA_BASE | 0x3a000) /* BAS=FPGA,BS=2MB,BU=R/W,BW=16bit*/
#endif	/* !defined(CONFIG_ARCHES) */

#define CONFIG_SYS_EBC_CFG		0xbfc00000

/*
 * Arches doesn't use PerCS3 but GPIO43, so let's configure the GPIO
 * pin multiplexing correctly
 */
#if defined(CONFIG_ARCHES)
#define GPIO43_USE		GPIO_SEL	/* On Arches this pin is used as GPIO */
#else
#define GPIO43_USE		GPIO_ALT1	/* On Glacier this pin is used as ALT1 -> PerCS3 */
#endif

/*
 * PPC4xx GPIO Configuration
 */
#ifdef CONFIG_460EX
/* 460EX: Use USB configuration */
#define CONFIG_SYS_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO0	GMC1TxD(0)	USB2HostD(0)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO1	GMC1TxD(1)	USB2HostD(1)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO2	GMC1TxD(2)	USB2HostD(2)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO3	GMC1TxD(3)	USB2HostD(3)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO4	GMC1TxD(4)	USB2HostD(4)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO5	GMC1TxD(5)	USB2HostD(5)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO6	GMC1TxD(6)	USB2HostD(6)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO7	GMC1TxD(7)	USB2HostD(7)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	GMC1RxD(0)	USB2OTGD(0)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	GMC1RxD(1)	USB2OTGD(1)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 GMC1RxD(2)	USB2OTGD(2)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO11 GMC1RxD(3)	USB2OTGD(3)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO12 GMC1RxD(4)	USB2OTGD(4)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO13 GMC1RxD(5)	USB2OTGD(5)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO14 GMC1RxD(6)	USB2OTGD(6)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO15 GMC1RxD(7)	USB2OTGD(7)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL,  GPIO_OUT_0}, /* GPIO16 GMC1TxER	USB2HostStop	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 GMC1CD		USB2HostNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 GMC1RxER	USB2HostDir	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO19 GMC1TxEN	USB2OTGStop	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO20 GMC1CRS	USB2OTGNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO21 GMC1RxDV	USB2OTGDir	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO22 NFRDY				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO23 NFREN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO24 NFWEN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO25 NFCLE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO26 NFALE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO27 IRQ(0)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO28 IRQ(1)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO29 IRQ(2)				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO30 PerPar0	DMAReq2		IRQ(7)*/ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO31 PerPar1	DMAAck2		IRQ(8)*/ \
},											\
{											\
/* GPIO Core 1 */									\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO32 PerPar2	EOT2/TC2	IRQ(9)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO33 PerPar3	DMAReq3		IRQ(4)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_1}, /* GPIO34 UART0_DCD_N	UART1_DSR_CTS_N	UART2_SOUT*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO35 UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO36 UART0_8PIN_CTS_N DMAAck3	UART3_SIN*/ \
{GPIO1_BASE, GPIO_BI , GPIO_ALT2, GPIO_OUT_0}, /* GPIO37 UART0_RTS_N	EOT3/TC3	UART3_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_1}, /* GPIO38 UART0_DTR_N	UART1_SOUT	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO39 UART0_RI_N	UART1_SIN	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO40 IRQ(3)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO41 CS(1)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO42 CS(2)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO43 CS(3)		DMAReq1		IRQ(10)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO44 CS(4)		DMAAck1		IRQ(11)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO45 CS(5)		EOT/TC1		IRQ(12)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO46 PerAddr(5)	DMAReq0		IRQ(13)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO47 PerAddr(6)	DMAAck0		IRQ(14)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO48 PerAddr(7)	EOT/TC0		IRQ(15)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO49  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO50  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO51  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO52  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO53  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO54  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO55  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO56  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO61  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO62  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO63  Unselect via TraceSelect Bit	*/	\
}											\
}
#else
/* 460GT: Use EMAC2+3 configuration */
#define CONFIG_SYS_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO0	GMC1TxD(0)	USB2HostD(0)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO1	GMC1TxD(1)	USB2HostD(1)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO2	GMC1TxD(2)	USB2HostD(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO3	GMC1TxD(3)	USB2HostD(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO4	GMC1TxD(4)	USB2HostD(4)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO5	GMC1TxD(5)	USB2HostD(5)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO6	GMC1TxD(6)	USB2HostD(6)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO7	GMC1TxD(7)	USB2HostD(7)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	GMC1RxD(0)	USB2OTGD(0)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	GMC1RxD(1)	USB2OTGD(1)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 GMC1RxD(2)	USB2OTGD(2)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO11 GMC1RxD(3)	USB2OTGD(3)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO12 GMC1RxD(4)	USB2OTGD(4)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO13 GMC1RxD(5)	USB2OTGD(5)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO14 GMC1RxD(6)	USB2OTGD(6)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO15 GMC1RxD(7)	USB2OTGD(7)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO16 GMC1TxER	USB2HostStop	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 GMC1CD		USB2HostNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 GMC1RxER	USB2HostDir	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO19 GMC1TxEN	USB2OTGStop	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO20 GMC1CRS	USB2OTGNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO21 GMC1RxDV	USB2OTGDir	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO22 NFRDY				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO23 NFREN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO24 NFWEN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO25 NFCLE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO26 NFALE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO27 IRQ(0)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO28 IRQ(1)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO29 IRQ(2)				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO30 PerPar0	DMAReq2		IRQ(7)*/ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO31 PerPar1	DMAAck2		IRQ(8)*/ \
},											\
{											\
/* GPIO Core 1 */									\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO32 PerPar2	EOT2/TC2	IRQ(9)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO33 PerPar3	DMAReq3		IRQ(4)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_1}, /* GPIO34 UART0_DCD_N	UART1_DSR_CTS_N	UART2_SOUT*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO35 UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO36 UART0_8PIN_CTS_N DMAAck3	UART3_SIN*/ \
{GPIO1_BASE, GPIO_BI , GPIO_ALT2, GPIO_OUT_0}, /* GPIO37 UART0_RTS_N	EOT3/TC3	UART3_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_1}, /* GPIO38 UART0_DTR_N	UART1_SOUT	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO39 UART0_RI_N	UART1_SIN	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO40 IRQ(3)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO41 CS(1)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO42 CS(2)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO43_USE, GPIO_OUT_0},/* GPIO43 CS(3)		DMAReq1		IRQ(10)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO44 CS(4)		DMAAck1		IRQ(11)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO45 CS(5)		EOT/TC1		IRQ(12)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO46 PerAddr(5)	DMAReq0		IRQ(13)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO47 PerAddr(6)	DMAAck0		IRQ(14)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO48 PerAddr(7)	EOT/TC0		IRQ(15)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO49  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO50  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO51  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO52  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO53  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO54  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO55  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO56  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO61  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO62  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO63  Unselect via TraceSelect Bit	*/	\
}											\
}
#endif

#endif	/* __CONFIG_H */
