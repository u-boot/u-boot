/*
 * (C) Copyright 2001-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_PMC405		1	/* ...on a PMC405 board		*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

#define CONFIG_SYS_CLK_FREQ	33330000 /* external frequency to pll	*/

#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

/* Only interrupt boot if space is pressed. */
#define CONFIG_AUTOBOOT_KEYED 1
#define CONFIG_AUTOBOOT_PROMPT	\
	"Press SPACE to abort autoboot in %d seconds\n", bootdelay
#undef CONFIG_AUTOBOOT_DELAY_STR
#define CONFIG_AUTOBOOT_STOP_STR " "

#undef CONFIG_BOOTARGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_PREBOOT			/* enable preboot variable	*/

#define CFG_BOOTM_LEN		0x1000000 /* support booting of huge images */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE 1	/* allow baudrate change	*/

#define CONFIG_NET_MULTI	1
#undef  CONFIG_HAS_ETH1

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_LXT971_NO_SLEEP	1	/* disable sleep mode in LXT971	*/
#define CONFIG_RESET_PHY_R	1	/* use reset_phy()		*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BSP
#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ELF
#define CONFIG_CMD_DATE
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_I2C
#define CONFIG_CMD_PING
#define CONFIG_CMD_UNIVERSE
#define CONFIG_CMD_EEPROM

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_RTC_MC146818		/* DS1685 is MC146818 compatible */
#define CONFIG_SYS_RTC_REG_BASE_ADDR	0xF0000500 /* RTC Base Address */

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */

#undef CONFIG_SYS_HUSH_PARSER			/* use "hush" command parser */
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Argument Buffer Sz */

#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device */

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1	/* don't print console info */

#define CONFIG_AUTO_COMPLETE		1       /* add autocompletion support */

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM */

#undef CONFIG_SYS_EXT_SERIAL_CLOCK		/* no external serial clock */
#define CONFIG_SYS_BASE_BAUD	806400

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1	/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */
#define CONFIG_LOOPW		1	/* enable loopw command */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

#define CONFIG_SYS_RX_ETH_BUFFER	16

/*
 * PCI stuff
 */
#define PCI_HOST_ADAPTER	0	/* configure as pci adapter	*/
#define PCI_HOST_FORCE		1	/* configure as pci host	*/
#define PCI_HOST_AUTO		2	/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST	PCI_HOST_AUTO   /* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CONFIG_PCI_CONFIG_HOST_BRIDGE 1	/* don't skip host bridge config */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x12FE  /* PCI Vendor ID: esd gmbh */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID_NONMONARCH 0x0408 /* PCI Device ID */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID_MONARCH 0x0409 /* PCI Device ID */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID pmc405_pci_subsys_deviceid()

#define CONFIG_SYS_PCI_CLASSCODE       0x0b20 /* Processor/PPC */

#define CONFIG_SYS_PCI_PTM1LA  (bd->bi_memstart) /* point to sdram	*/
#define CONFIG_SYS_PCI_PTM1MS  (~(bd->bi_memsize - 1) | 1) /* memsize, enable */
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000	/* Host: use this pci address */
#define CONFIG_SYS_PCI_PTM2LA  0xef000000	/* point to internal regs */
#define CONFIG_SYS_PCI_PTM2MS  0xff000001	/* 16MB, enable */
#define CONFIG_SYS_PCI_PTM2PCI 0x00000000	/* Host: use this pci address */

#define CONFIG_PCI_4xx_PTM_OVERWRITE	1 /* overwrite PTMx settings by env */

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_MONITOR_BASE		TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(~(TEXT_BASE) + 1)
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024) /* 128 kB for malloc() */

#define CONFIG_PRAM			0 /* use pram variable to overwrite */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20) /* Initial Memory map for Linux */

/*
 * FLASH organization
 */
#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_INCREMENT	0x01000000

#define CONFIG_SYS_FLASH_CFI		1 /* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER		1 /* Use the common driver */
#define CONFIG_SYS_FLASH_PROTECTION	1 /* don't use hardware protection */
#define CONFIG_SYS_FLASH_AUTOPROTECT_LIST {{0xfff80000, 0x80000}}
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1 /* use buffered writes (faster) */
#define CONFIG_SYS_MAX_FLASH_BANKS	2 /* max num of flash banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE, \
			CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_INCREMENT}
#define CONFIG_SYS_MAX_FLASH_SECT	128 /* max num of sects on one chip */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sector on fli */

/*
 * Environment Variable setup
 */
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */

/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_OFFSET	0x000
#define CONFIG_ENV_SIZE		0x800 /* 2048 bytes may be used for env vars */

#define CONFIG_SYS_NVRAM_BASE_ADDR	0xF0000500	/* NVRAM base address */
#define CONFIG_SYS_NVRAM_SIZE		242		/* NVRAM size */

/*
 * I2C EEPROM (CAT24WC16) for environment
 */
#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		100000 /* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT24W16 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address */
/* mask of address bits that overflow into the "EEPROM chip address" */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24W16 has */
					/* 16 byte page write mode using*/
					/* last	4 bits of the address */

#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10 /* and takes up to 10 msec */

/*
 * External Bus Controller (EBC) Setup
 */
#define FLASH0_BA	0xFF000000	/* FLASH 0 Base Address	*/
#define FLASH1_BA	0xFE000000	/* FLASH 1 Base Address	*/
#define CAN_BA		0xF0000000	/* CAN Base Addres	*/
#define RTC_BA		0xF0000500	/* RTC Base Address	*/
#define NVRAM_BA	0xF0200000	/* NVRAM Base Address	*/

/* Memory Bank 0 (Flash Bank 0) initialization */
#define CONFIG_SYS_EBC_PB0AP	0x92015480
/* BAS=0xFF0,BS=16MB,BU=R/W,BW=16bit */
#define CONFIG_SYS_EBC_PB0CR	(FLASH0_BA | 0x9A000)

/* Memory Bank 1 (Flash Bank 1) initialization */
#define CONFIG_SYS_EBC_PB1AP	0x92015480
/* BAS=0xFE0,BS=16MB,BU=R/W,BW=16bit*/
#define CONFIG_SYS_EBC_PB1CR	(FLASH1_BA | 0x9A000)

/* Memory Bank 2 (CAN0, 1, RTC) initialization */
/* TWT=5,TH=2,CSN=0,OEN=0,WBN=0,WBF=0      */
#define CONFIG_SYS_EBC_PB2AP	0x03000440
/* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit */
#define CONFIG_SYS_EBC_PB2CR	(CAN_BA | 0x18000)

/* Memory Bank 3 -> unused */

/* Memory Bank 4 (NVRAM) initialization */
/* TWT=5,TH=2,CSN=0,OEN=0,WBN=0,WBF=0 */
#define CONFIG_SYS_EBC_PB4AP	0x03000440
/* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit */
#define CONFIG_SYS_EBC_PB4CR	(NVRAM_BA | 0x18000)

/*
 * FPGA stuff
 */
/* FPGA program pin configuration */
#define CONFIG_SYS_FPGA_PRG		0x04000000 /* JTAG TMS pin (output) */
#define CONFIG_SYS_FPGA_CLK		0x02000000 /* JTAG TCK pin (output) */
#define CONFIG_SYS_FPGA_DATA		0x01000000 /* JTAG TDO pin (output) */
#define CONFIG_SYS_FPGA_INIT		0x00010000 /* unused (ppc input) */
#define CONFIG_SYS_FPGA_DONE		0x00008000 /* JTAG TDI pin (input) */

/* pass Ethernet MAC to VxWorks */
#define CONFIG_SYS_VXWORKS_MAC_PTR	0x00000000

/*
 * GPIOs
 */
#define CONFIG_SYS_VPEN			(0x80000000 >>  3) /* GPIO3 */
#define CONFIG_SYS_NONMONARCH		(0x80000000 >> 14) /* GPIO14 */
#define CONFIG_SYS_XEREADY		(0x80000000 >> 15) /* GPIO15 */
#define CONFIG_SYS_INTA_FAKE		(0x80000000 >> 19) /* GPIO19 */
#define CONFIG_SYS_SELF_RST		(0x80000000 >> 21) /* GPIO21 */
#define CONFIG_SYS_REV1_2		(0x80000000 >> 23) /* GPIO23 */

/*
 * Definitions for initial stack pointer and data area (in data cache)
 */

/* use on chip memory (OCM) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000

/* inside of SDRAM */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR

/* End of used area in RAM */
#define CONFIG_SYS_INIT_RAM_END		CONFIG_SYS_OCM_DATA_SIZE

/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
					 CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP

#endif /* __CONFIG_H */
