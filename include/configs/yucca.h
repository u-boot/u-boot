/*
 * (C) Copyright 2004 Paul Reynolds <PaulReynolds@lhsolutions.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************
 * 1 january 2005	Alain Saurel <asaurel@amcc.com>
 * Adapted to current Das U-Boot source
 ***********************************************************************/
/************************************************************************
 * yucca.h - configuration for AMCC 440SPe Ref (yucca)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_4xx			1	/* ... PPC4xx family	*/
#define CONFIG_440			1	/* ... PPC440 family	*/
#define CONFIG_440SPE			1	/* Specifc SPe support	*/
#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init	*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll	*/
#define EXTCLK_33_33		33333333
#define EXTCLK_66_66		66666666
#define EXTCLK_50		50000000
#define EXTCLK_83		83333333

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		yucca
#include "amcc-common.h"

#define	CONFIG_MISC_INIT_F	1	/* Use misc_init_f()		*/
#undef  CONFIG_SHOW_BOOT_PROGRESS
#undef  CONFIG_STRESS

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_FLASH_BASE		0xfff00000	/* start of FLASH	*/
#define CFG_PERIPHERAL_BASE	0xa0000000	/* internal peripherals	*/
#define CFG_ISRAM_BASE		0x90000000	/* internal SRAM	*/

#define CFG_PCI_MEMBASE		0x80000000	/* mapped PCI memory	*/
#define CFG_PCI_BASE		0xd0000000	/* internal PCI regs	*/
#define CFG_PCI_TARGBASE	CFG_PCI_MEMBASE

#define CFG_PCIE_MEMBASE	0xb0000000	/* mapped PCIe memory	*/
#define CFG_PCIE_MEMSIZE	0x08000000	/* smallest incr for PCIe port */
#define CFG_PCIE_BASE		0xe0000000	/* PCIe UTL regs */

#define CFG_PCIE0_CFGBASE	0xc0000000
#define CFG_PCIE1_CFGBASE	0xc1000000
#define CFG_PCIE2_CFGBASE	0xc2000000
#define CFG_PCIE0_XCFGBASE	0xc3000000
#define CFG_PCIE1_XCFGBASE	0xc3001000
#define CFG_PCIE2_XCFGBASE	0xc3002000

/* base address of inbound PCIe window */
#define CFG_PCIE_INBOUND_BASE	0x0000000400000000ULL

/* System RAM mapped to PCI space */
#define CONFIG_PCI_SYS_MEM_BUS	CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_PHYS	CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_SIZE	(1024 * 1024 * 1024)

#define CFG_FPGA_BASE		0xe2000000	/* epld			*/
#define CFG_OPER_FLASH		0xe7000000	/* SRAM - OPER Flash	*/

/* #define CFG_NVRAM_BASE_ADDR 0x08000000 */
/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CFG_TEMP_STACK_OCM	1
#define CFG_OCM_DATA_ADDR	CFG_ISRAM_BASE
#define CFG_INIT_RAM_ADDR	CFG_ISRAM_BASE	/* Initial RAM address	*/
#define CFG_INIT_RAM_END	0x2000		/* End of used area in RAM */
#define CFG_GBL_DATA_SIZE	128		/* num bytes initial data */

#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 0x4)
#define CFG_INIT_SP_OFFSET	CFG_POST_WORD_ADDR

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef CONFIG_UART1_CONSOLE

#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#undef CFG_EXT_SERIAL_CLOCK
/* #define CFG_EXT_SERIAL_CLOCK	(1843200 * 6) */ /* Ext clk @ 11.059 MHz */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS	{0x53, 0x52}	/* SPD i2c spd addresses*/
#define CONFIG_DDR_ECC		1	/* with ECC support		*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/

#define IIC0_BOOTPROM_ADDR	0x50
#define IIC0_ALT_BOOTPROM_ADDR	0x54

/* Don't probe these addrs */
#define CFG_I2C_NOPROBES	{0x50, 0x52, 0x53, 0x54}

/* #if defined(CONFIG_CMD_EEPROM) */
/* #define CFG_I2C_EEPROM_ADDR	0x50 */	/* I2C boot EEPROM		*/
#define CFG_I2C_EEPROM_ADDR_LEN	2	/* Bytes of address		*/
/* #endif */

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/* #define CFG_NVRAM_SIZE	(0x2000 - 8) */	/* NVRAM size(8k)- RTC regs */

#undef  CFG_ENV_IS_IN_NVRAM		/* ... not in NVRAM		*/
#define	CFG_ENV_IS_IN_FLASH	1	/* Environment uses flash	*/
#undef	CFG_ENV_IS_IN_EEPROM		/* ... not in EEPROM		*/
#define CONFIG_ENV_OVERWRITE	1

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_PPC						\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=E7F10000\0"					\
	"ramdisk_addr=E7F20000\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:EP:EP\0"						\
	""

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM

#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/
#define CONFIG_HAS_ETH0
#define CONFIG_PHY_RESET        1	/* reset phy upon startup	*/
#define CONFIG_PHY_RESET_DELAY	1000
#define CONFIG_CIS8201_PHY	1	/* Enable 'special' RGMII mode for Cicada phy */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_MAX_FLASH_BANKS	3	/* number of banks		*/
#define CFG_MAX_FLASH_SECT	256	/* sectors per device		*/

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

#define CFG_FLASH_ADDR0		0x5555
#define CFG_FLASH_ADDR1		0x2aaa
#define CFG_FLASH_WORD_SIZE	unsigned char

#define CFG_FLASH_2ND_16BIT_DEV	1	/* evb440SPe has 8 and 16bit device */
#define CFG_FLASH_2ND_ADDR	0xe7c00000 /* evb440SPe has 8 and 16bit device*/

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000	/* size of one complete sector	*/
#define CFG_ENV_ADDR		0xfffa0000
/* #define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE) */
#define CFG_ENV_SIZE		0x10000	/* Size of Environment vars	*/
#endif /* CFG_ENV_IS_IN_FLASH */
/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_PNP		1	/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW	1	/* show pci devices on startup	*/
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT		/* let board init pci target    */
#undef	CFG_PCI_MASTER_INIT

#define CFG_PCI_SUBSYS_VENDORID 0x1014	/* IBM				*/
#define CFG_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever			*/
/* #define CFG_PCI_SUBSYS_ID	CFG_PCI_SUBSYS_DEVICEID */

/*
 *  NETWORK Support (PCI):
 */
/* Support for Intel 82557/82559/82559ER chips. */
#define CONFIG_EEPRO100

/* FB Divisor selection */
#define FPGA_FB_DIV_6		6
#define FPGA_FB_DIV_10		10
#define FPGA_FB_DIV_12		12
#define FPGA_FB_DIV_20		20

/* VCO Divisor selection */
#define	FPGA_VCO_DIV_4		4
#define	FPGA_VCO_DIV_6		6
#define	FPGA_VCO_DIV_8		8
#define	FPGA_VCO_DIV_10		10

/*----------------------------------------------------------------------------+
| FPGA registers and bit definitions
+----------------------------------------------------------------------------*/
/* PowerPC 440SPe Board FPGA is reached with physical address 0x1 E2000000. */
/* TLB initialization makes it correspond to logical address 0xE2000000. */
/* => Done init_chip.s in bootlib */
#define FPGA_REG_BASE_ADDR	0xE2000000
#define FPGA_GPIO_BASE_ADDR	0xE2010000
#define FPGA_INT_BASE_ADDR	0xE2020000

/*----------------------------------------------------------------------------+
| Display
+----------------------------------------------------------------------------*/
#define PPC440SPE_DISPLAY	FPGA_REG_BASE_ADDR

#define PPC440SPE_DISPLAY_D8	(FPGA_REG_BASE_ADDR+0x06)
#define PPC440SPE_DISPLAY_D4	(FPGA_REG_BASE_ADDR+0x04)
#define PPC440SPE_DISPLAY_D2	(FPGA_REG_BASE_ADDR+0x02)
#define PPC440SPE_DISPLAY_D1	(FPGA_REG_BASE_ADDR+0x00)
/*define   WRITE_DISPLAY_DIGIT(n) IOREG8(FPGA_REG_BASE_ADDR + (2*n))*/
/*#define   IOREG8(addr) *((volatile unsigned char *)(addr))*/

/*----------------------------------------------------------------------------+
| ethernet/reset/boot Register 1
+----------------------------------------------------------------------------*/
#define FPGA_REG10	(FPGA_REG_BASE_ADDR+0x10)

#define FPGA_REG10_10MHZ_ENABLE		0x8000
#define FPGA_REG10_100MHZ_ENABLE	0x4000
#define FPGA_REG10_GIGABIT_ENABLE	0x2000
#define FPGA_REG10_FULL_DUPLEX		0x1000	/* force Full Duplex*/
#define FPGA_REG10_RESET_ETH		0x0800
#define FPGA_REG10_AUTO_NEG_DIS		0x0400
#define FPGA_REG10_INTP_ETH		0x0200

#define FPGA_REG10_RESET_HISR		0x0080
#define FPGA_REG10_ENABLE_DISPLAY	0x0040
#define FPGA_REG10_RESET_SDRAM		0x0020
#define FPGA_REG10_OPER_BOOT		0x0010
#define FPGA_REG10_SRAM_BOOT		0x0008
#define FPGA_REG10_SMALL_BOOT		0x0004
#define FPGA_REG10_FORCE_COLA		0x0002
#define FPGA_REG10_COLA_MANUAL		0x0001

#define FPGA_REG10_SDRAM_ENABLE		0x0020

#define FPGA_REG10_ENET_ENCODE2(n) ((((unsigned long)(n))&0x0F)<<4) /*from ocotea ?*/
#define FPGA_REG10_ENET_DECODE2(n) ((((unsigned long)(n))>>4)&0x0F) /*from ocotea ?*/

/*----------------------------------------------------------------------------+
| MUX control
+----------------------------------------------------------------------------*/
#define FPGA_REG12	(FPGA_REG_BASE_ADDR+0x12)

#define FPGA_REG12_EBC_CTL		0x8000
#define FPGA_REG12_UART1_CTS_RTS	0x4000
#define FPGA_REG12_UART0_RX_ENABLE	0x2000
#define FPGA_REG12_UART1_RX_ENABLE	0x1000
#define FPGA_REG12_UART2_RX_ENABLE	0x0800
#define FPGA_REG12_EBC_OUT_ENABLE	0x0400
#define FPGA_REG12_GPIO0_OUT_ENABLE	0x0200
#define FPGA_REG12_GPIO1_OUT_ENABLE	0x0100
#define FPGA_REG12_GPIO_SELECT		0x0010
#define FPGA_REG12_GPIO_CHREG		0x0008
#define FPGA_REG12_GPIO_CLK_CHREG	0x0004
#define FPGA_REG12_GPIO_OETRI		0x0002
#define FPGA_REG12_EBC_ERROR		0x0001

/*----------------------------------------------------------------------------+
| PCI Clock control
+----------------------------------------------------------------------------*/
#define FPGA_REG16	(FPGA_REG_BASE_ADDR+0x16)

#define FPGA_REG16_PCI_CLK_CTL0		0x8000
#define FPGA_REG16_PCI_CLK_CTL1		0x4000
#define FPGA_REG16_PCI_CLK_CTL2		0x2000
#define FPGA_REG16_PCI_CLK_CTL3		0x1000
#define FPGA_REG16_PCI_CLK_CTL4		0x0800
#define FPGA_REG16_PCI_CLK_CTL5		0x0400
#define FPGA_REG16_PCI_CLK_CTL6		0x0200
#define FPGA_REG16_PCI_CLK_CTL7		0x0100
#define FPGA_REG16_PCI_CLK_CTL8		0x0080
#define FPGA_REG16_PCI_CLK_CTL9		0x0040
#define FPGA_REG16_PCI_EXT_ARB0		0x0020
#define FPGA_REG16_PCI_MODE_1		0x0010
#define FPGA_REG16_PCI_TARGET_MODE	0x0008
#define FPGA_REG16_PCI_INTP_MODE	0x0004

/* FB1 Divisor selection */
#define FPGA_REG16_FB2_DIV_MASK		0x1000
#define FPGA_REG16_FB2_DIV_LOW		0x0000
#define FPGA_REG16_FB2_DIV_HIGH		0x1000
/* FB2 Divisor selection */
/* S3 switch on Board */
#define FPGA_REG16_FB1_DIV_MASK		0x2000
#define FPGA_REG16_FB1_DIV_LOW		0x0000
#define FPGA_REG16_FB1_DIV_HIGH		0x2000
/* PCI0 Clock Selection */
/* S3 switch on Board */
#define FPGA_REG16_PCI0_CLK_MASK	0x0c00
#define FPGA_REG16_PCI0_CLK_33_33	0x0000
#define FPGA_REG16_PCI0_CLK_66_66	0x0800
#define FPGA_REG16_PCI0_CLK_100		0x0400
#define FPGA_REG16_PCI0_CLK_133_33	0x0c00
/* VCO Divisor selection */
/* S3 switch on Board */
#define FPGA_REG16_VCO_DIV_MASK		0xc000
#define FPGA_REG16_VCO_DIV_4		0x0000
#define FPGA_REG16_VCO_DIV_8		0x4000
#define FPGA_REG16_VCO_DIV_6		0x8000
#define FPGA_REG16_VCO_DIV_10		0xc000
/* Master Clock Selection */
/* S3, S4 switches on Board */
#define FPGA_REG16_MASTER_CLK_MASK	0x01c0
#define FPGA_REG16_MASTER_CLK_EXT	0x0000
#define FPGA_REG16_MASTER_CLK_66_66	0x0040
#define FPGA_REG16_MASTER_CLK_50	0x0080
#define FPGA_REG16_MASTER_CLK_33_33	0x00c0
#define FPGA_REG16_MASTER_CLK_25	0x0100

/*----------------------------------------------------------------------------+
| PCI Miscellaneous
+----------------------------------------------------------------------------*/
#define FPGA_REG18	(FPGA_REG_BASE_ADDR+0x18)

#define FPGA_REG18_PCI_PRSNT1		0x8000
#define FPGA_REG18_PCI_PRSNT2		0x4000
#define FPGA_REG18_PCI_INTA		0x2000
#define FPGA_REG18_PCI_SLOT0_INTP	0x1000
#define FPGA_REG18_PCI_SLOT1_INTP	0x0800
#define FPGA_REG18_PCI_SLOT2_INTP	0x0400
#define FPGA_REG18_PCI_SLOT3_INTP	0x0200
#define FPGA_REG18_PCI_PCI0_VC		0x0100
#define FPGA_REG18_PCI_PCI0_VTH1	0x0080
#define FPGA_REG18_PCI_PCI0_VTH2	0x0040
#define FPGA_REG18_PCI_PCI0_VTH3	0x0020

/*----------------------------------------------------------------------------+
| PCIe Miscellaneous
+----------------------------------------------------------------------------*/
#define FPGA_REG1A	(FPGA_REG_BASE_ADDR+0x1A)

#define FPGA_REG1A_PE0_GLED		0x8000
#define FPGA_REG1A_PE1_GLED		0x4000
#define FPGA_REG1A_PE2_GLED		0x2000
#define FPGA_REG1A_PE0_YLED		0x1000
#define FPGA_REG1A_PE1_YLED		0x0800
#define FPGA_REG1A_PE2_YLED		0x0400
#define FPGA_REG1A_PE0_PWRON		0x0200
#define FPGA_REG1A_PE1_PWRON		0x0100
#define FPGA_REG1A_PE2_PWRON		0x0080
#define FPGA_REG1A_PE0_REFCLK_ENABLE	0x0040
#define FPGA_REG1A_PE1_REFCLK_ENABLE	0x0020
#define FPGA_REG1A_PE2_REFCLK_ENABLE	0x0010
#define FPGA_REG1A_PE_SPREAD0		0x0008
#define FPGA_REG1A_PE_SPREAD1		0x0004
#define FPGA_REG1A_PE_SELSOURCE_0	0x0002
#define FPGA_REG1A_PE_SELSOURCE_1	0x0001

/*----------------------------------------------------------------------------+
| PCIe Miscellaneous
+----------------------------------------------------------------------------*/
#define FPGA_REG1C	(FPGA_REG_BASE_ADDR+0x1C)

#define FPGA_REG1C_PE0_ROOTPOINT	0x8000
#define FPGA_REG1C_PE1_ENDPOINT		0x4000
#define FPGA_REG1C_PE2_ENDPOINT		0x2000
#define FPGA_REG1C_PE0_PRSNT		0x1000
#define FPGA_REG1C_PE1_PRSNT		0x0800
#define FPGA_REG1C_PE2_PRSNT		0x0400
#define FPGA_REG1C_PE0_WAKE		0x0080
#define FPGA_REG1C_PE1_WAKE		0x0040
#define FPGA_REG1C_PE2_WAKE		0x0020
#define FPGA_REG1C_PE0_PERST		0x0010
#define FPGA_REG1C_PE1_PERST		0x0008
#define FPGA_REG1C_PE2_PERST		0x0004

/*----------------------------------------------------------------------------+
| Defines
+----------------------------------------------------------------------------*/
#define PERIOD_133_33MHZ	7500	/* 7,5ns */
#define PERIOD_100_00MHZ	10000	/* 10ns */
#define PERIOD_83_33MHZ		12000	/* 12ns */
#define PERIOD_75_00MHZ		13333	/* 13,333ns */
#define PERIOD_66_66MHZ		15000	/* 15ns */
#define PERIOD_50_00MHZ		20000	/* 20ns */
#define PERIOD_33_33MHZ		30000	/* 30ns */
#define PERIOD_25_00MHZ		40000	/* 40ns */

#endif	/* __CONFIG_H */
