/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 *
 * Configuration settings for the AmigaOneG3SE board.
 *
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_AMIGAONEG3SE	1

#define CONFIG_BOARD_EARLY_INIT_F 1
#define CONFIG_MISC_INIT_R	1

#define CONFIG_VERY_BIG_RAM	1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#undef CONFIG_CLOCKS_IN_MHZ		/* clocks passed to Linux in Hz */

#define CONFIG_BOOTARGS		"root=/dev/ram rw ramdisk_size=4096"

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_AMIGA_PARTITION


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_NET
#define CONFIG_CMD_IDE
#define CONFIG_CMD_FDC
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_USB
#define CONFIG_CMD_BSP
#define CONFIG_CMD_PCI


#define CONFIG_PCI		1
/* #define CONFIG_PCI_SCAN_SHOW 1 */
#define CONFIG_PCI_PNP		1	/* PCI plug-and-play */

#define atoi(x)		simple_strtoul(x,NULL,10)

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"] "		/* Monitor Command Prompt	*/

#define CONFIG_SYS_HUSH_PARSER		1	/* use "hush" command parser	*/
/* #undef CONFIG_SYS_HUSH_PARSER */
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/

/* Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MAXARGS	64		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR	0x00500000	/* Default load address		*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE	    0x00000000
#define CONFIG_SYS_FLASH_BASE	    0xFFF00000
#define CONFIG_SYS_FLASH_MAX_SIZE  0x00080000
/* Maximum amount of RAM.
 */
#define CONFIG_SYS_MAX_RAM_SIZE    0x80000000	/* 2G			*/

#define CONFIG_SYS_RESET_ADDRESS   0xFFF00100

#define CONFIG_SYS_MONITOR_BASE    TEXT_BASE

#define CONFIG_SYS_MONITOR_LEN	    (768 << 10) /* Reserve 512 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN	    (2500 << 10) /* Reserve 128 kB for malloc() */

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_SDRAM_BASE && \
    CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_MAX_RAM_SIZE
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MEMTEST_START	0x00004000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x02000000	/* 0 ... 32 MB in DRAM	*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */

/* Size in bytes reserved for initial data
 */
/* HJF: used to be 0x400000 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_END	0x8000
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_INIT_RAM_LOCK

/*
 * Temporary buffer for serial data until the real serial driver
 * is initialised (memtest will destroy this buffer)
 */
#define CONFIG_SYS_SCONSOLE_ADDR     CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SYS_SCONSOLE_SIZE     0x0002000

/* SDRAM 0 - 256MB
 */

/*HJF: #define CONFIG_SYS_IBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT0U (CONFIG_SYS_SDRAM_BASE | BATU_BL_4M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_DBAT0U CONFIG_SYS_IBAT0U*/

#define CONFIG_SYS_DBAT0L	      (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT0U	      (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT0L      (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U      (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
/* PCI Range
 */
#define CONFIG_SYS_DBAT1L	 (0x80000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT1U	 (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT1L	 (0x80000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U	 (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)
/* HJF:
#define CONFIG_SYS_IBAT1L ((CONFIG_SYS_SDRAM_BASE+CONFIG_SYS_INIT_RAM_ADDR) | BATL_PP_RW)
#define CONFIG_SYS_IBAT1U ((CONFIG_SYS_SDRAM_BASE+CONFIG_SYS_INIT_RAM_ADDR) | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L ((CONFIG_SYS_SDRAM_BASE+CONFIG_SYS_INIT_RAM_ADDR + 0x20000) | BATL_PP_RW )
#define CONFIG_SYS_DBAT1U ((CONFIG_SYS_SDRAM_BASE+CONFIG_SYS_INIT_RAM_ADDR + 0x20000) | BATU_BL_256M | BATU_VS | BATU_VP)
*/

/* Init RAM in the CPU DCache (no backing memory)
 */
#define CONFIG_SYS_DBAT2L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT2U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
/* This used to be commented out */
#define CONFIG_SYS_IBAT2L	  CONFIG_SYS_DBAT2L
/* This here too */
#define CONFIG_SYS_IBAT2U	  CONFIG_SYS_DBAT2U


/* I/O and PCI memory at 0xf0000000
 */
#define CONFIG_SYS_DBAT3L	(0xf0000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	(0xf0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT3L	(0xf0000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U	(0xf0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 */
#define CONFIG_SYS_HZ		1000
#define CONFIG_SYS_BUS_HZ	133000000 /* bus speed - 100 mhz		*/
#define CONFIG_SYS_CPU_CLK	133000000
#define CONFIG_SYS_BUS_CLK	133000000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* Max number of flash banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	8	/* Max number of sectors in one bank	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000	/* Timeout for Flash Write (in ms)	*/

/*
 * Environment is stored in NVRAM.
 */
#define CONFIG_ENV_IS_IN_NVRAM	1
#define CONFIG_ENV_ADDR		0xFD0E0000 /* This should be 0xFD0E0000, but we skip bytes to
					    * protect softex's settings for now.
					    * Original 768 bytes where not enough.
					    */
#define CONFIG_ENV_SIZE		0x8000	   /* Size of the Environment. See comment above */

#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1 /* stdin/stdout/stderr are in environment */
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE	1
#define CONFIG_ENV_OVERWRITE 1

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * L2 cache
 */
#define CONFIG_SYS_L2
#define L2_INIT	  (L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
		   L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)
#define L2_ENABLE (L2_INIT | L2CR_L2E)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/


/*-----------------------------------------------------------------------
 * IDE ATAPI Configuration
 */

#define CONFIG_ATAPI		1
#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_IDE_MAXDEVICE	4
#define CONFIG_ISO_PARTITION	1

#define CONFIG_SYS_ATA_BASE_ADDR	0xFE000000  /* was: via_get_base_addr() */
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x1F0
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x170

#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_DATA_OFFSET	0
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0200

/*-----------------------------------------------------------------------
 * Disk-On-Chip configuration
 */

#define CONFIG_SYS_MAX_DOC_DEVICE	1	/* Max number of DOC devices		*/

#define CONFIG_SYS_DOC_SUPPORT_2000
#undef CONFIG_SYS_DOC_SUPPORT_MILLENNIUM

/*-----------------------------------------------------------------------
  RTC
*/
#define CONFIG_RTC_MC146818

/*-----------------------------------------------------------------------
 * NS16550 Configuration
 */

#define CONFIG_SYS_NS16550

#define CONFIG_SYS_NS16550_COM1 0xFE0003F8
#define CONFIG_SYS_NS16550_COM2 0xFE0002F8

#define CONFIG_SYS_NS16550_REG_SIZE 1

/* base address for ISA I/O
 */
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS 0xFE000000

/* ISA Interrupt stuff (taken from JWL) */

#define ISA_INT1_OCW1		0x21
#define ISA_INT2_OCW1		0xA1
#define ISA_INT1_OCW2		0x20
#define ISA_INT2_OCW2		0xA0
#define ISA_INT1_OCW3		0x20
#define ISA_INT2_OCW3		0xA0

#define ISA_INT1_ICW1		0x20
#define ISA_INT2_ICW1		0xA0
#define ISA_INT1_ICW2		0x21
#define ISA_INT2_ICW2		0xA1
#define ISA_INT1_ICW3		0x21
#define ISA_INT2_ICW3		0xA1
#define ISA_INT1_ICW4		0x21
#define ISA_INT2_ICW4		0xA1


/*
 * misc
 */

#define CONFIG_NET_MULTI
#define CONFIG_SYS_BOARD_ASM_INIT
#define CONFIG_LAST_STAGE_INIT

/* #define CONFIG_ETHADDR	00:09:D2:10:00:76 */
/* #define CONFIG_IPADDR	192.168.0.2 */
/* #define CONFIG_NETMASK	255.255.255.240 */
/* #define CONFIG_GATEWAYIP	192.168.0.3 */

#define CONFIG_3COM
/* #define CONFIG_BOOTP_RANDOM_DELAY */

/*
 * USB configuration
 */
#define CONFIG_USB_UHCI		1
#define CONFIG_USB_STORAGE	1
#define CONFIG_USB_KEYBOARD	1
#define CONFIG_SYS_STDIO_DEREGISTER	1 /* needed by CONFIG_USB_KEYBOARD */

/*
 * Autoboot stuff
 */
#define CONFIG_BOOTDELAY	5 /* Boot automatically after five seconds */
#define CONFIG_PREBOOT		""
#define CONFIG_BOOTCOMMAND	"fdcboot; diskboot"
#define CONFIG_MENUPROMPT	\
	"Press any key to interrupt autoboot: %2d ", bootdelay
#define CONFIG_MENUKEY		' '
#define CONFIG_MENUCOMMAND	"menu"
/* #define CONFIG_AUTOBOOT_KEYED */

/*
 * Extra ENV stuff
 */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"stdout=vga\0"				\
	"stdin=ps2kbd\0"			\
	"ide_doreset=on\0"			\
	"ide_maxbus=2\0"			\
	"ide_cd_timeout=30\0"			\
	"menucmd=menu\0"			\
	"pci_irqa=9\0"				\
	"pci_irqa_select=edge\0"		\
	"pci_irqb=10\0"				\
	"pci_irqb_select=edge\0"		\
	"pci_irqc=11\0"				\
	"pci_irqc_select=edge\0"		\
	"pci_irqd=7\0"				\
	"pci_irqd_select=edge\0"


/* #define CONFIG_MII		1 */
/* #define CONFIG_BITBANGMII	1 */


#endif	/* __CONFIG_H */
