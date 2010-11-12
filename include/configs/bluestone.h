/*
 * bluestone.h - configuration for Bluestone (APM821XX)
 *
 * Copyright (c) 2010, Applied Micro Circuits Corporation
 * Author: Tirumala R Marri <tmarri@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_APM821XX		1	/* APM821XX series    */
#define CONFIG_HOSTNAME		bluestone

#define CONFIG_4xx		1	/* ... PPC4xx family */
#define CONFIG_440		1

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFFA0000
#endif

/*
 * Include common defines/options for all AMCC eval boards
 */
#include "amcc-common.h"
#define CONFIG_SYS_CLK_FREQ	50000000

#define CONFIG_BOARD_TYPES		1	/* support board types */
#define CONFIG_BOARD_EARLY_INIT_F	1       /* Call board_early_init_f */
#define CONFIG_MISC_INIT_R		1       /* Call misc_init_r */

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
/* EBC stuff */
/* later mapped to this addr */
#define CONFIG_SYS_FLASH_BASE		0xFFF00000
#define CONFIG_SYS_FLASH_SIZE		(4 << 20)	/* 1MB usable */

/* EBC Boot Space: 0xFF000000 */
#define CONFIG_SYS_BOOT_BASE_ADDR	0xFF000000
#define CONFIG_SYS_OCM_BASE		0xE3000000 /* OCM: 32k             */
#define CONFIG_SYS_SRAM_BASE		0xE8000000 /* SRAM: 256k           */
#define CONFIG_SYS_AHB_BASE		0xE2000000 /* internal AHB peripherals*/

#define CONFIG_SYS_SRAM_SIZE            (256 << 10)
/*
 * Initial RAM & stack pointer (placed in OCM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_BASE	/* OCM    */
#define CONFIG_SYS_INIT_RAM_SIZE		(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Environment
 */
/*
 * Define here the location of the environment variables (FLASH).
 */
#define CONFIG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */

/*
 * FLASH related
 */
#define CONFIG_SYS_FLASH_CFI	/* The flash is CFI compatible  */
#define CONFIG_FLASH_CFI_DRIVER	/* Use common CFI driver        */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE}
/* max number of memory banks           */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
/* max number of sectors on one chip    */
#define CONFIG_SYS_MAX_FLASH_SECT	80
/* Timeout for Flash Erase (in ms)      */
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000
/* Timeout for Flash Write (in ms)      */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500
/* use buffered writes (20x faster)     */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_EMPTY_INFO
#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000	/* size of one complete sector  */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector   */
/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/* SDRAM */
#define CONFIG_SPD_EEPROM	1       /* Use SPD EEPROM for setup     */
#define SPD_EEPROM_ADDRESS	{0x53, 0x51}	/* SPD i2c spd addresses */
#define CONFIG_PPC4xx_DDR_AUTOCALIBRATION       /* IBM DDR autocalibration */
#define CONFIG_AUTOCALIB	"silent\0"	/* default is non-verbose    */
#define CONFIG_DDR_ECC		1	/* with ECC support             */

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/

/*
 * I2C
 */
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed            */
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5	/* Data sheet */

/* I2C bootstrap EEPROM */
#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR	0x52
#define CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET	0
#define CONFIG_4xx_CONFIG_BLOCKSIZE		16

/*
 * Ethernet
 */
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_EMAC_PHY_MODE	EMAC_PHY_MODE_NONE_RGMII
#define CONFIG_HAS_ETH0
/* PHY address, See schematics  */
#define CONFIG_PHY_ADDR			0x1f
/* reset phy upon startup       */
#define CONFIG_PHY_RESET		1
/* Include GbE speed/duplex detection */
#define CONFIG_PHY_GIGE			1
#define CONFIG_PHY_DYNAMIC_ANEG		1

/*
 * External Bus Controller (EBC) Setup
 **/
#define CONFIG_SYS_EBC_CFG	(EBC_CFG_LE_LOCK    |	\
				 EBC_CFG_PTD_ENABLE   |	\
				 EBC_CFG_RTC_2048PERCLK | \
				 EBC_CFG_ATC_HI | \
				 EBC_CFG_DTC_HI | \
				 EBC_CFG_CTC_HI | \
				 EBC_CFG_OEO_PREVIOUS)
/* NOR Flash */
#define CONFIG_SYS_EBC_PB0AP 	(EBC_BXAP_BME_DISABLED   | \
				EBC_BXAP_TWT_ENCODE(64)  | \
				EBC_BXAP_BCE_DISABLE    | \
				EBC_BXAP_BCT_2TRANS     | \
				EBC_BXAP_CSN_ENCODE(1)  | \
				EBC_BXAP_OEN_ENCODE(2)  | \
				EBC_BXAP_WBN_ENCODE(2)  | \
				EBC_BXAP_WBF_ENCODE(2)  | \
				EBC_BXAP_TH_ENCODE(7)   | \
				EBC_BXAP_SOR_DELAYED    | \
				EBC_BXAP_BEM_WRITEONLY  | \
				EBC_BXAP_PEN_DISABLED)
/* Peripheral Bank Configuration Register - EBC_BxCR */
#define CONFIG_SYS_EBC_PB0CR	\
			(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_FLASH_BASE) | \
			EBC_BXCR_BS_1MB                | \
			EBC_BXCR_BU_RW                  | \
			EBC_BXCR_BW_8BIT)


#endif /* __CONFIG_H */
