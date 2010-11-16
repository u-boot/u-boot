/*
 * (C) Copyright 2007-2008 Netstal Maschinen AG
 * Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
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
 * hcu5.h - configuration for HCU5 board (derived from sequoia.h)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_HCU5		1		/* Board is HCU5	*/
#define CONFIG_440EPX		1		/* Specific PPC440EPx	*/
#define CONFIG_440		1		/* ... PPC440 family	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_HOSTNAME		hcu5

#define	CONFIG_SYS_TEXT_BASE	0xFFFB0000

/*
 * Include common defines/options for all boards produced by Netstal Maschinen
 */
#include "netstal-common.h"

#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll	*/
#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MONITOR_LEN	(320 * 1024)	/* Reserve 320 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(256 * 1024) /* Reserve 256 kB for malloc() */

#define CONFIG_SYS_TLB_FOR_BOOT_FLASH  3
#define CONFIG_SYS_BOOT_BASE_ADDR	0xfff00000
#define CONFIG_SYS_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CONFIG_SYS_FLASH_BASE		0xfff80000	/* start of FLASH	*/
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_OCM_BASE		0xe0010000      /* ocm			*/
#define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_OCM_BASE
#define CONFIG_SYS_PCI_BASE		0xe0000000      /* Internal PCI regs	*/
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CONFIG_SYS_PCI_MEMBASE1	CONFIG_SYS_PCI_MEMBASE  + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE2	CONFIG_SYS_PCI_MEMBASE1 + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE3	CONFIG_SYS_PCI_MEMBASE2 + 0x10000000

#define CONFIG_SYS_USB2D0_BASE		0xe0000100
#define CONFIG_SYS_USB_DEVICE		0xe0000000
#define CONFIG_SYS_USB_HOST		0xe0000400

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache	*/
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_BASE	/* OCM			*/

#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef CONFIG_SYS_EXT_SERIAL_CLOCK	       /* external serial clock */
#define CONFIG_BAUDRATE		115200

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/

#undef	CONFIG_ENV_IS_IN_NVRAM
#define  CONFIG_ENV_IS_IN_FLASH
#undef	CONFIG_ENV_IS_IN_EEPROM
#undef  CONFIG_ENV_IS_NOWHERE

#ifdef  CONFIG_ENV_IS_IN_EEPROM
/* Put the environment after the SDRAM and bootstrap configuration */
#define PROM_SIZE	2048
#define CONFIG_SYS_BOOSTRAP_OPTION_OFFSET	 512
#define CONFIG_ENV_OFFSET	 (CONFIG_SYS_BOOSTRAP_OPTION_OFFSET + 0x10)
#define CONFIG_ENV_SIZE	(PROM_SIZE-CONFIG_ENV_OFFSET)
#endif

#ifdef CONFIG_ENV_IS_IN_FLASH
/* Put the environment in Flash */
#define CONFIG_ENV_SECT_SIZE	0x10000 /* size of one complete sector	*/
#define CONFIG_ENV_ADDR		((-CONFIG_SYS_MONITOR_LEN)-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		8*1024	/* 8 KB Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#endif

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MBYTES_SDRAM        (128)		/* 128 MB or 256 MB	*/
#define CONFIG_SYS_DDR_CACHED_ADDR	0x50000000	/* setup 2nd TLB cached here */
#undef  CONFIG_DDR_DATA_EYE		/* Do not use DDR2 optimization	*/
#define CONFIG_DDR_ECC		1	/* enable ECC			*/

/* Following two definitions must be kept in sync with config.h of vxWorks */
#define USER_RESERVED_MEM     (   0)  /* in kB */
#define PM_RESERVED_MEM       (  64)  /* in kB: pmLib reserved area size */
#define CONFIG_PRAM           ( USER_RESERVED_MEM + PM_RESERVED_MEM )

#define CONFIG_SYS_MEM_TOP_HIDE	(4 << 10) /* don't use last 4kbytes	*/
					/* 440EPx errata CHIP 11	*/

/*-----------------------------------------------------------------------
 * I2C stuff for a ATMEL AT24C16 (2kB holding ENV, we are using the
 * the second internal I2C controller of the PPC440EPx
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_SPD_BUS_NUM	1

/* Setup some board specific values for the default environment variables */
#define CONFIG_IPADDR		172.25.1.15

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_NETSTAL_DEF_ENV						\
	CONFIG_NETSTAL_DEF_ENV_POWERPC					\
	""

#define CONFIG_M88E1111_PHY	1
#define	CONFIG_IBM_EMAC4_V4	1

#define CONFIG_HAS_ETH1	1	/* add support for "eth1addr" */
#define CONFIG_PHY1_ADDR	2

/* USB */
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

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

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_USB

/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_MEMORY   | \
				 CONFIG_SYS_POST_UART	   | \
				 CONFIG_SYS_POST_I2C	   | \
				 CONFIG_SYS_POST_CACHE	   | \
				 CONFIG_SYS_POST_FPU	   | \
				 CONFIG_SYS_POST_ETHER	   | \
				 CONFIG_SYS_POST_SPR)

#define CONFIG_SYS_POST_UART_TABLE	{ CONFIG_SYS_NS16550_COM1 }
#define CONFIG_SYS_POST_CACHE_ADDR	0x7fff0000 /* free virtual address	*/
#define CONFIG_SYS_CONSOLE_IS_IN_ENV /* Otherwise it catches logbuffer as output */

#define CONFIG_SUPPORT_VFAT

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	        16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	        CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000  /* default load address	*/

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI		1	/* include pci support	        */
#undef CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CONFIG_SYS_PCI_TARGBASE        0x80000000 /* PCIaddr map to CONFIG_SYS_PCI_MEMBASE*/

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT
#define CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC				*/
#define CONFIG_SYS_PCI_SUBSYS_ID       0xcafe	/* Whatever			*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * Flash
 *----------------------------------------------------------------------*/

/* Use common CFI driver */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
/* board provides its own flash_init code */
#define CONFIG_FLASH_CFI_LEGACY		1
#define CONFIG_SYS_FLASH_CFI_WIDTH		FLASH_CFI_8BIT
#define CONFIG_SYS_FLASH_LEGACY_512Kx8 1

/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	8	/* max number of sectors on one chip */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS_1		0xC8000000 /* CAN */
#define CONFIG_SYS_CS_2		0xCC000000 /* CPLD and IMC-Bus Standard */
#define CONFIG_SYS_CPLD		CONFIG_SYS_CS_2
#define CONFIG_SYS_CS_3		0xCE000000 /* CPLD and IMC-Bus Fast  */

#define CONFIG_SYS_BOOTFLASH_CS	0	/* Boot Flash chip connected to CSx */
#define CONFIG_SYS_EBC_PB0AP		0x02005400
#define CONFIG_SYS_EBC_PB0CR		0xFFF18000 /* (CONFIG_SYS_FLASH | 0xda000)  */
#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE	/* FLASH bank #0	*/

/* Memory Bank 1 CAN-Chips initialization				*/
#define CONFIG_SYS_EBC_PB1AP		0x02054500
#define CONFIG_SYS_EBC_PB1CR		0xC8018000

/* Memory Bank 2 CPLD/IMC-Bus standard initialization			*/
#define CONFIG_SYS_EBC_PB2AP		0x01840300
#define CONFIG_SYS_EBC_PB2CR		0xCC0BA000

/* Memory Bank 3 IMC-Bus fast mode initialization			*/
#define CONFIG_SYS_EBC_PB3AP		0x01800300
#define CONFIG_SYS_EBC_PB3CR		0xCE0BA000

/* Memory Bank 4 (not used) initialization				*/
#undef CONFIG_SYS_EBC_PB4AP
#undef CONFIG_SYS_EBC_PB4CR

/* Memory Bank 5 (not used) initialization				*/
#undef CONFIG_SYS_EBC_PB5AP
#undef CONFIG_SYS_EBC_PB5CR

#define HCU_CPLD_VERSION_REGISTER ( CONFIG_SYS_CPLD + 0x0F00000 )
#define HCU_HW_VERSION_REGISTER   ( CONFIG_SYS_CPLD + 0x1400000 )

#define CONFIG_SYS_HUSH_PARSER                 /* use "hush" command parser    */
#ifdef  CONFIG_SYS_HUSH_PARSER
	#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	    /* which serial port to use */
#endif

#endif	/* __CONFIG_H */
