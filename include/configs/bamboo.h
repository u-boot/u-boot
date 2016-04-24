/*
 * (C) Copyright 2005-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/************************************************************************
 * bamboo.h - configuration for BAMBOO board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_BAMBOO		1	/* Board is BAMBOO              */
#define CONFIG_440EP		1	/* Specific PPC440EP support    */
#define CONFIG_440		1	/* ... PPC440 family	        */
#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFFA0000
#endif

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		bamboo
#include "amcc-common.h"

/* Reclaim some space. */
#undef CONFIG_SYS_LONGHELP

#define CONFIG_BOARD_EARLY_INIT_F 1     /* Call board_early_init_f	*/

/*
 * Please note that, if NAND support is enabled, the 2nd ethernet port
 * can't be used because of pin multiplexing. So, if you want to use the
 * 2nd ethernet port you have to "undef" the following define.
 */
#define CONFIG_BAMBOO_NAND      1       /* enable nand flash support    */

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE	        0xfff00000	    /* start of FLASH	*/
#define CONFIG_SYS_PCI_MEMBASE	        0xa0000000	    /* mapped pci memory*/
#define CONFIG_SYS_PCI_MEMBASE1        CONFIG_SYS_PCI_MEMBASE  + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE2        CONFIG_SYS_PCI_MEMBASE1 + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE3        CONFIG_SYS_PCI_MEMBASE2 + 0x10000000

/*Don't change either of these*/
#define CONFIG_SYS_PCI_BASE		0xe0000000	    /* internal PCI regs*/
/*Don't change either of these*/

#define CONFIG_SYS_USB_DEVICE          0x50000000
#define CONFIG_SYS_NVRAM_BASE_ADDR     0x80000000
#define CONFIG_SYS_BOOT_BASE_ADDR      0xf0000000
#define CONFIG_SYS_NAND_ADDR           0x90000000
#define CONFIG_SYS_NAND2_ADDR          0x94000000

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in SDRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_INIT_RAM_DCACHE	1		/* d-cache as init ram	*/
#define CONFIG_SYS_INIT_RAM_ADDR	0x70000000	/* DCache       */
#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_EXT_SERIAL_CLOCK	11059200 /* use external 11.059MHz clk	*/

/*-----------------------------------------------------------------------
 * NVRAM/RTC
 *
 * NOTE: The RTC registers are located at 0x7FFF0 - 0x7FFFF
 * The DS1558 code assumes this condition
 *
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_NVRAM_SIZE	        (0x2000 - 0x10) /* NVRAM size(8k)- RTC regs     */
#define CONFIG_RTC_DS1556	1		         /* DS1556 RTC		*/

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_FLASH_BANKS	3	/* number of banks			*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* sectors per device			*/

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_ADDR0         0x555
#define CONFIG_SYS_FLASH_ADDR1         0x2aa
#define CONFIG_SYS_FLASH_WORD_SIZE     unsigned char

#define CONFIG_SYS_FLASH_2ND_16BIT_DEV 1	/* bamboo has 8 and 16bit device	*/
#define CONFIG_SYS_FLASH_2ND_ADDR      0x87800000  /* bamboo has 8 and 16bit device	*/

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000	/* size of one complete sector		*/
#define CONFIG_ENV_ADDR		((-CONFIG_SYS_MONITOR_LEN)-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * NAND FLASH
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_NAND_DEVICE	2
#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_NAND_ADDR + CONFIG_SYS_NAND_CS)
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE, CONFIG_SYS_NAND_ADDR + 2 }
#define CONFIG_SYS_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/
#define CONFIG_SYS_NAND_CS		1

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------------- */
#define CONFIG_SPD_EEPROM               /* Use SPD EEPROM for setup             */
#undef CONFIG_DDR_ECC			/* don't use ECC			*/
#define CONFIG_SYS_SIMULATE_SPD_EEPROM	0xff	/* simulate spd eeprom on this address	*/
#define SPD_EEPROM_ADDRESS	{CONFIG_SYS_SIMULATE_SPD_EEPROM, 0x50, 0x51}
#define CONFIG_SYS_MBYTES_SDRAM	(64)	/* 64MB fixed size for early-sdram-init */
#define CONFIG_PROG_SDRAM_TLB

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000

#define CONFIG_SYS_I2C_EEPROM_ADDR	(0xa8>>1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_SIZE		0x200	    /* Size of Environment vars */
#define CONFIG_ENV_OFFSET		0x0
#endif /* CONFIG_ENV_IS_IN_EEPROM */

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fff00000\0"					\
	"ramdisk_addr=fff10000\0"					\
	""

#define CONFIG_HAS_ETH0
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR        1

#ifndef CONFIG_BAMBOO_NAND
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#endif /* CONFIG_BAMBOO_NAND */

#ifdef CONFIG_440EP
/* USB */
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/*Comment this out to enable USB 1.1 device*/
#define USB_2_0_DEVICE
#endif /*CONFIG_440EP*/

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM

#ifdef CONFIG_BAMBOO_NAND
#define CONFIG_CMD_NAND
#endif

#define CONFIG_SUPPORT_VFAT

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#undef  CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CONFIG_SYS_PCI_TARGBASE        0x80000000 /* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE*/

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT
#define CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_ID       0xcafe	/* Whatever */

#endif	/* __CONFIG_H */
