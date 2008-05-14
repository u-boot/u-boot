/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
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
 * quad100hd.h - configuration for Quad100hd board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_QUAD100HD	1		/* Board is Quad100hd	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_405EP		1		/* Specifc 405EP support*/

#define CONFIG_SYS_CLK_FREQ     33333333 /* external frequency to pll   */

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */

#define PLLMR0_DEFAULT		PLLMR0_266_133_66 /* no PCI */
#define PLLMR1_DEFAULT		PLLMR1_266_133_66 /* no PCI */

/* the environment is in the EEPROM by default */
#define CFG_ENV_IS_IN_EEPROM
#undef CFG_ENV_IS_IN_FLASH

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0x01	/* PHY address			*/
#define CFG_RX_ETH_BUFFER	16	/* Number of ethernet rx buffers & descriptors */
#define CONFIG_PHY_RESET	1
#define CONFIG_PHY_RESET_DELAY	300	/* PHY RESET recovery delay	*/

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_ASKENV
#undef CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#undef CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#undef CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#undef CONFIG_CMD_IRQ
#define CONFIG_CMD_JFFS2
#undef CONFIG_CMD_LOG
#undef CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#undef CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*-----------------------------------------------------------------------
 * SDRAM
 *----------------------------------------------------------------------*/
/*
 * SDRAM configuration (please see cpu/ppc/sdram.[ch])
 */
#define CONFIG_SDRAM_BANK0  1
#define CFG_SDRAM_SIZE      0x02000000      /* 32 MB */

/* FIX! SDRAM timings used in datasheet */
#define CFG_SDRAM_CL            3       /* CAS latency */
#define CFG_SDRAM_tRP           20      /* PRECHARGE command period */
#define CFG_SDRAM_tRC           66      /* ACTIVE-to-ACTIVE command period */
#define CFG_SDRAM_tRCD          20      /* ACTIVE-to-READ delay */
#define CFG_SDRAM_tRFC          66      /* Auto refresh period */

/*
 * JFFS2
 */
#define CFG_JFFS2_FIRST_BANK    0
#ifdef  CFG_KERNEL_IN_JFFS2
#define CFG_JFFS2_FIRST_SECTOR  0   /* JFFS starts at block 0 */
#else /* kernel not in JFFS */
#define CFG_JFFS2_FIRST_SECTOR  8   /* block 0-7 is kernel (1MB = 8 sectors) */
#endif
#define CFG_JFFS2_NUM_BANKS     1

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CFG_EXT_SERIAL_CLOCK			/* external serial clock */
#define CFG_BASE_BAUD		691200
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	        16	/* max number of command args	*/
#define CFG_BARGSIZE	        CFG_CBSIZE /* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000  /* default load address	*/
#define CFG_EXTBDINFO		1	/* To use extended board_info (bd_t) */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1		/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x50		/* base address */
#define CFG_I2C_EEPROM_ADDR_LEN	2		/* bytes of address */

#define CFG_EEPROM_PAGE_WRITE_BITS	5	/* 8 byte write page size */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */
#define CFG_EEPROM_SIZE			0x2000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFC00000
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/
#define CFG_MONITOR_BASE	(TEXT_BASE)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_FLASH_CFI			/* The flash is CFI compatible	*/
#define	CFG_FLASH_CFI_DRIVER

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks	*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */
#define CFG_FLASH_INCREMENT      0       /* there is only one bank         */

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000	/* size of one complete sector	*/
/* the environment is located before u-boot */
#define CFG_ENV_ADDR		(TEXT_BASE - CFG_ENV_SECT_SIZE)

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR - CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SECT_SIZE)
#endif

#ifdef CFG_ENV_IS_IN_EEPROM
#define CFG_ENV_SIZE		0x400		/* Size of Environment vars */
#define CFG_ENV_OFFSET		0x00000000
#define CFG_ENABLE_CRC_16	1       /* Intrinsyc formatting used crc16 */
#endif

/* partly from PPCBoot */
/* NAND */
#define CONFIG_NAND
#ifdef CONFIG_NAND
#define CFG_NAND_BASE   0x60000000
#define CFG_NAND_CS	10   /* our CS is GPIO10 */
#define CFG_NAND_RDY	23   /* our RDY is GPIO23 */
#define CFG_NAND_CE	24   /* our CE is GPIO24  */
#define CFG_NAND_CLE	31   /* our CLE is GPIO31 */
#define CFG_NAND_ALE	30   /* our ALE is GPIO30 */
#define NAND_MAX_CHIPS	1
#define CFG_MAX_NAND_DEVICE	1
#endif

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory (OCM) for temperary stack until sdram is tested */
/* see ./cpu/ppc4xx/start.S */
#define CFG_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of OCM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/

#define CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 * Taken from PPCBoot board/icecube/icecube.h
 */

/* see ./cpu/ppc4xx/cpu_init.c ./cpu/ppc4xx/ndfc.c */
#define CFG_EBC_PB0AP		0x04002480
/* AMD NOR flash - this corresponds to FLASH_BASE so may be correct */
#define CFG_EBC_PB0CR		0xFFC5A000
#define CFG_EBC_PB1AP           0x04005480
#define CFG_EBC_PB1CR           0x60018000
#define CFG_EBC_PB2AP           0x00000000
#define CFG_EBC_PB2CR           0x00000000
#define CFG_EBC_PB3AP           0x00000000
#define CFG_EBC_PB3CR           0x00000000
#define CFG_EBC_PB4AP           0x00000000
#define CFG_EBC_PB4CR           0x00000000

/*-----------------------------------------------------------------------
 * Definitions for GPIO setup (PPC405EP specific)
 *
 * Taken in part from PPCBoot board/icecube/icecube.h
 */
/* see ./cpu/ppc4xx/cpu_init.c ./cpu/ppc4xx/start.S */
#define CFG_GPIO0_OSRH		0x55555550
#define CFG_GPIO0_OSRL		0x00000110
#define CFG_GPIO0_ISR1H		0x00000000
#define CFG_GPIO0_ISR1L		0x15555445
#define CFG_GPIO0_TSRH		0x00000000
#define CFG_GPIO0_TSRL		0x00000000
#define CFG_GPIO0_TCR		0xFFFF8097
#define CFG_GPIO0_ODR		0x00000000

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/* ENVIRONMENT VARS */

#define CONFIG_IPADDR		192.168.1.67
#define CONFIG_SERVERIP		192.168.1.50
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_LOADADDR		300000
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1

#endif	/* __CONFIG_H */
