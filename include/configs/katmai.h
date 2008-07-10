/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
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
 * katmai.h - configuration for AMCC Katmai (440SPe)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_KATMAI			1	/* Board is Katmai	*/
#define CONFIG_4xx			1	/* ... PPC4xx family	*/
#define CONFIG_440			1	/* ... PPC440 family	*/
#define CONFIG_440SPE			1	/* Specifc SPe support	*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll	*/
#define CFG_4xx_RESET_TYPE	0x2	/* use chip reset on this board	*/

/*
 * Enable this board for more than 2GB of SDRAM
 */
#define CONFIG_PHYS_64BIT
#define	CONFIG_VERY_BIG_RAM
#define CONFIG_MAX_MEM_MAPPED	((phys_size_t)2 << 30)

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		katmai
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_pre_init		*/
#define	CONFIG_MISC_INIT_F	1	/* Use misc_init_f()		*/
#undef  CONFIG_SHOW_BOOT_PROGRESS

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_FLASH_BASE		0xff000000	/* start of FLASH	*/
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
#define CFG_PCIE_INBOUND_BASE	0x0000000000000000ULL

/* System RAM mapped to PCI space */
#define CONFIG_PCI_SYS_MEM_BUS	CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_PHYS	CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_SIZE	(1024 * 1024 * 1024)

#define CFG_ACE_BASE		0xfe000000	/* Xilinx ACE controller - Compact Flash */

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
#undef CFG_EXT_SERIAL_CLOCK

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS	{0x51, 0x52}	/* SPD i2c spd addresses*/
#define CONFIG_DDR_ECC		1	/* with ECC support		*/
#define CONFIG_DDR_RQDC_FIXED	0x80000038 /* optimal value found by GDA*/
#undef  CONFIG_STRESS

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address	*/

#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_SPD_BUS_NUM		0	/* The I2C bus for SPD		*/

#define IIC0_BOOTPROM_ADDR	0x50
#define IIC0_ALT_BOOTPROM_ADDR	0x54

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	(0x50)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

/* I2C RTC */
#define CONFIG_RTC_M41T11	1
#define CFG_RTC_BUS_NUM		1	/* The I2C bus for RTC		*/
#define CFG_I2C_RTC_ADDR	0x68
#define CFG_M41T11_BASE_YEAR	1900	/* play along with linux	*/

/* I2C DTT */
#define CONFIG_DTT_ADM1021	1	/* ADM1021 temp sensor support	*/
#define CFG_DTT_BUS_NUM		1	/* The I2C bus for DTT		*/
/*
 * standard dtt sensor configuration - bottom bit will determine local or
 * remote sensor of the ADM1021, the rest determines index into
 * CFG_DTT_ADM1021 array below.
 */
#define CONFIG_DTT_SENSORS	{ 0, 1 }

/*
 * ADM1021 temp sensor configuration (see dtt/adm1021.c for details).
 * there will be one entry in this array for each two (dummy) sensors in
 * CONFIG_DTT_SENSORS.
 *
 * For Katmai board:
 * - only one ADM1021
 * - i2c addr 0x18
 * - conversion rate 0x02 = 0.25 conversions/second
 * - ALERT ouput disabled
 * - local temp sensor enabled, min set to 0 deg, max set to 85 deg
 * - remote temp sensor enabled, min set to 0 deg, max set to 85 deg
 */
#define CFG_DTT_ADM1021		{ { 0x18, 0x02, 0, 1, 0, 85, 1, 0, 58} }

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define	CFG_ENV_IS_IN_FLASH	1	/* Environment uses flash	*/

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fff10000\0"					\
	"ramdisk_addr=fff20000\0"					\
	"kozio=bootm ffc60000\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:RP:RP\0"						\
	""

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

#define	CONFIG_IBM_EMAC4_V4	1	/* 440SPe has this EMAC version	*/
#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/
#define CONFIG_HAS_ETH0
#define CONFIG_PHY_RESET        1	/* reset phy upon startup	*/
#define CONFIG_PHY_RESET_DELAY	1000
#define CONFIG_CIS8201_PHY	1	/* Enable 'special' RGMII mode for Cicada phy */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/

#define CFG_FLASH_BANKS_LIST    {CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS     1		    /* number of banks	    */
#define CFG_MAX_FLASH_SECT	1024		    /* sectors per device   */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_ENV_SECT_SIZE	0x20000 /* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

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

/*-----------------------------------------------------------------------
 * Xilinx System ACE support
 *----------------------------------------------------------------------*/
#define CONFIG_SYSTEMACE	1	/* Enable SystemACE support	*/
#define CFG_SYSTEMACE_WIDTH	16	/* Data bus width is 16		*/
#define CFG_SYSTEMACE_BASE	CFG_ACE_BASE
#define CONFIG_DOS_PARTITION	1

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/* Memory Bank 0 (Flash) initialization					*/
#define CFG_EBC_PB0AP		(EBC_BXAP_BME_DISABLED      |		\
				 EBC_BXAP_TWT_ENCODE(7)     |		\
				 EBC_BXAP_BCE_DISABLE       |		\
				 EBC_BXAP_BCT_2TRANS        |		\
				 EBC_BXAP_CSN_ENCODE(0)     |		\
				 EBC_BXAP_OEN_ENCODE(0)     |		\
				 EBC_BXAP_WBN_ENCODE(0)     |		\
				 EBC_BXAP_WBF_ENCODE(0)     |		\
				 EBC_BXAP_TH_ENCODE(0)      |		\
				 EBC_BXAP_RE_DISABLED       |		\
				 EBC_BXAP_SOR_DELAYED       |		\
				 EBC_BXAP_BEM_WRITEONLY     |		\
				 EBC_BXAP_PEN_DISABLED)
#define CFG_EBC_PB0CR		(EBC_BXCR_BAS_ENCODE(CFG_FLASH_BASE) |	\
				 EBC_BXCR_BS_16MB                    |	\
				 EBC_BXCR_BU_RW                      |	\
				 EBC_BXCR_BW_16BIT)

/* Memory Bank 1 (Xilinx System ACE controller) initialization		*/
#define CFG_EBC_PB1AP		(EBC_BXAP_BME_DISABLED      |		\
				 EBC_BXAP_TWT_ENCODE(4)     |		\
				 EBC_BXAP_BCE_DISABLE       |		\
				 EBC_BXAP_BCT_2TRANS        |		\
				 EBC_BXAP_CSN_ENCODE(0)     |		\
				 EBC_BXAP_OEN_ENCODE(0)     |		\
				 EBC_BXAP_WBN_ENCODE(0)     |		\
				 EBC_BXAP_WBF_ENCODE(0)     |		\
				 EBC_BXAP_TH_ENCODE(0)      |		\
				 EBC_BXAP_RE_DISABLED       |		\
				 EBC_BXAP_SOR_NONDELAYED    |		\
				 EBC_BXAP_BEM_WRITEONLY     |		\
				 EBC_BXAP_PEN_DISABLED)
#define CFG_EBC_PB1CR		(EBC_BXCR_BAS_ENCODE(CFG_ACE_BASE)  |	\
				 EBC_BXCR_BS_1MB                    |	\
				 EBC_BXCR_BU_RW                     |	\
				 EBC_BXCR_BW_16BIT)

/*-------------------------------------------------------------------------
 * Initialize EBC CONFIG -
 * Keep the Default value, but the bit PDT which has to be set to 1 ?TBC
 * default value : 0x07C00000 - 0 0 000 1 1 1 1 1 0000 0 00000 000000000000
 *-------------------------------------------------------------------------*/
#define CFG_EBC_CFG		(EBC_CFG_LE_UNLOCK    |	\
				 EBC_CFG_PTD_ENABLE   |	\
				 EBC_CFG_RTC_16PERCLK | \
				 EBC_CFG_ATC_PREVIOUS | \
				 EBC_CFG_DTC_PREVIOUS | \
				 EBC_CFG_CTC_PREVIOUS | \
				 EBC_CFG_OEO_PREVIOUS | \
				 EBC_CFG_EMC_DEFAULT  |	\
				 EBC_CFG_PME_DISABLE  |	\
				 EBC_CFG_PR_16)

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CFG_GPIO_PCIE_PRESENT0	17
#define CFG_GPIO_PCIE_PRESENT1	21
#define CFG_GPIO_PCIE_PRESENT2	23
#define CFG_GPIO_RS232_FORCEOFF	30

#define CFG_PFC0		(GPIO_VAL(CFG_GPIO_PCIE_PRESENT0) | \
				 GPIO_VAL(CFG_GPIO_PCIE_PRESENT1) | \
				 GPIO_VAL(CFG_GPIO_PCIE_PRESENT2) | \
				 GPIO_VAL(CFG_GPIO_RS232_FORCEOFF))
#define CFG_GPIO_OR		GPIO_VAL(CFG_GPIO_RS232_FORCEOFF)
#define CFG_GPIO_TCR		GPIO_VAL(CFG_GPIO_RS232_FORCEOFF)
#define CFG_GPIO_ODR		0

#endif	/* __CONFIG_H */
