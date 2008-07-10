/*
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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
 * board/config_EBONY.h - configuration for AMCC 440GP Ref (Ebony)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_EBONY		1	    /* Board is ebony		*/
#define CONFIG_440GP		1	    /* Specifc GP support	*/
#define CONFIG_440		1	    /* ... PPC440 family	*/
#define CONFIG_4xx		1	    /* ... PPC4xx family	*/
#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_early_init_f	*/
#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		ebony
#include "amcc-common.h"

/*
 * Define here the location of the environment variables (FLASH or NVRAM).
 * Note: DENX encourages to use redundant environment in FLASH. NVRAM is only
 *       supported for backward compatibility.
 */
#if 1
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CFG_ENV_IS_IN_NVRAM	1	/* use NVRAM for environment vars	*/
#endif

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE	    0x00000000	    /* _must_ be 0		*/
#define CFG_FLASH_BASE	    0xff800000	    /* start of FLASH		*/
#define CFG_PCI_MEMBASE	    0x80000000	    /* mapped pci memory	*/
#define CFG_PERIPHERAL_BASE 0xe0000000	    /* internal peripherals	*/
#define CFG_ISRAM_BASE	    0xc0000000	    /* internal SRAM		*/
#define CFG_PCI_BASE	    0xd0000000	    /* internal PCI regs	*/

#define CFG_NVRAM_BASE_ADDR (CFG_PERIPHERAL_BASE + 0x08000000)
#define CFG_FPGA_BASE	    (CFG_PERIPHERAL_BASE + 0x08300000)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_ADDR   CFG_ISRAM_BASE  /* Initial RAM address	*/
#define CFG_INIT_RAM_END    0x2000	    /* End of used area in RAM	*/
#define CFG_GBL_DATA_SIZE  128		    /* num bytes initial data	*/

#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_EXT_SERIAL_CLOCK	(1843200 * 6)	/* Ext clk @ 11.059 MHz */

/*-----------------------------------------------------------------------
 * NVRAM/RTC
 *
 * NOTE: Upper 8 bytes of NVRAM is where the RTC registers are located.
 * The DS1743 code assumes this condition (i.e. -- it assumes the base
 * address for the RTC registers is:
 *
 *	CFG_NVRAM_BASE_ADDR + CFG_NVRAM_SIZE
 *
 *----------------------------------------------------------------------*/
#define CFG_NVRAM_SIZE	    (0x2000 - 8)    /* NVRAM size(8k)- RTC regs */
#define CONFIG_RTC_DS174x	1		    /* DS1743 RTC		*/

#ifdef CFG_ENV_IS_IN_NVRAM
#define CFG_ENV_SIZE		0x1000	    /* Size of Environment vars */
#define CFG_ENV_ADDR		\
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_NVRAM */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_MAX_FLASH_BANKS	3		    /* number of banks	    */
#define CFG_MAX_FLASH_SECT	32		    /* sectors per device   */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define CFG_FLASH_ADDR0         0x5555
#define CFG_FLASH_ADDR1         0x2aaa
#define CFG_FLASH_WORD_SIZE     unsigned char

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000	/* size of one complete sector		*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS {0x53,0x52}	/* SPD i2c spd addresses	*/
#define CONFIG_PROG_SDRAM_TLB	1	/* setup SDRAM TLB's dynamically*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	(0xa8>>1)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=ff800000\0"					\
	"ramdisk_addr=ff810000\0"					\
	""

#define CONFIG_PHY_ADDR		8	/* PHY address			*/
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	9	/* EMAC1 PHY address		*/

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
#define CONFIG_PCI			            /* include pci support	        */
#define CONFIG_PCI_PNP			        /* do pci plug-and-play         */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CFG_PCI_TARGBASE    0x80000000  /* PCIaddr mapped to CFG_PCI_MEMBASE */

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT	            /* let board init pci target    */

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CFG_PCI_SUBSYS_DEVICEID 0xcafe  /* Whatever */

#endif	/* __CONFIG_H */
