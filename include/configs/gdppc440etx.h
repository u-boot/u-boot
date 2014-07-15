/*
 * (C) Copyright 2008
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * Based on include/configs/yosemite.h
 * (C) Copyright 2005-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * gdppc440etx.h - configuration for G&D 440EP/GR ETX-Module
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_440GR		1		/* Specific PPC440GR support */
#define CONFIG_HOSTNAME		gdppc440etx
#define CONFIG_440		1		/* ... PPC440 family         */
#define CONFIG_SYS_CLK_FREQ	66666666	/* external freq to pll      */

#define	CONFIG_SYS_TEXT_BASE	0xFFF80000

/*
 * Include common defines/options for all AMCC eval boards
 */
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F	1	/* call board_early_init_f*/
#define CONFIG_MISC_INIT_R		1	/* call misc_init_r()     */

#undef CONFIG_ZERO_BOOTDELAY_CHECK     /* ignore keypress on bootdelay==0 */
#define CONFIG_AUTOBOOT_KEYED          /* use key strings to stop autoboot */
#define CONFIG_AUTOBOOT_STOP_STR " "

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_FLASH_BASE		0xfc000000	/* start of FLASH    */
#define CONFIG_SYS_PCI_MEMBASE		0xa0000000	/* mapped pci memory */
#define CONFIG_SYS_PCI_MEMBASE1		CONFIG_SYS_PCI_MEMBASE  + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE2		CONFIG_SYS_PCI_MEMBASE1 + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE3		CONFIG_SYS_PCI_MEMBASE2 + 0x10000000

/*Don't change either of these*/
#define CONFIG_SYS_PCI_BASE		0xe0000000	/* internal PCI regs */
/*Don't change either of these*/

#define CONFIG_SYS_USB_DEVICE		0x50000000
#define CONFIG_SYS_BOOT_BASE_ADDR	0xf0000000

/*
 * Initial RAM & stack pointer (placed in SDRAM)
 */
#define CONFIG_SYS_INIT_RAM_DCACHE	1		/* d-cache as init ram*/
#define CONFIG_SYS_INIT_RAM_ADDR	0x70000000	/* DCache             */
#define CONFIG_SYS_INIT_RAM_SIZE		(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE \
					 - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	2	/* Use UART1			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#define CONFIG_SYS_EXT_SERIAL_CLOCK	11059200	/* ext. 11.059MHz clk */

/*
 * Environment
 * Define here the location of the environment variables (FLASH or EEPROM).
 * Note: DENX encourages to use redundant environment in FLASH.
 */
#define CONFIG_ENV_IS_IN_FLASH		1		/* FLASH for env. vars*/

/*
 * FLASH related
 */
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible*/
#define CONFIG_FLASH_CFI_DRIVER			/* Use common CFI driver      */
#define CONFIG_SYS_FLASH_CFI_AMD_RESET	1	/* AMD RESET for STM 29W320DB!*/

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors/chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT 	120000	/* Timeout/Flash Erase (in ms)*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout/Flash Write (in ms)*/

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1/* use buffered writes (20x faster)*/

#define CONFIG_SYS_FLASH_EMPTY_INFO /* print 'E' for empty sector on flinfo */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE		0x20000 /* size of one complete sector*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE			0x2000 /* Total Size of Env. Sector */

/* Address and size of Redundant Environment Sector */
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*
 * DDR SDRAM
 */
#undef CONFIG_SPD_EEPROM		/* Don't use SPD EEPROM for setup*/
#define CONFIG_SYS_KBYTES_SDRAM		(128 * 1024)    /* 128MB         */
#define CONFIG_SYS_SDRAM_BANKS		(2)

#define CONFIG_SDRAM_BANK0
#define CONFIG_SDRAM_BANK1

#define CONFIG_SYS_SDRAM0_TR0		0x410a4012
#define CONFIG_SYS_SDRAM0_WDDCTR	0x40000000
#define CONFIG_SYS_SDRAM0_RTR		0x04080000
#define CONFIG_SYS_SDRAM0_CFG0		0x80000000

#undef CONFIG_SDRAM_ECC

/*
 * I2C
 */
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000

/*
 * Default environment variables
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc180000\0"					\
	""

#define CONFIG_HAS_ETH1			1	/* add support for "eth1addr" */
#define CONFIG_PHY_ADDR			1
#define CONFIG_PHY1_ADDR		3

#ifdef DEBUG
#define CONFIG_PANIC_HANG
#endif

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_PCI
#undef CONFIG_CMD_EEPROM

/*
 * PCI stuff
 */

/* General PCI */
#define CONFIG_PCI				/* include pci support        */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#undef  CONFIG_PCI_PNP				/* do (not) pci plug-and-play */
#define CONFIG_PCI_SCAN_SHOW			/* show pci devices on startup*/
#define CONFIG_SYS_PCI_TARGBASE		0x80000000	/* PCIaddr mapped to \
							CONFIG_SYS_PCI_MEMBASE*/

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT
#define CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID	0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_ID	0xcafe	/* tbd */

/*
 * External Bus Controller (EBC) Setup
 */
#define CONFIG_SYS_FLASH		CONFIG_SYS_FLASH_BASE

/* Memory Bank 0 (NOR-FLASH) initialization */
#define CONFIG_SYS_EBC_PB0AP		0x03017200
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH | 0xda000)

#endif	/* __CONFIG_H */
