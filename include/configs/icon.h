/*
 * (C) Copyright 2009-2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/*
 * icon.h - configuration for Mosaixtech ICON (440SPe)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ICON		1		/* Board is icon	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_440		1		/* ... PPC440 family	*/
#define CONFIG_440SPE		1		/* Specifc SPe support	*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll	*/
#define CONFIG_SYS_4xx_RESET_TYPE 0x2	/* use chip reset on this board	*/

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		icon
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F	/* Call board_pre_init	*/
#define CONFIG_BOARD_EARLY_INIT_R	/* Call board_early_init_r */

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_FLASH_BASE	0xfc000000	/* later mapped to this addr */
#define CONFIG_SYS_PERIPHERAL_BASE 0xa0000000	/* internal peripherals	*/
#define CONFIG_SYS_ISRAM_BASE	0x90000000	/* internal SRAM	*/

#define CONFIG_SYS_PCI_MEMBASE	0x80000000	/* mapped PCI memory	*/
#define CONFIG_SYS_PCI_BASE	0xd0000000	/* internal PCI regs	*/
#define CONFIG_SYS_PCI_TARGBASE	CONFIG_SYS_PCI_MEMBASE

#define CONFIG_SYS_PCIE_MEMBASE	0xb0000000	/* mapped PCIe memory	*/
#define CONFIG_SYS_PCIE_MEMSIZE	0x08000000	/* incr for PCIe port */
#define CONFIG_SYS_PCIE_BASE	0xe0000000	/* PCIe UTL regs */

#define CONFIG_SYS_PCIE0_CFGBASE	0xc0000000
#define CONFIG_SYS_PCIE1_CFGBASE	0xc1000000
#define CONFIG_SYS_PCIE2_CFGBASE	0xc2000000
#define CONFIG_SYS_PCIE0_XCFGBASE	0xc3000000
#define CONFIG_SYS_PCIE1_XCFGBASE	0xc3001000
#define CONFIG_SYS_PCIE2_XCFGBASE	0xc3002000

/* base address of inbound PCIe window */
#define CONFIG_SYS_PCIE_INBOUND_BASE	0x0000000000000000ULL

/* System RAM mapped to PCI space */
#define CONFIG_PCI_SYS_MEM_BUS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_PHYS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_SIZE	(1024 * 1024 * 1024)

#define CONFIG_SYS_ACE_BASE		0xfb000000	/* Xilinx ACE CF */
#define CONFIG_SYS_ACE_BASE_PHYS_H	0x4
#define CONFIG_SYS_ACE_BASE_PHYS_L	0xfe000000

#define CONFIG_SYS_FLASH_SIZE		(64 << 20)
#define CONFIG_SYS_BOOT_BASE_ADDR	0xFF000000	/* EBC Boot Space */
#define CONFIG_SYS_FLASH_BASE_PHYS_H	0x4
#define CONFIG_SYS_FLASH_BASE_PHYS_L	0xEC000000
#define CONFIG_SYS_FLASH_BASE_PHYS	(((u64)CONFIG_SYS_FLASH_BASE_PHYS_H << 32) | \
					 (u64)CONFIG_SYS_FLASH_BASE_PHYS_L)

/*
 * Initial RAM & stack pointer (placed in internal SRAM)
 */
#define CONFIG_SYS_TEMP_STACK_OCM	1
#define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_ISRAM_BASE	/* Init RAM */
#define CONFIG_SYS_INIT_RAM_END		0x2000		/* end used area */
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* sizeof init data */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
					 CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_POST_WORD_ADDR	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_POST_WORD_ADDR

/*
 * Serial Port
 */
#undef CONFIG_UART1_CONSOLE
#undef CONFIG_SYS_EXT_SERIAL_CLOCK

/*
 * DDR2 SDRAM
 */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS	{ 0x51 } /* SPD I2C SPD addresses	*/
#define CONFIG_DDR_ECC			/* with ECC support		*/
#define CONFIG_DDR_RQDC_FIXED	0x80000038 /* fixed value for RQDC	*/

/*
 * I2C
 */
#define CONFIG_SYS_I2C_SPEED	100000	/* I2C speed and slave address	*/

#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_SPD_BUS_NUM	0	/* The I2C bus for SPD		*/

#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/* I2C bootstrap EEPROM */
#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR	0x50
#define CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET	0
#define CONFIG_4xx_CONFIG_BLOCKSIZE		8

/* I2C RTC */
#define CONFIG_RTC_M41T11
#define CONFIG_SYS_RTC_BUS_NUM	1	/* The I2C bus for RTC		*/
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#define CONFIG_SYS_M41T11_BASE_YEAR 1900 /* play along with linux	*/

/*
 * Environment
 */
#define	CONFIG_ENV_IS_IN_FLASH	1	/* Environment uses flash	*/

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fc000000\0"					\
	"fdt_addr=fc1e0000\0"						\
	"ramdisk_addr=fc200000\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:RP:RP\0"						\
	""

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_CHIP_CONFIG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

#define	CONFIG_IBM_EMAC4_V4		/* 440SPe has this EMAC version	*/
#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/
#define CONFIG_HAS_ETH0
#define CONFIG_PHY_RESET		/* reset phy upon startup	*/
#define CONFIG_PHY_RESET_DELAY	1000
#define CONFIG_CIS8201_PHY		/* Enable RGMII mode for Cicada phy */
#define CONFIG_PHY_GIGE			/* Include GbE speed/duplex det. */

/*
 * FLASH related
 */
#define CONFIG_SYS_FLASH_CFI		/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/
#define CONFIG_SYS_FLASH_CFI_AMD_RESET	/* Use AMD (Spansion) reset cmd */
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL /* use status poll method	*/

#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* TO for Flash Erase (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* TO for Flash Write (ms) */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* use buffered writes	*/
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* 'E' for empty sector */

#define CONFIG_ENV_SECT_SIZE	0x20000	/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Env Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*
 * PCI stuff
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup	*/
#define CONFIG_PCI_CONFIG_HOST_BRIDGE
#define CONFIG_PCI_BOOTDELAY	1000	/* enable pci bootdelay variable*/

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT	/* let board init pci target    */
#undef	CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1014	/* IBM			*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever		*/

/*
 * Xilinx System ACE support
 */
#define CONFIG_SYSTEMACE		/* Enable SystemACE support	*/
#define CONFIG_SYS_SYSTEMACE_WIDTH	16	/* Data bus width is 16	*/
#define CONFIG_SYS_SYSTEMACE_BASE	CONFIG_SYS_ACE_BASE
#define CONFIG_DOS_PARTITION

/*
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash) initialization					*/
#define CONFIG_SYS_EBC_PB0AP	(EBC_BXAP_BME_DISABLED      |		\
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
#define CONFIG_SYS_EBC_PB0CR	(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_FLASH_BASE) | \
				 EBC_BXCR_BS_64MB                    |	\
				 EBC_BXCR_BU_RW                      |	\
				 EBC_BXCR_BW_16BIT)

/* Memory Bank 1 (Xilinx System ACE controller) initialization		*/
#define CONFIG_SYS_EBC_PB1AP	(EBC_BXAP_BME_DISABLED      |		\
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
#define CONFIG_SYS_EBC_PB1CR	(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_ACE_BASE_PHYS_L) | \
				 EBC_BXCR_BS_1MB                    |	\
				 EBC_BXCR_BU_RW                     |	\
				 EBC_BXCR_BW_16BIT)

/*
 * Initialize EBC CONFIG -
 * Keep the Default value, but the bit PDT which has to be set to 1 ?TBC
 * default value : 0x07C00000 - 0 0 000 1 1 1 1 1 0000 0 00000 000000000000
 */
#define CONFIG_SYS_EBC_CFG	(EBC_CFG_LE_UNLOCK    |	\
				 EBC_CFG_PTD_ENABLE   |	\
				 EBC_CFG_RTC_16PERCLK | \
				 EBC_CFG_ATC_PREVIOUS | \
				 EBC_CFG_DTC_PREVIOUS | \
				 EBC_CFG_CTC_PREVIOUS | \
				 EBC_CFG_OEO_PREVIOUS | \
				 EBC_CFG_EMC_DEFAULT  |	\
				 EBC_CFG_PME_DISABLE  |	\
				 EBC_CFG_PR_16)

/*
 * GPIO Setup
 */
#define CONFIG_SYS_GPIO_PCIE_PRESENT0	17
#define CONFIG_SYS_GPIO_PCIE_PRESENT1	21
#define CONFIG_SYS_GPIO_PCIE_PRESENT2	23
#define CONFIG_SYS_GPIO_RS232_FORCEOFF	30

#define CONFIG_SYS_PFC0		(GPIO_VAL(CONFIG_SYS_GPIO_PCIE_PRESENT0) | \
				 GPIO_VAL(CONFIG_SYS_GPIO_PCIE_PRESENT1) | \
				 GPIO_VAL(CONFIG_SYS_GPIO_PCIE_PRESENT2) | \
				 GPIO_VAL(CONFIG_SYS_GPIO_RS232_FORCEOFF))
#define CONFIG_SYS_GPIO_OR	GPIO_VAL(CONFIG_SYS_GPIO_RS232_FORCEOFF)
#define CONFIG_SYS_GPIO_TCR	GPIO_VAL(CONFIG_SYS_GPIO_RS232_FORCEOFF)
#define CONFIG_SYS_GPIO_ODR	0

#endif	/* __CONFIG_H */
