/*
 * (C) Copyright 2009
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * Based on include/configs/canyonlands.h
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * intip.h - configuration for CompactCenter aka intip (460EX) and DevCon-Center
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
/*
 * This config file is used for CompactCenter(codename intip) and DevCon-Center
 */
#define CONFIG_460EX		1	/* Specific PPC460EX		*/
#ifdef CONFIG_DEVCONCENTER
#define CONFIG_HOSTNAME		devconcenter
#define CONFIG_IDENT_STRING	" devconcenter 0.06"
#else
#define CONFIG_HOSTNAME		intip
#define CONFIG_IDENT_STRING	" intip 0.06"
#endif
#define CONFIG_440		1

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFFA0000
#endif

/*
 * Include common defines/options for all AMCC eval boards
 */
#include "amcc-common.h"

#define CONFIG_SYS_CLK_FREQ	66666667	/* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_early_init_f */
#define CONFIG_BOARD_EARLY_INIT_R	1	/* Call board_early_init_r */
#define CONFIG_MISC_INIT_R		1	/* Call misc_init_r */
#define CONFIG_BOARD_TYPES		1	/* support board types */
#define CONFIG_FIT
#define CFG_ALT_MEMTEST

#undef CONFIG_ZERO_BOOTDELAY_CHECK     /* ignore keypress on bootdelay==0 */
#define CONFIG_AUTOBOOT_KEYED          /* use key strings to stop autoboot */
#define CONFIG_AUTOBOOT_STOP_STR " "

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped PCI memory */
#define CONFIG_SYS_PCI_BASE		0xd0000000	/* internal PCI regs */
#define CONFIG_SYS_PCI_TARGBASE	CONFIG_SYS_PCI_MEMBASE

/* EBC stuff */
#ifdef CONFIG_DEVCONCENTER               /* Devcon-Center has 128 MB of flash */
#define CONFIG_SYS_FLASH_BASE		0xF8000000	/* later mapped here */
#define CONFIG_SYS_FLASH_SIZE		(128 << 20)
#else
#define CONFIG_SYS_FLASH_BASE		0xFC000000	/* later mapped here */
#define CONFIG_SYS_FLASH_SIZE		(64 << 20)
#endif

#define CONFIG_SYS_NVRAM_BASE		0xE0000000
#define CONFIG_SYS_UART_BASE		0xE0100000
#define CONFIG_SYS_IO_BASE		0xE0200000

#define CONFIG_SYS_BOOT_BASE_ADDR	0xFF000000	/* EBC Boot Space */
#define CONFIG_SYS_FLASH_BASE_PHYS_H	0x4
#ifdef CONFIG_DEVCONCENTER               /* Devcon-Center has 128 MB of flash */
#define CONFIG_SYS_FLASH_BASE_PHYS_L	0xC8000000
#else
#define CONFIG_SYS_FLASH_BASE_PHYS_L	0xCC000000
#endif
#define CONFIG_SYS_FLASH_BASE_PHYS \
	(((u64)CONFIG_SYS_FLASH_BASE_PHYS_H << 32) \
	| (u64)CONFIG_SYS_FLASH_BASE_PHYS_L)

#define CONFIG_SYS_OCM_BASE		0xE3000000	/* OCM: 64k */
#define CONFIG_SYS_SRAM_BASE		0xE8000000	/* SRAM: 256k */
#define CONFIG_SYS_SRAM_SIZE		(256 << 10)
#define CONFIG_SYS_LOCAL_CONF_REGS	0xEF000000

#define CONFIG_SYS_AHB_BASE		0xE2000000	/* int. AHB periph. */

/*
 * Initial RAM & stack pointer (placed in OCM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_BASE	/* OCM */
#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/

/*
 * Environment
 */
/*
 * Define here the location of the environment variables (FLASH).
 */
#define	CONFIG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */
#define CONFIG_SYS_NOR_CS		0	/* NOR chip connected to CSx */

/*
 * FLASH related
 */
#define CONFIG_SYS_FLASH_CFI		/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/
#define CONFIG_SYS_FLASH_CFI_AMD_RESET	1	/* Use AMD reset cmd */

#define CONFIG_SYS_FLASH_BANKS_LIST    {CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks */
#ifdef CONFIG_DEVCONCENTER
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* max num of sectors per chip*/
#else
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max num of sectors per chip*/
#endif

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase/ms */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write/ms */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* buff'd writes (20x faster) */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* 'E' for empty sector on flinfo */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000		/* size of one complete sector*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*
 * DDR SDRAM
 */

#define CONFIG_AUTOCALIB	"silent\0"	/* default is non-verbose    */

#define CONFIG_PPC4xx_DDR_AUTOCALIBRATION	/* IBM DDR autocalibration   */
#define DEBUG_PPC4xx_DDR_AUTOCALIBRATION	/* dynamic DDR autocal debug */
#undef CONFIG_PPC4xx_DDR_METHOD_A

/* DDR1/2 SDRAM Device Control Register Data Values */
/* Memory Queue */
#define CONFIG_SYS_SDRAM_R0BAS		0x0000f800
#define CONFIG_SYS_SDRAM_R1BAS		0x00000000
#define CONFIG_SYS_SDRAM_R2BAS		0x00000000
#define CONFIG_SYS_SDRAM_R3BAS		0x00000000
#define CONFIG_SYS_SDRAM_PLBADDULL	0x00000000
#define CONFIG_SYS_SDRAM_PLBADDUHB	0x00000008
#define CONFIG_SYS_SDRAM_CONF1LL	0x80001C00
#define CONFIG_SYS_SDRAM_CONF1HB	0x80001C80
#define CONFIG_SYS_SDRAM_CONFPATHB	0x10a68000

/* SDRAM Controller */
#define CONFIG_SYS_SDRAM0_MB0CF		0x00000201
#define CONFIG_SYS_SDRAM0_MB1CF		0x00000000
#define CONFIG_SYS_SDRAM0_MB2CF		0x00000000
#define CONFIG_SYS_SDRAM0_MB3CF		0x00000000
#define CONFIG_SYS_SDRAM0_MCOPT1	0x05120000
#define CONFIG_SYS_SDRAM0_MCOPT2	0x00000000
#define CONFIG_SYS_SDRAM0_MODT0		0x00000000
#define CONFIG_SYS_SDRAM0_MODT1		0x00000000
#define CONFIG_SYS_SDRAM0_MODT2		0x00000000
#define CONFIG_SYS_SDRAM0_MODT3		0x00000000
#define CONFIG_SYS_SDRAM0_CODT		0x00000020
#define CONFIG_SYS_SDRAM0_RTR		0x06180000
#define CONFIG_SYS_SDRAM0_INITPLR0	0xA8380000
#define CONFIG_SYS_SDRAM0_INITPLR1	0x81900400
#define CONFIG_SYS_SDRAM0_INITPLR2	0x81020000
#define CONFIG_SYS_SDRAM0_INITPLR3	0x81030000
#define CONFIG_SYS_SDRAM0_INITPLR4	0x81010002
#define CONFIG_SYS_SDRAM0_INITPLR5	0xE4000552
#define CONFIG_SYS_SDRAM0_INITPLR6	0x81900400
#define CONFIG_SYS_SDRAM0_INITPLR7	0x8A880000
#define CONFIG_SYS_SDRAM0_INITPLR8	0x8A880000
#define CONFIG_SYS_SDRAM0_INITPLR9	0x8A880000
#define CONFIG_SYS_SDRAM0_INITPLR10	0x8A880000
#define CONFIG_SYS_SDRAM0_INITPLR11	0x81000452
#define CONFIG_SYS_SDRAM0_INITPLR12	0x81010382
#define CONFIG_SYS_SDRAM0_INITPLR13	0x81010002
#define CONFIG_SYS_SDRAM0_INITPLR14	0x00000000
#define CONFIG_SYS_SDRAM0_INITPLR15	0x00000000
#define CONFIG_SYS_SDRAM0_RQDC		0x80000038
#define CONFIG_SYS_SDRAM0_RFDC		0x00000257
#define CONFIG_SYS_SDRAM0_RDCC		0x40000000
#define CONFIG_SYS_SDRAM0_DLCR		0x00000000
#define CONFIG_SYS_SDRAM0_CLKTR		0x40000000
#define CONFIG_SYS_SDRAM0_WRDTR		0x86000823
#define CONFIG_SYS_SDRAM0_SDTR1		0x80201000
#define CONFIG_SYS_SDRAM0_SDTR2		0x32204232
#define CONFIG_SYS_SDRAM0_SDTR3		0x090C0D15
#define CONFIG_SYS_SDRAM0_MMODE		0x00000452
#define CONFIG_SYS_SDRAM0_MEMODE	0x00000002

#define CONFIG_SYS_MBYTES_SDRAM	256	/* 256MB */

/*
 * I2C
 */
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000

#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR		(0xa8>>1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/* I2C bootstrap EEPROM */
#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR	0x54
#define CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET	0
#define CONFIG_4xx_CONFIG_BLOCKSIZE		16

/* I2C SYSMON */
#define CONFIG_DTT_LM63         1       /* National LM63        */
#define CONFIG_DTT_SENSORS      { 0 }   /* Sensor addresses     */
#define CONFIG_DTT_PWM_LOOKUPTABLE      \
	{ { 40, 10 }, { 50, 20 }, { 60, 40 } }
#define CONFIG_DTT_TACH_LIMIT   0xa10

/* RTC configuration */
#define CONFIG_RTC_DS1337	1
#define CONFIG_SYS_I2C_RTC_ADDR	0x68

/*
 * Ethernet
 */
#define CONFIG_IBM_EMAC4_V4	1

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1

#define CONFIG_PHY_ADDR		2	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR	3

#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_PHY_DYNAMIC_ANEG	1

/*
 * USB-OHCI
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#undef CONFIG_SYS_OHCI_BE_CONTROLLER	/* 460EX has little endian descriptors*/
#define CONFIG_SYS_OHCI_SWAP_REG_ACCESS	/* 460EX has little endian register */
#define CONFIG_SYS_OHCI_USE_NPS		/* force NoPowerSwitching mode */
#define CONFIG_SYS_USB_OHCI_REGS_BASE	(CONFIG_SYS_AHB_BASE | 0xd0000)
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"ppc440"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 15

/*
 * Default environment variables
 */
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

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_CHIP_CONFIG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/*
 * PCI stuff
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP			/* do pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CONFIG_PCI_CONFIG_HOST_BRIDGE
#define CONFIG_PCI_DISABLE_PCIE

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT	/* let board init pci target */
#undef	CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1014	/* IBM */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */


/*
 * External Bus Controller (EBC) Setup
 */

/*
 * CompactCenter has 64MBytes of NOR FLASH (Spansion 29GL512), but the
 * boot EBC mapping only supports a maximum of 16MBytes
 * (4.ff00.0000 - 4.ffff.ffff).
 * To solve this problem, the FLASH has to get remapped to another
 * EBC address which accepts bigger regions:
 *
 * 0xfc00.0000 -> 4.cc00.0000
 */


/* Memory Bank 0 (NOR-FLASH) initialization */
#define CONFIG_SYS_EBC_PB0AP		0x10055e00
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_BOOT_BASE_ADDR | 0x9a000)

/* Memory Bank 1 (NVRAM) initialization	*/
#define CONFIG_SYS_EBC_PB1AP		0x02815480
/* BAS=NVRAM,BS=1MB,BU=R/W,BW=8bit*/
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_NVRAM_BASE | 0x18000)

/* Memory Bank 2 (UART) initialization	*/
#define CONFIG_SYS_EBC_PB2AP		0x02815480
/* BAS=UART,BS=1MB,BU=R/W,BW=16bit*/
#define CONFIG_SYS_EBC_PB2CR		(CONFIG_SYS_UART_BASE | 0x1A000)

/* Memory Bank 3 (IO) initialization */
#define CONFIG_SYS_EBC_PB3AP		0x02815480
/* BAS=IO,BS=1MB,BU=R/W,BW=16bit*/
#define CONFIG_SYS_EBC_PB3CR		(CONFIG_SYS_IO_BASE | 0x1A000)

/*
 * PPC4xx GPIO Configuration
 */
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
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO50  USB_SERVICE_SUSPEND_N		*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO51  SPI_CSS_N			*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO52  FPGA_PROGRAM_UC_N		*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO53  FPGA_INIT_UC_N		*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO54  WD_STROBE			*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO55  LED_2_OUT			*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO56  LED_1_OUT			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO61  STARTUP_FINISHED_N		*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO62  STARTUP_FINISHED		*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO63  SERVICE_PORT_ACTIVE		*/	\
}											\
}

#endif	/* __CONFIG_H */
