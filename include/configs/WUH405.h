/*
 * (C) Copyright 2004
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

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_IDENT_STRING     " $Name:  $"

#define CONFIG_405EP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_WUH405		1	/* ...on a WUH405 board		*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

#define CONFIG_SYS_CLK_FREQ	33333300 /* external frequency to pll	*/

#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#undef	CONFIG_BOOTARGS
#undef  CONFIG_BOOTCOMMAND

#define CONFIG_PREBOOT                  /* enable preboot variable      */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_LXT971_NO_SLEEP  1       /* disable sleep mode in LXT971 */

#define CONFIG_PHY_CLK_FREQ	EMAC_STACR_CLK_66MHZ /* 66 MHz OPB clock*/


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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ELF
#define CONFIG_CMD_NAND
#define CONFIG_CMD_DATE
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_EEPROM


#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_RTC_MC146818		/* DS1685 is MC146818 compatible*/
#define CONFIG_SYS_RTC_REG_BASE_ADDR	 0xF0000500 /* RTC Base Address		*/

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/

#undef	CONFIG_SYS_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

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

#define CONFIG_CONS_INDEX	2	/* Use UART1			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef	CONFIG_SYS_EXT_SERIAL_CLOCK	       /* no external serial clock used */
#define CONFIG_SYS_BASE_BAUD	    691200

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

#define CONFIG_SYS_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

/*-----------------------------------------------------------------------
 * NAND-FLASH stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1         /* Max number of NAND devices */
#define NAND_BIG_DELAY_US	25

#define CONFIG_SYS_NAND_CE             (0x80000000 >> 1)   /* our CE is GPIO1  */
#define CONFIG_SYS_NAND_RDY            (0x80000000 >> 4)   /* our RDY is GPIO4 */
#define CONFIG_SYS_NAND_CLE            (0x80000000 >> 2)   /* our CLE is GPIO2 */
#define CONFIG_SYS_NAND_ALE            (0x80000000 >> 3)   /* our ALE is GPIO3 */

#define CONFIG_SYS_NAND_SKIP_BAD_DOT_I      1  /* ".i" read skips bad blocks   */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure as pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_HOST	/* select pci host function	*/
#undef	CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#undef	CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x12FE	/* PCI Vendor ID: esd gmbh	*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0405	/* PCI Device ID: CPCI-405	*/
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
#define CONFIG_SYS_FLASH_BASE		0xFFFC0000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
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

#if 0 /* test-only */
#define CONFIG_SYS_JFFS2_FIRST_BANK	0	    /* use for JFFS2 */
#define CONFIG_SYS_JFFS2_NUM_BANKS	1	    /* ! second bank contains U-Boot */
#endif

/*-----------------------------------------------------------------------
 * Environment Variable setup
 */
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x100	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x700	/* 2048 bytes may be used for env vars*/
				   /* total size of a CAT24WC16 is 2048 bytes */

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
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
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
/*#define CONFIG_SYS_EBC_PB0AP		  0x08055880  /XXX* TWT=16,CSN=1,OEN=1,WBN=1,WBF=1,TH=4,SOR=1 */
#define CONFIG_SYS_EBC_PB0CR		0xFFC5A000  /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (Flash Bank 1, NAND-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB1AP		0x92015480
#define CONFIG_SYS_EBC_PB1CR		0xF4018000  /* BAS=0xF40,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 2 (8 Bit Peripheral: CAN, UART, RTC) initialization		*/
#define CONFIG_SYS_EBC_PB2AP		0x010053C0  /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB2CR		0xF0018000  /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (16 Bit Peripheral: FPGA internal, dig. IO) initialization	*/
#define CONFIG_SYS_EBC_PB3AP		0x010053C0  /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB3CR		0xF011A000  /* BAS=0xF01,BS=1MB,BU=R/W,BW=16bit */

#define CAN_BA		0xF0000000	    /* CAN Base Address			*/
#define DUART0_BA	0xF0000400	    /* DUART Base Address		*/
#define DUART1_BA	0xF0000408	    /* DUART Base Address		*/
#define DUART2_BA	0xF0000410	    /* DUART Base Address		*/
#define DUART3_BA	0xF0000418	    /* DUART Base Address		*/
#define RTC_BA		0xF0000500	    /* RTC Base Address			*/
#define CONFIG_SYS_NAND_BASE	0xF4000000

/*-----------------------------------------------------------------------
 * FPGA stuff
 */
#define CONFIG_SYS_FPGA_SPARTAN2	1	    /* using Xilinx Spartan 2 now    */
#define CONFIG_SYS_FPGA_MAX_SIZE	128*1024    /* 128kByte is enough for XC2S50E*/

/* FPGA program pin configuration */
#define CONFIG_SYS_FPGA_PRG		0x04000000  /* FPGA program pin (ppc output) */
#define CONFIG_SYS_FPGA_CLK		0x02000000  /* FPGA clk pin (ppc output)     */
#define CONFIG_SYS_FPGA_DATA		0x01000000  /* FPGA data pin (ppc output)    */
#define CONFIG_SYS_FPGA_INIT		0x00010000  /* FPGA init pin (ppc input)     */
#define CONFIG_SYS_FPGA_DONE		0x00008000  /* FPGA done pin (ppc input)     */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	  1

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
 * GPIO0[30-31] - EMAC0 and EMAC1 reject packet inputs
 */
#define CONFIG_SYS_GPIO0_OSRL		0x40000550
#define CONFIG_SYS_GPIO0_OSRH		0x00000110
#define CONFIG_SYS_GPIO0_ISR1L		0x00000000
#define CONFIG_SYS_GPIO0_ISR1H		0x15555445
#define CONFIG_SYS_GPIO0_TSRL		0x00000000
#define CONFIG_SYS_GPIO0_TSRH		0x00000000
#define CONFIG_SYS_GPIO0_TCR		0xF7FE0014

#define CONFIG_SYS_DUART_RST		(0x80000000 >> 14)

/*
 * Default speed selection (cpu_plb_opb_ebc) in mhz.
 * This value will be set if iic boot eprom is disabled.
 */
#if 0
#define PLLMR0_DEFAULT	 PLLMR0_266_133_66_33
#define PLLMR1_DEFAULT	 PLLMR1_266_133_66_33
#endif
#if 1
#define PLLMR0_DEFAULT	 PLLMR0_200_100_50_33
#define PLLMR1_DEFAULT	 PLLMR1_200_100_50_33
#endif
#if 0
#define PLLMR0_DEFAULT	 PLLMR0_133_66_66_33
#define PLLMR1_DEFAULT	 PLLMR1_133_66_66_33
#endif

#endif	/* __CONFIG_H */
