/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/************************************************************************
 * TAISHAN.h - configuration for AMCC 440GX Ref
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_TAISHAN		1	/* Board is taishan		*/
#define CONFIG_440GX		1	/* Specifc GX support		*/
#define CONFIG_440		1	/* ... PPC440 family		*/
#define CONFIG_4xx		1	/* ... PPC4xx family		*/
#define CONFIG_SYS_CLK_FREQ	33333333 /* external freq to pll	*/

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		taishan
#define CONFIG_USE_TTY		ttyS1
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_pre_init		*/
#define CONFIG_MISC_INIT_R	1	/* Call misc_init_r		*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE		0xfc000000	/* start of FLASH	*/
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CONFIG_SYS_PERIPHERAL_BASE	0xe0000000	/* internal peripherals	*/
#define CONFIG_SYS_ISRAM_BASE		0xc0000000	/* internal SRAM	*/
#define CONFIG_SYS_PCI_BASE		0xd0000000	/* internal PCI regs	*/

#define CONFIG_SYS_EBC0_FLASH_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_EBC1_FPGA_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x01000000)
#define CONFIG_SYS_EBC2_LCM_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x02000000)
#define CONFIG_SYS_EBC3_CONN_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x08000000)

#define CONFIG_SYS_GPIO_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x00000700)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_TEMP_STACK_OCM	1
#define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_ISRAM_BASE  /* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_END	0x2000		/* End of used area in RAM*/
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* num bytes initial data*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_UART1_CONSOLE	1	/* use of UART1 as console	*/
#define CONFIG_SYS_EXT_SERIAL_CLOCK	(1843200 * 6)	/* Ext clk @ 11.059 MHz */

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/

#define CONFIG_SYS_FLASH_BANKS_LIST    {CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_BANKS     1		    /* number of banks	    */
#define CONFIG_SYS_MAX_FLASH_SECT	1024		    /* sectors per device   */

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_ENV_SECT_SIZE	0x40000 /* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*-----------------------------------------------------------------------
 * E2PROM bootstrap configure value
 *----------------------------------------------------------------------*/

/*
 * 800/133/66
 * IIC 0~15: 86 78 11 6a 61 A7 04 62 00 00 00 00 00 00 00 00
 */

/*
 * 800/160/80
 * IIC 0~15: 86 78 c1 a6 09 67 04 63 00 00 00 00 00 00 00 00
 */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#undef CONFIG_SPD_EEPROM		/* Don't use SPD EEPROM for setup	*/
#define CONFIG_SDRAM_BANK0	1	/* init onboard DDR SDRAM bank 0	*/
#define	CONFIG_SYS_SDRAM0_TR0		0xC10A401A
#undef CONFIG_SDRAM_ECC			/* enable ECC support			*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/

#undef CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

#define CONFIG_SYS_BOOTSTRAP_IIC_ADDR	0x50

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75		1		/* ON Semi's LM75	*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc180000\0"					\
	"kozio=bootm 0xffe00000\0"					\
	""

/*-----------------------------------------------------------------------
 * Networking
 *----------------------------------------------------------------------*/
#define CONFIG_EMAC_NR_START	2	/* start with EMAC 2 (skip 0&1)	*/
#define CONFIG_PHY_ADDR		0xff	     /* no phy on EMAC0		*/
#define CONFIG_PHY1_ADDR	0xff	     /* no phy on EMAC1		*/
#define CONFIG_PHY2_ADDR	0x1
#define CONFIG_PHY3_ADDR	0x3
#define CONFIG_ET1011C_PHY	1
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_PHY_RESET        1       /* reset phy upon startup         */
#define CONFIG_PHY_RESET_DELAY	1000

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_DTT
#define CONFIG_CMD_PCI

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
#define CONFIG_EEPRO100       1		/* include PCI EEPRO100		*/
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup	*/
#define CONFIG_SYS_PCI_TARGBASE    0x80000000	/* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT		/* let board init pci target    */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */

#endif	/* __CONFIG_H */
