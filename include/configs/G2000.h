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

#define CONFIG_405EP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_G2000		1	/* ...on a PLU405 board		*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

#define CONFIG_SYS_CLK_FREQ     33333333 /* external frequency to pll   */

#if 0 /* test-only */
#define CONFIG_BAUDRATE		115200
#else
#define CONFIG_BAUDRATE		9600
#endif

#define CONFIG_PREBOOT

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off\0"				\
	"addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"panic=1\0"						\
	"flash_nfs=run nfsargs addip addmisc;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addmisc;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};"				\
		"run nfsargs addip addmisc;bootm\0"			\
	"rootpath=/opt/eldk/ppc_4xx\0"					\
	"bootfile=/tftpboot/g2000/pImage\0"				\
	"kernel_addr=ff800000\0"				        \
	"ramdisk_addr=ff900000\0"				        \
	"pciconfighost=yes\0"				                \
	""
#define CONFIG_BOOTCOMMAND	"run net_nfs"

#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_NET_MULTI	1

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_PHY1_ADDR	1	/* PHY address			*/

#if 0 /* test-only */
#define CONFIG_PHY_CLK_FREQ	EMAC_STACR_CLK_66MHZ /* 66 MHz OPB clock*/
#endif

#define CONFIG_COMMANDS	      ( CONFIG_CMD_DFL	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_PCI	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_ELF	| \
				CFG_CMD_DATE	| \
				CFG_CMD_I2C	| \
				CFG_CMD_MII	| \
				CFG_CMD_PING	| \
				CFG_CMD_BSP	| \
				CFG_CMD_EEPROM	)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#if 0 /* test-only */
#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#undef	CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_DEVICE_NULLDEV	1	/* include nulldev device	*/

#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#undef	CFG_EXT_SERIAL_CLOCK	       /* no external serial clock used */
#define CFG_IGNORE_405_UART_ERRATA_59	/* ignore ppc405gp errata #59	*/
#define CFG_BASE_BAUD	    691200
#undef	CONFIG_UART1_CONSOLE		/* define for uart1 as console	*/

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CFG_LOAD_ADDR	0x100000	/* default load address */
#define CFG_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

#define CFG_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

/*----------------------------------------------------------------------------*/
/* adding Ethernet setting:  FTS OUI 00:11:0B */
/*----------------------------------------------------------------------------*/
#define CONFIG_ETHADDR          00:11:0B:00:00:01
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR         00:11:0B:00:00:02
#define CONFIG_IPADDR		10.48.8.178
#define CONFIG_IP1ADDR		10.48.8.188
#define CONFIG_NETMASK		255.255.255.128
#define CONFIG_SERVERIP		10.48.8.138

/*-----------------------------------------------------------------------
 * RTC stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_RTC_DS1337
#define CFG_I2C_RTC_ADDR	0x68

#if 0 /* test-only */
/*-----------------------------------------------------------------------
 * NAND-FLASH stuff
 *-----------------------------------------------------------------------
 */
#define CFG_MAX_NAND_DEVICE	1	/* Max number of NAND devices		*/
#define SECTORSIZE 512

#define ADDR_COLUMN 1
#define ADDR_PAGE 2
#define ADDR_COLUMN_PAGE 3

#define NAND_ChipID_UNKNOWN	0x00
#define NAND_MAX_FLOORS 1
#define NAND_MAX_CHIPS 1

#define CFG_NAND_CE  (0x80000000 >> 1)	/* our CE is GPIO1 */
#define CFG_NAND_CLE (0x80000000 >> 2)	/* our CLE is GPIO2 */
#define CFG_NAND_ALE (0x80000000 >> 3)	/* our ALE is GPIO3 */
#define CFG_NAND_RDY (0x80000000 >> 4)	/* our RDY is GPIO4 */

#define NAND_DISABLE_CE(nand) do { out32(GPIO0_OR, in32(GPIO0_OR) | CFG_NAND_CE);} while(0)
#define NAND_ENABLE_CE(nand) do { out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_NAND_CE);} while(0)
#define NAND_CTL_CLRALE(nandptr) do { out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_NAND_ALE);} while(0)
#define NAND_CTL_SETALE(nandptr) do { out32(GPIO0_OR, in32(GPIO0_OR) | CFG_NAND_ALE);} while(0)
#define NAND_CTL_CLRCLE(nandptr) do { out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_NAND_CLE);} while(0)
#define NAND_CTL_SETCLE(nandptr) do { out32(GPIO0_OR, in32(GPIO0_OR) | CFG_NAND_CLE);} while(0)
#define NAND_WAIT_READY(nand) while (!(in32(GPIO0_IR) & CFG_NAND_RDY))

#define WRITE_NAND_COMMAND(d, adr) do{ *(volatile __u8 *)((unsigned long)adr) = (__u8)(d); } while(0)
#define WRITE_NAND_ADDRESS(d, adr) do{ *(volatile __u8 *)((unsigned long)adr) = (__u8)(d); } while(0)
#define WRITE_NAND(d, adr) do{ *(volatile __u8 *)((unsigned long)adr) = (__u8)d; } while(0)
#define READ_NAND(adr) ((volatile unsigned char)(*(volatile __u8 *)(unsigned long)adr))
#endif

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0              /* configure as pci adapter     */
#define PCI_HOST_FORCE  1               /* configure as pci host        */
#define PCI_HOST_AUTO   2               /* detected via arbiter enable  */

#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_HOST	PCI_HOST_HOST   /* select pci host function     */
#define CONFIG_PCI_PNP			/* do pci plug-and-play         */
					/* resource configuration       */

#define CONFIG_PCI_SCAN_SHOW            /* print pci devices @ startup  */

#define CONFIG_PCI_CONFIG_HOST_BRIDGE 1 /* don't skip host bridge config*/

#define CFG_PCI_SUBSYS_VENDORID 0x12FE  /* PCI Vendor ID: esd gmbh      */
#define CFG_PCI_SUBSYS_DEVICEID 0x0405  /* PCI Device ID: CPCI-405      */
#define CFG_PCI_CLASSCODE       0x0b20  /* PCI Class Code: Processor/PPC*/
#define CFG_PCI_PTM1LA  0x00000000      /* point to sdram               */
#define CFG_PCI_PTM1MS  0xfc000001      /* 64MB, enable hard-wired to 1 */
#define CFG_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CFG_PCI_PTM2LA  0xffc00000      /* point to flash               */
#define CFG_PCI_PTM2MS  0xffc00001      /* 4MB, enable                  */
#define CFG_PCI_PTM2PCI 0x04000000      /* Host: use this pci address   */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#if 0 /* APC405 */
#define CFG_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#undef CFG_FLASH_PROTECTION		/* don't use hardware protection	*/
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_BASE		0xFE000000 /* test-only...*/
#define CFG_FLASH_INCREMENT	0x01000000 /* test-only */
#else /* G2000 */
#define CFG_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#undef CFG_FLASH_PROTECTION		/* don't use hardware protection	*/
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_BASE		0xFF800000 /* test-only...*/
#define CFG_FLASH_INCREMENT	0x01000000 /* test-only */
#endif

#define CFG_FLASH_EMPTY_INFO            /* print 'E' for empty sector on flinfo */

#define CFG_JFFS2_FIRST_BANK    0           /* use for JFFS2 */
#define CFG_JFFS2_NUM_BANKS     1           /* ! second bank contains u-boot    */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MONITOR_BASE	0xFFFC0000
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc()	*/

/*-----------------------------------------------------------------------
 * Environment Variable setup
 */
#if 1 /* test-only */
#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x100	/* environment starts at the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x700	/* 2048 bytes may be used for env vars*/
				   /* total size of a CAT24WC16 is 2048 bytes */

#else	/* DEFAULT: environment in flash, using redundand flash sectors */

#define CFG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */
#define CFG_ENV_ADDR		0xFFFA0000 /* environment starts before u-boot */
#define CFG_ENV_SECT_SIZE	0x20000	/* 128k bytes may be used for env vars*/

#endif

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC16) for environment
 */
#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT24WC08		*/
/* CAT24WC08/16... */
#define CFG_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CFG_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has	*/
					/* 16 byte page write mode using*/
					/* last 4 bits of the address	*/
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */
#define CFG_EEPROM_PAGE_WRITE_ENABLE

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384	/* For AMCC 405 CPUs, older 405 ppc's	*/
					/* have only 8kB, 16kB is save here	*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Intel Strata Flash) initialization                            */
#define CFG_EBC_PB0AP   0x92015480
#define CFG_EBC_PB0CR   0xFF87A000          /* BAS=0xFF8,BS=08MB,BU=R/W,BW=16bit*/

/* Memory Bank 1 ( Power TAU) initialization               */
/* #define CFG_EBC_PB1AP           0x04041000 */
/* #define CFG_EBC_PB1CR           0xF0018000   */  /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit  */
#define CFG_EBC_PB1AP           0x00000000
#define CFG_EBC_PB1CR           0x00000000

/* Memory Bank 2 (Intel Flash) initialization                 */
#define CFG_EBC_PB2AP           0x00000000
#define CFG_EBC_PB2CR           0x00000000

/* Memory Bank 3 (NAND) initialization                        */
#define CFG_EBC_PB3AP           0x92015480
#define CFG_EBC_PB3CR           0xF40B8000  /*addr 0xF40, BS=32M,BU=R/W, BW=8bit */

/* Memory Bank 4 (FPGA regs) initialization                                     */
#define CFG_EBC_PB4AP           0x00000000
#define CFG_EBC_PB4CR           0x00000000  /* leave it blank  */

#define CFG_NAND_BASE   0xF4000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CFG_TEMP_STACK_OCM	  1

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/

#define CFG_GBL_DATA_SIZE      128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Definitions for GPIO setup (PPC405EP specific)
 *
 * GPIO0[0]     - External Bus Controller BLAST output
 * GPIO0[1-9]   - Instruction trace outputs
 * GPIO0[10-13] - External Bus Controller CS_1 - CS_4 outputs
 * GPIO0[14-16] - External Bus Controller ABUS3-ABUS5 outputs
 * GPIO0[17-23] - External Interrupts IRQ0 - IRQ6 inputs
 * GPIO0[24-27] - UART0 control signal inputs/outputs
 * GPIO0[28-29] - UART1 data signal input/output
 * GPIO0[30-31] - EMAC0 and EMAC1 reject packet inputs
 *
 * following GPIO setting changed for G20000, 080304
 */
#define CFG_GPIO0_OSRH          0x40005555
#define CFG_GPIO0_OSRL          0x40000110
#define CFG_GPIO0_ISR1H         0x00000000
#define CFG_GPIO0_ISR1L         0x15555445
#define CFG_GPIO0_TSRH          0x00000000
#define CFG_GPIO0_TSRL          0x00000000
#define CFG_GPIO0_TCR           0xF7FF8014

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

/*
 * Default speed selection (cpu_plb_opb_ebc) in mhz.
 * This value will be set if iic boot eprom is disabled.
 */
#if 1
#define PLLMR0_DEFAULT	 PLLMR0_266_66_33_33
#define PLLMR1_DEFAULT	 PLLMR1_266_66_33_33
#endif
#if 0
#define PLLMR0_DEFAULT	 PLLMR0_266_133_66_33
#define PLLMR1_DEFAULT	 PLLMR1_266_133_66_33
#endif
#if 0
#define PLLMR0_DEFAULT	 PLLMR0_200_100_50_33
#define PLLMR1_DEFAULT	 PLLMR1_200_100_50_33
#endif
#if 0
#define PLLMR0_DEFAULT	 PLLMR0_133_66_66_33
#define PLLMR1_DEFAULT	 PLLMR1_133_66_66_33
#endif

#endif	/* __CONFIG_H */
