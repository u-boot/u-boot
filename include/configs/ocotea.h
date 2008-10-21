/*
 * (C) Copyright 2004 Paul Reynolds <PaulReynolds@lhsolutions.com>
 *
 * (C) Copyright 2005
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************
 * 1 March 2004	 Travis B. Sawyer <tsawyer@sandburst.com>
 * Adapted to current Das U-Boot source
 ***********************************************************************/


/************************************************************************
 * OCOTEA.h - configuration for AMCC 440GX Ref (Ocotea)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_OCOTEA		1	    /* Board is ebony		*/
#define CONFIG_440GX		1	    /* Specifc GX support	*/
#define CONFIG_440		1	    /* ... PPC440 family	*/
#define CONFIG_4xx		1	    /* ... PPC4xx family	*/
#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_pre_init	*/
#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		ocotea
#include "amcc-common.h"

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE	    0xff800000	    /* start of FLASH		*/
#define CONFIG_SYS_PCI_MEMBASE	    0x80000000	    /* mapped pci memory	*/
#define CONFIG_SYS_PERIPHERAL_BASE 0xe0000000	    /* internal peripherals	*/
#define CONFIG_SYS_ISRAM_BASE	    0xc0000000	    /* internal SRAM		*/
#define CONFIG_SYS_PCI_BASE	    0xd0000000	    /* internal PCI regs	*/

#define CONFIG_SYS_FPGA_BASE	    (CONFIG_SYS_PERIPHERAL_BASE + 0x08300000)
#define CONFIG_SYS_NVRAM_BASE_ADDR (CONFIG_SYS_PERIPHERAL_BASE + 0x08000000)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_TEMP_STACK_OCM  1
#define CONFIG_SYS_OCM_DATA_ADDR   CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_ADDR   CONFIG_SYS_ISRAM_BASE  /* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_END    0x2000	    /* End of used area in RAM	*/
#define CONFIG_SYS_GBL_DATA_SIZE   128		    /* num bytes initial data	*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_POST_WORD_ADDR	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_POST_WORD_ADDR

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_EXT_SERIAL_CLOCK	(1843200 * 6)	/* Ext clk @ 11.059 MHz */

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/*
 * Define here the location of the environment variables (FLASH or NVRAM).
 * Note: DENX encourages to use redundant environment in FLASH. NVRAM is only
 *       supported for backward compatibility.
 */
#if 1
#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CONFIG_ENV_IS_IN_NVRAM	1	/* use NVRAM for environment vars	*/
#endif


/*-----------------------------------------------------------------------
 * NVRAM/RTC
 *
 * NOTE: Upper 8 bytes of NVRAM is where the RTC registers are located.
 * The DS1743 code assumes this condition (i.e. -- it assumes the base
 * address for the RTC registers is:
 *
 *	CONFIG_SYS_NVRAM_BASE_ADDR + CONFIG_SYS_NVRAM_SIZE
 *
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_NVRAM_SIZE	    (0x2000 - 8)    /* NVRAM size(8k)- RTC regs */
#define CONFIG_RTC_DS174x	1		    /* DS1743 RTC		*/

#ifdef CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_ENV_SIZE		0x1000	    /* Size of Environment vars */
#define CONFIG_ENV_ADDR		\
	(CONFIG_SYS_NVRAM_BASE_ADDR+CONFIG_SYS_NVRAM_SIZE-CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_NVRAM */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_FLASH_BANKS	3		    /* number of banks	    */
#define CONFIG_SYS_MAX_FLASH_SECT	64		    /* sectors per device   */

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_ADDR0         0x5555
#define CONFIG_SYS_FLASH_ADDR1         0x2aaa
#define CONFIG_SYS_FLASH_WORD_SIZE     unsigned char

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000		/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS {0x53,0x52}	/* SPD i2c spd addresses	*/
#define CONFIG_PROG_SDRAM_TLB	1	/* setup SDRAM TLB's dynamically*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/

#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR	(0xa8>>1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_PPC						\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fff00000\0"					\
	"ramdisk_addr=fff10000\0"					\
	""

#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR	2
#define CONFIG_PHY2_ADDR	0x10
#define CONFIG_PHY3_ADDR	0x18
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#define CONFIG_CIS8201_PHY	1	/* Enable 'special' RGMII mode for Cicada phy */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_PHY_RESET        1       /* reset phy upon startup         */
#define CONFIG_PHY_RESET_DELAY	1000

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup	*/
#define CONFIG_SYS_PCI_TARGBASE    0x80000000	/* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT		/* let board init pci target    */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */

#endif	/* __CONFIG_H */
