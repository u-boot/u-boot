/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2003
 * DAVE Srl
 *
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * Credits: Stefan Roese, Wolfgang Denk
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_PPCHAMELEON_MODULE_BA	0	/* Basic    Model */
#define CONFIG_PPCHAMELEON_MODULE_ME	1	/* Medium   Model */
#define CONFIG_PPCHAMELEON_MODULE_HI	2	/* High-End Model */
#ifndef CONFIG_PPCHAMELEON_MODULE_MODEL
#define CONFIG_PPCHAMELEON_MODULE_MODEL CONFIG_PPCHAMELEON_MODULE_BA
#endif


/* Only one of the following two symbols must be defined (default is 25 MHz)
 * CONFIG_PPCHAMELEON_CLK_25
 * CONFIG_PPCHAMELEON_CLK_33
 */
#if (!defined(CONFIG_PPCHAMELEON_CLK_25) && !defined(CONFIG_PPCHAMELEON_CLK_33))
#define CONFIG_PPCHAMELEON_CLK_25
#endif

#if (defined(CONFIG_PPCHAMELEON_CLK_25) && defined(CONFIG_PPCHAMELEON_CLK_33))
#error "* Two external frequencies (SysClk) are defined! *"
#endif

#undef	CONFIG_PPCHAMELEON_SMI712

/*
 * Debug stuff
 */
#undef	__DEBUG_START_FROM_SRAM__
#define __DISABLE_MACHINE_EXCEPTION__

#ifdef __DEBUG_START_FROM_SRAM__
#define CONFIG_SYS_DUMMY_FLASH_SIZE		1024*1024*4
#endif

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405EP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_PPCHAMELEONEVB	1	/* ...on a PPChameleonEVB board */

#define	CONFIG_SYS_TEXT_BASE	0xFFFB0000	/* Reserve 320 kB for Monitor */
#define CONFIG_SYS_LDSCRIPT	"board/dave/PPChameleonEVB/u-boot.lds"

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/


#ifdef CONFIG_PPCHAMELEON_CLK_25
# define CONFIG_SYS_CLK_FREQ	25000000 /* external frequency to pll	*/
#elif (defined (CONFIG_PPCHAMELEON_CLK_33))
# define CONFIG_SYS_CLK_FREQ	33333333 /* external frequency to pll	*/
#else
# error "* External frequency (SysClk) not defined! *"
#endif

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#undef	CONFIG_BOOTARGS

/* Ethernet stuff */
#define CONFIG_ENV_OVERWRITE /* Let the user to change the Ethernet MAC addresses */
#define CONFIG_ETHADDR	00:50:c2:1e:af:fe
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR 00:50:c2:1e:af:fd

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#undef CONFIG_EXT_PHY

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#ifndef	 CONFIG_EXT_PHY
#define CONFIG_PHY_ADDR		1	/* EMAC0 PHY address		*/
#define CONFIG_PHY1_ADDR	2	/* EMAC1 PHY address		*/
#else
#define CONFIG_PHY_ADDR		2	/* PHY address			*/
#endif
#define CONFIG_PHY_CLK_FREQ	EMAC_STACR_CLK_66MHZ


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

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SNTP


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_RTC_M41T11	1	/* uses a M41T00 RTC		*/
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#define CONFIG_SYS_M41T11_BASE_YEAR	1900

/*
 * SDRAM configuration (please see cpu/ppc/sdram.[ch])
 */
#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

/* SDRAM timings used in datasheet */
#define CONFIG_SYS_SDRAM_CL            2
#define CONFIG_SYS_SDRAM_tRP           20
#define CONFIG_SYS_SDRAM_tRC           65
#define CONFIG_SYS_SDRAM_tRCD          20
#undef  CONFIG_SYS_SDRAM_tRFC

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt	*/

#undef	CONFIG_SYS_HUSH_PARSER			/* use "hush" command parser	*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device	*/

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef	CONFIG_SYS_EXT_SERIAL_CLOCK		/* no external serial clock used */
#define CONFIG_SYS_BASE_BAUD		691200

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/*-----------------------------------------------------------------------
 * NAND-FLASH stuff
 *-----------------------------------------------------------------------
 */

/*
 * nand device 1 on dave (PPChameleonEVB) needs more time,
 * so we just introduce additional wait in nand_wait(),
 * effectively for both devices.
 */
#define PPCHAMELON_NAND_TIMER_HACK

#define CONFIG_SYS_NAND0_BASE 0xFF400000
#define CONFIG_SYS_NAND1_BASE 0xFF000000
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND0_BASE, CONFIG_SYS_NAND1_BASE }
#define NAND_BIG_DELAY_US	25
#define CONFIG_SYS_MAX_NAND_DEVICE	2	/* Max number of NAND devices */

#define CONFIG_SYS_NAND0_CE  (0x80000000 >> 1)	 /* our CE is GPIO1 */
#define CONFIG_SYS_NAND0_RDY (0x80000000 >> 4)	 /* our RDY is GPIO4 */
#define CONFIG_SYS_NAND0_CLE (0x80000000 >> 2)	 /* our CLE is GPIO2 */
#define CONFIG_SYS_NAND0_ALE (0x80000000 >> 3)	 /* our ALE is GPIO3 */

#define CONFIG_SYS_NAND1_CE  (0x80000000 >> 14)  /* our CE is GPIO14 */
#define CONFIG_SYS_NAND1_RDY (0x80000000 >> 31)  /* our RDY is GPIO31 */
#define CONFIG_SYS_NAND1_CLE (0x80000000 >> 15)  /* our CLE is GPIO15 */
#define CONFIG_SYS_NAND1_ALE (0x80000000 >> 16)  /* our ALE is GPIO16 */

#define MACRO_NAND_DISABLE_CE(nandptr) do \
{ \
	switch((unsigned long)nandptr) \
	{ \
	    case CONFIG_SYS_NAND0_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) | CONFIG_SYS_NAND0_CE); \
		break; \
	    case CONFIG_SYS_NAND1_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) | CONFIG_SYS_NAND1_CE); \
		break; \
	} \
} while(0)

#define MACRO_NAND_ENABLE_CE(nandptr) do \
{ \
	switch((unsigned long)nandptr) \
	{ \
	    case CONFIG_SYS_NAND0_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CONFIG_SYS_NAND0_CE); \
		break; \
	    case CONFIG_SYS_NAND1_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CONFIG_SYS_NAND1_CE); \
		break; \
	} \
} while(0)

#define MACRO_NAND_CTL_CLRALE(nandptr) do \
{ \
	switch((unsigned long)nandptr) \
	{ \
	    case CONFIG_SYS_NAND0_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CONFIG_SYS_NAND0_ALE); \
		break; \
	    case CONFIG_SYS_NAND1_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CONFIG_SYS_NAND1_ALE); \
		break; \
	} \
} while(0)

#define MACRO_NAND_CTL_SETALE(nandptr) do \
{ \
	switch((unsigned long)nandptr) \
	{ \
	    case CONFIG_SYS_NAND0_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) | CONFIG_SYS_NAND0_ALE); \
		break; \
	    case CONFIG_SYS_NAND1_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) | CONFIG_SYS_NAND1_ALE); \
		break; \
	} \
} while(0)

#define MACRO_NAND_CTL_CLRCLE(nandptr) do \
{ \
	switch((unsigned long)nandptr) \
	{ \
	    case CONFIG_SYS_NAND0_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CONFIG_SYS_NAND0_CLE); \
		break; \
	    case CONFIG_SYS_NAND1_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CONFIG_SYS_NAND1_CLE); \
		break; \
	} \
} while(0)

#define MACRO_NAND_CTL_SETCLE(nandptr) do { \
	switch((unsigned long)nandptr) { \
	case CONFIG_SYS_NAND0_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) | CONFIG_SYS_NAND0_CLE); \
		break; \
	case CONFIG_SYS_NAND1_BASE: \
		out32(GPIO0_OR, in32(GPIO0_OR) | CONFIG_SYS_NAND1_CLE); \
		break; \
	} \
} while(0)

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure as pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_FORCE	 /* select pci host function	 */
#undef	CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1014	/* PCI Vendor ID: IBM	*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0000	/* PCI Device ID: ---	*/
#define CONFIG_SYS_PCI_CLASSCODE	0x0b20	/* PCI Class Code: Processor/PPC*/

#define CONFIG_SYS_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CONFIG_SYS_PCI_PTM1MS	0xfc000001	/* 64MB, enable hard-wired to 1 */
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000	/* Host: use this pci address	*/
#define CONFIG_SYS_PCI_PTM2LA	0xffc00000	/* point to flash		*/
#define CONFIG_SYS_PCI_PTM2MS	0xffc00001	/* 4MB, enable			*/
#define CONFIG_SYS_PCI_PTM2PCI 0x04000000	/* Host: use this pci address	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000

/* Reserve 256 kB for Monitor	*/
/*
#define CONFIG_SYS_FLASH_BASE		0xFFFC0000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
*/

/* Reserve 320 kB for Monitor	*/
#define CONFIG_SYS_FLASH_BASE		0xFFFB0000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(320 * 1024)

#define CONFIG_SYS_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned short	/* flash word size (width)	*/
#define CONFIG_SYS_FLASH_ADDR0		0x5555	/* 1st address for flash config cycles	*/
#define CONFIG_SYS_FLASH_ADDR1		0x2AAA	/* 2nd address for flash config cycles	*/
/*
 * The following defines are added for buggy IOP480 byte interface.
 * All other boards should use the standard values (CPCI405 etc.)
 */
#define CONFIG_SYS_FLASH_READ0		0x0000	/* 0 is standard			*/
#define CONFIG_SYS_FLASH_READ1		0x0001	/* 1 is standard			*/
#define CONFIG_SYS_FLASH_READ2		0x0002	/* 2 is standard			*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

/*-----------------------------------------------------------------------
 * Environment Variable setup
 */
#ifdef ENVIRONMENT_IN_EEPROM

#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x100	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x700	/* 2048-256 bytes may be used for env vars (total size of a CAT24WC16 is 2048 bytes)*/

#else	/* DEFAULT: environment in flash, using redundand flash sectors */

#define CONFIG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */
#define CONFIG_ENV_ADDR		0xFFFF8000	/* environment starts at the first small sector */
#define CONFIG_ENV_SECT_SIZE	0x2000	/* 8196 bytes may be used for env vars*/
#define CONFIG_ENV_ADDR_REDUND	0xFFFFA000
#define CONFIG_ENV_SIZE_REDUND	0x2000

#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

#endif	/* ENVIRONMENT_IN_EEPROM */


#define CONFIG_SYS_NVRAM_BASE_ADDR	0xF0000500		/* NVRAM base address	*/
#define CONFIG_SYS_NVRAM_SIZE		242			/* NVRAM size		*/

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC16) for environment
 */
#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
/*#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has	*/
					/* 16 byte page write mode using*/
					/* last 4 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFFC00000	/* FLASH bank #0	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash Bank 0, NOR-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB0AP		0x92015480
#define CONFIG_SYS_EBC_PB0CR		0xFFC5A000  /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (External SRAM) initialization					*/
/* Since this must replace NOR Flash, we use the same settings for CS0		*/
#define CONFIG_SYS_EBC_PB1AP		0x92015480
#define CONFIG_SYS_EBC_PB1CR		0xFF85A000  /* BAS=0xFF8,BS=4MB,BU=R/W,BW=8bit	*/

/* Memory Bank 2 (Flash Bank 1, NAND-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB2AP		0x92015480
#define CONFIG_SYS_EBC_PB2CR		0xFF458000  /* BAS=0xFF4,BS=4MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (Flash Bank 2, NAND-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB3AP		0x92015480
#define CONFIG_SYS_EBC_PB3CR		0xFF058000  /* BAS=0xFF0,BS=4MB,BU=R/W,BW=8bit	*/

#ifdef CONFIG_PPCHAMELEON_SMI712
/*
 * Video console (graphic: SMI LynxEM)
 */
#define CONFIG_VIDEO
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_SMI_LYNXEM
#define CONFIG_VIDEO_LOGO
/*#define CONFIG_VIDEO_BMP_LOGO*/
#define CONFIG_CONSOLE_EXTRA_INFO
#define CONFIG_VGA_AS_SINGLE_DEVICE
/* This is the base address (on 405EP-side) used to generate I/O accesses on PCI bus */
#define CONFIG_SYS_ISA_IO 0xE8000000
/* see also drivers/video/videomodes.c */
#define CONFIG_SYS_DEFAULT_VIDEO_MODE 0x303
#endif

/*-----------------------------------------------------------------------
 * FPGA stuff
 */
/* FPGA internal regs */
#define CONFIG_SYS_FPGA_MODE		0x00
#define CONFIG_SYS_FPGA_STATUS		0x02
#define CONFIG_SYS_FPGA_TS		0x04
#define CONFIG_SYS_FPGA_TS_LOW		0x06
#define CONFIG_SYS_FPGA_TS_CAP0	0x10
#define CONFIG_SYS_FPGA_TS_CAP0_LOW	0x12
#define CONFIG_SYS_FPGA_TS_CAP1	0x14
#define CONFIG_SYS_FPGA_TS_CAP1_LOW	0x16
#define CONFIG_SYS_FPGA_TS_CAP2	0x18
#define CONFIG_SYS_FPGA_TS_CAP2_LOW	0x1a
#define CONFIG_SYS_FPGA_TS_CAP3	0x1c
#define CONFIG_SYS_FPGA_TS_CAP3_LOW	0x1e

/* FPGA Mode Reg */
#define CONFIG_SYS_FPGA_MODE_CF_RESET	0x0001
#define CONFIG_SYS_FPGA_MODE_TS_IRQ_ENABLE 0x0100
#define CONFIG_SYS_FPGA_MODE_TS_IRQ_CLEAR  0x1000
#define CONFIG_SYS_FPGA_MODE_TS_CLEAR	0x2000

/* FPGA Status Reg */
#define CONFIG_SYS_FPGA_STATUS_DIP0	0x0001
#define CONFIG_SYS_FPGA_STATUS_DIP1	0x0002
#define CONFIG_SYS_FPGA_STATUS_DIP2	0x0004
#define CONFIG_SYS_FPGA_STATUS_FLASH	0x0008
#define CONFIG_SYS_FPGA_STATUS_TS_IRQ	0x1000

#define CONFIG_SYS_FPGA_SPARTAN2	1		/* using Xilinx Spartan 2 now	 */
#define CONFIG_SYS_FPGA_MAX_SIZE	128*1024	/* 128kByte is enough for XC2S50E*/

/* FPGA program pin configuration */
#define CONFIG_SYS_FPGA_PRG		0x04000000	/* FPGA program pin (ppc output) */
#define CONFIG_SYS_FPGA_CLK		0x02000000	/* FPGA clk pin (ppc output)	 */
#define CONFIG_SYS_FPGA_DATA		0x01000000	/* FPGA data pin (ppc output)	 */
#define CONFIG_SYS_FPGA_INIT		0x00010000	/* FPGA init pin (ppc input)	 */
#define CONFIG_SYS_FPGA_DONE		0x00008000	/* FPGA done pin (ppc input)	 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM	*/

#define CONFIG_SYS_GBL_DATA_OFFSET    (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Definitions for GPIO setup (PPC405EP specific)
 *
 * GPIO0[0]	- External Bus Controller BLAST output
 * GPIO0[1-9]	- Instruction trace outputs -> GPIO
 * GPIO0[10-13] - External Bus Controller CS_1 - CS_4 outputs
 * GPIO0[14-16] - External Bus Controller ABUS3-ABUS5 outputs -> GPIO
 * GPIO0[17-23] - External Interrupts IRQ0 - IRQ6 inputs
 * GPIO0[24-27] - UART0 control signal inputs/outputs
 * GPIO0[28-29] - UART1 data signal input/output
 * GPIO0[30]	- EMAC0 input
 * GPIO0[31]	- EMAC1 reject packet as output
 */
#define CONFIG_SYS_GPIO0_OSRL		0x40000550
#define CONFIG_SYS_GPIO0_OSRH		0x00000110
#define CONFIG_SYS_GPIO0_ISR1L		0x00000000
/*#define CONFIG_SYS_GPIO0_ISR1H	0x15555445*/
#define CONFIG_SYS_GPIO0_ISR1H		0x15555444
#define CONFIG_SYS_GPIO0_TSRL		0x00000000
#define CONFIG_SYS_GPIO0_TSRH		0x00000000
#define CONFIG_SYS_GPIO0_TCR		0xF7FF8014

#define CONFIG_NO_SERIAL_EEPROM

/*--------------------------------------------------------------------*/

#ifdef CONFIG_NO_SERIAL_EEPROM

/*
!-----------------------------------------------------------------------
! Defines for entry options.
! Note: Because the 405EP SDRAM controller does not support ECC, ECC DIMMs that
!	are plugged in the board will be utilized as non-ECC DIMMs.
!-----------------------------------------------------------------------
*/
#undef		AUTO_MEMORY_CONFIG
#define		DIMM_READ_ADDR 0xAB
#define		DIMM_WRITE_ADDR 0xAA

/* Defines for CPC0_PLLMR1 Register fields */
#define PLL_ACTIVE		0x80000000
#define CPC0_PLLMR1_SSCS	0x80000000
#define PLL_RESET		0x40000000
#define CPC0_PLLMR1_PLLR	0x40000000
    /* Feedback multiplier */
#define PLL_FBKDIV		0x00F00000
#define CPC0_PLLMR1_FBDV	0x00F00000
#define PLL_FBKDIV_16		0x00000000
#define PLL_FBKDIV_1		0x00100000
#define PLL_FBKDIV_2		0x00200000
#define PLL_FBKDIV_3		0x00300000
#define PLL_FBKDIV_4		0x00400000
#define PLL_FBKDIV_5		0x00500000
#define PLL_FBKDIV_6		0x00600000
#define PLL_FBKDIV_7		0x00700000
#define PLL_FBKDIV_8		0x00800000
#define PLL_FBKDIV_9		0x00900000
#define PLL_FBKDIV_10		0x00A00000
#define PLL_FBKDIV_11		0x00B00000
#define PLL_FBKDIV_12		0x00C00000
#define PLL_FBKDIV_13		0x00D00000
#define PLL_FBKDIV_14		0x00E00000
#define PLL_FBKDIV_15		0x00F00000
    /* Forward A divisor */
#define PLL_FWDDIVA		0x00070000
#define CPC0_PLLMR1_FWDVA	0x00070000
#define PLL_FWDDIVA_8		0x00000000
#define PLL_FWDDIVA_7		0x00010000
#define PLL_FWDDIVA_6		0x00020000
#define PLL_FWDDIVA_5		0x00030000
#define PLL_FWDDIVA_4		0x00040000
#define PLL_FWDDIVA_3		0x00050000
#define PLL_FWDDIVA_2		0x00060000
#define PLL_FWDDIVA_1		0x00070000
    /* Forward B divisor */
#define PLL_FWDDIVB		0x00007000
#define CPC0_PLLMR1_FWDVB	0x00007000
#define PLL_FWDDIVB_8		0x00000000
#define PLL_FWDDIVB_7		0x00001000
#define PLL_FWDDIVB_6		0x00002000
#define PLL_FWDDIVB_5		0x00003000
#define PLL_FWDDIVB_4		0x00004000
#define PLL_FWDDIVB_3		0x00005000
#define PLL_FWDDIVB_2		0x00006000
#define PLL_FWDDIVB_1		0x00007000
    /* PLL tune bits */
#define PLL_TUNE_MASK		0x000003FF
#define PLL_TUNE_2_M_3		0x00000133	/*  2 <= M <= 3			*/
#define PLL_TUNE_4_M_6		0x00000134	/*  3 <	 M <= 6			*/
#define PLL_TUNE_7_M_10		0x00000138	/*  6 <	 M <= 10		*/
#define PLL_TUNE_11_M_14	0x0000013C	/* 10 <	 M <= 14		*/
#define PLL_TUNE_15_M_40	0x0000023E	/* 14 <	 M <= 40		*/
#define PLL_TUNE_VCO_LOW	0x00000000	/* 500MHz <= VCO <=  800MHz	*/
#define PLL_TUNE_VCO_HI		0x00000080	/* 800MHz <  VCO <= 1000MHz	*/

/* Defines for CPC0_PLLMR0 Register fields */
    /* CPU divisor */
#define PLL_CPUDIV		0x00300000
#define CPC0_PLLMR0_CCDV	0x00300000
#define PLL_CPUDIV_1		0x00000000
#define PLL_CPUDIV_2		0x00100000
#define PLL_CPUDIV_3		0x00200000
#define PLL_CPUDIV_4		0x00300000
    /* PLB divisor */
#define PLL_PLBDIV		0x00030000
#define CPC0_PLLMR0_CBDV	0x00030000
#define PLL_PLBDIV_1		0x00000000
#define PLL_PLBDIV_2		0x00010000
#define PLL_PLBDIV_3		0x00020000
#define PLL_PLBDIV_4		0x00030000
    /* OPB divisor */
#define PLL_OPBDIV		0x00003000
#define CPC0_PLLMR0_OPDV	0x00003000
#define PLL_OPBDIV_1		0x00000000
#define PLL_OPBDIV_2		0x00001000
#define PLL_OPBDIV_3		0x00002000
#define PLL_OPBDIV_4		0x00003000
    /* EBC divisor */
#define PLL_EXTBUSDIV		0x00000300
#define CPC0_PLLMR0_EPDV	0x00000300
#define PLL_EXTBUSDIV_2		0x00000000
#define PLL_EXTBUSDIV_3		0x00000100
#define PLL_EXTBUSDIV_4		0x00000200
#define PLL_EXTBUSDIV_5		0x00000300
    /* MAL divisor */
#define PLL_MALDIV		0x00000030
#define CPC0_PLLMR0_MPDV	0x00000030
#define PLL_MALDIV_1		0x00000000
#define PLL_MALDIV_2		0x00000010
#define PLL_MALDIV_3		0x00000020
#define PLL_MALDIV_4		0x00000030
    /* PCI divisor */
#define PLL_PCIDIV		0x00000003
#define CPC0_PLLMR0_PPFD	0x00000003
#define PLL_PCIDIV_1		0x00000000
#define PLL_PCIDIV_2		0x00000001
#define PLL_PCIDIV_3		0x00000002
#define PLL_PCIDIV_4		0x00000003

#ifdef CONFIG_PPCHAMELEON_CLK_25
/* CPU - PLB/SDRAM - EBC - OPB - PCI (assuming a 25.0 MHz input clock to the 405EP) */
#define PPCHAMELEON_PLLMR0_133_133_33_66_33	 (PLL_CPUDIV_1 | PLL_PLBDIV_1 |	 \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PPCHAMELEON_PLLMR1_133_133_33_66_33	 (PLL_FBKDIV_8	|  \
			      PLL_FWDDIVA_6 | PLL_FWDDIVB_4 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PPCHAMELEON_PLLMR0_200_100_50_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PPCHAMELEON_PLLMR1_200_100_50_33 (PLL_FBKDIV_8  |  \
			      PLL_FWDDIVA_4 | PLL_FWDDIVB_4 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PPCHAMELEON_PLLMR0_266_133_33_66_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |	\
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PPCHAMELEON_PLLMR1_266_133_33_66_33 (PLL_FBKDIV_8  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_4 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PPCHAMELEON_PLLMR0_333_111_37_55_55 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |	\
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |	\
			      PLL_MALDIV_1 | PLL_PCIDIV_2)
#define PPCHAMELEON_PLLMR1_333_111_37_55_55 (PLL_FBKDIV_10	|  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_4 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)

#elif (defined (CONFIG_PPCHAMELEON_CLK_33))

/* CPU - PLB/SDRAM - EBC - OPB - PCI (assuming a 33.3MHz input clock to the 405EP) */
#define PPCHAMELEON_PLLMR0_133_133_33_66_33	 (PLL_CPUDIV_1 | PLL_PLBDIV_1 |	 \
				  PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |	\
				  PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PPCHAMELEON_PLLMR1_133_133_33_66_33	 (PLL_FBKDIV_4	|  \
				  PLL_FWDDIVA_6 | PLL_FWDDIVB_6 |  \
				  PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PPCHAMELEON_PLLMR0_200_100_50_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |  \
				  PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |	\
				  PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PPCHAMELEON_PLLMR1_200_100_50_33 (PLL_FBKDIV_6  |  \
				  PLL_FWDDIVA_4 | PLL_FWDDIVB_4 |  \
				  PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PPCHAMELEON_PLLMR0_266_133_33_66_33 (PLL_CPUDIV_1 | PLL_PLBDIV_2 |	\
				  PLL_OPBDIV_2 | PLL_EXTBUSDIV_4 |	\
				  PLL_MALDIV_1 | PLL_PCIDIV_4)
#define PPCHAMELEON_PLLMR1_266_133_33_66_33 (PLL_FBKDIV_8  |  \
				  PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
				  PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)

#define PPCHAMELEON_PLLMR0_333_111_37_55_55 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |	\
				  PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |	\
				  PLL_MALDIV_1 | PLL_PCIDIV_2)
#define PPCHAMELEON_PLLMR1_333_111_37_55_55 (PLL_FBKDIV_10	|  \
				  PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
				  PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)

#else
#error "* External frequency (SysClk) not defined! *"
#endif

#if   (CONFIG_PPCHAMELEON_MODULE_MODEL == CONFIG_PPCHAMELEON_MODULE_HI)
/* Model HI */
#define PLLMR0_DEFAULT	PPCHAMELEON_PLLMR0_333_111_37_55_55
#define PLLMR1_DEFAULT	PPCHAMELEON_PLLMR1_333_111_37_55_55
#define CONFIG_SYS_OPB_FREQ	55555555
/* Model ME */
#elif (CONFIG_PPCHAMELEON_MODULE_MODEL == CONFIG_PPCHAMELEON_MODULE_ME)
#define PLLMR0_DEFAULT	PPCHAMELEON_PLLMR0_266_133_33_66_33
#define PLLMR1_DEFAULT	PPCHAMELEON_PLLMR1_266_133_33_66_33
#define CONFIG_SYS_OPB_FREQ	66666666
#else
/* Model BA (default) */
#define PLLMR0_DEFAULT	PPCHAMELEON_PLLMR0_133_133_33_66_33
#define PLLMR1_DEFAULT	PPCHAMELEON_PLLMR1_133_133_33_66_33
#define CONFIG_SYS_OPB_FREQ	66666666
#endif

#endif /* CONFIG_NO_SERIAL_EEPROM */

#define CONFIG_JFFS2_NAND 1			/* jffs2 on nand support */
#define NAND_CACHE_PAGES 16			/* size of nand cache in 512 bytes pages */

/*
 * JFFS2 partitions
 */

/* No command line, one static partition */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nand0"
#define CONFIG_JFFS2_PART_SIZE		0x00400000
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=PPChameleon-0,nand0=ppchameleonevb-nand"
*/

/* 256 kB U-boot image */
/*
#define MTDPARTS_DEFAULT	"mtdparts=PPChameleon-0:1m(kernel1),1m(kernel2)," \
					"1792k(user),256k(u-boot);" \
				"ppchameleonevb-nand:-(nand)"
*/

/* 320 kB U-boot image */
/*
#define MTDPARTS_DEFAULT	"mtdparts=PPChameleon-0:1m(kernel1),1m(kernel2)," \
					"1728k(user),320k(u-boot);" \
				"ppchameleonevb-nand:-(nand)"
*/

#endif	/* __CONFIG_H */
