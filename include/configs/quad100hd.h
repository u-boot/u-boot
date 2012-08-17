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

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000

#define CONFIG_SYS_CLK_FREQ     33333333 /* external frequency to pll   */

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */

#define PLLMR0_DEFAULT		PLLMR0_266_133_66 /* no PCI */
#define PLLMR1_DEFAULT		PLLMR1_266_133_66 /* no PCI */

/* the environment is in the EEPROM by default */
#define CONFIG_ENV_IS_IN_EEPROM
#undef CONFIG_ENV_IS_IN_FLASH

#define CONFIG_PPC4xx_EMAC
#define CONFIG_HAS_ETH1		1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0x01	/* PHY address			*/
#define CONFIG_SYS_RX_ETH_BUFFER	16	/* Number of ethernet rx buffers & descriptors */
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

/* FIX! SDRAM timings used in datasheet */
#define CONFIG_SYS_SDRAM_CL            3       /* CAS latency */
#define CONFIG_SYS_SDRAM_tRP           20      /* PRECHARGE command period */
#define CONFIG_SYS_SDRAM_tRC           66      /* ACTIVE-to-ACTIVE command period */
#define CONFIG_SYS_SDRAM_tRCD          20      /* ACTIVE-to-READ delay */
#define CONFIG_SYS_SDRAM_tRFC          66      /* Auto refresh period */

/*
 * JFFS2
 */
#define CONFIG_SYS_JFFS2_FIRST_BANK    0
#ifdef  CONFIG_SYS_KERNEL_IN_JFFS2
#define CONFIG_SYS_JFFS2_FIRST_SECTOR  0   /* JFFS starts at block 0 */
#else /* kernel not in JFFS */
#define CONFIG_SYS_JFFS2_FIRST_SECTOR  8   /* block 0-7 is kernel (1MB = 8 sectors) */
#endif
#define CONFIG_SYS_JFFS2_NUM_BANKS     1

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#undef	CONFIG_SYS_EXT_SERIAL_CLOCK			/* external serial clock */
#define CONFIG_SYS_BASE_BAUD		691200
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

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
#define CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	        16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	        CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000  /* default load address	*/
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_info (bd_t) */

#define CONFIG_SYS_HZ		        1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

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
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50		/* base address */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2		/* bytes of address */

#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* 8 byte write page size */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */
#define CONFIG_SYS_EEPROM_SIZE			0x2000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFC00000
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_TEXT_BASE)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible	*/
#define	CONFIG_FLASH_CFI_DRIVER

#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max number of sectors on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */
#define CONFIG_SYS_FLASH_INCREMENT      0       /* there is only one bank         */

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000	/* size of one complete sector	*/
/* the environment is located before u-boot */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_TEXT_BASE - CONFIG_ENV_SECT_SIZE)

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SECT_SIZE)
#endif

#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_SIZE		0x400		/* Size of Environment vars */
#define CONFIG_ENV_OFFSET		0x00000000
#define CONFIG_SYS_ENABLE_CRC_16	1       /* Intrinsyc formatting used crc16 */
#endif

/* partly from PPCBoot */
/* NAND */
#define CONFIG_NAND
#ifdef CONFIG_NAND
#define CONFIG_SYS_NAND_BASE   0x60000000
#define CONFIG_SYS_NAND_CS	10   /* our CS is GPIO10 */
#define CONFIG_SYS_NAND_RDY	23   /* our RDY is GPIO23 */
#define CONFIG_SYS_NAND_CE	24   /* our CE is GPIO24  */
#define CONFIG_SYS_NAND_CLE	31   /* our CLE is GPIO31 */
#define CONFIG_SYS_NAND_ALE	30   /* our ALE is GPIO30 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1

#endif

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory (OCM) for temperary stack until sdram is tested */
/* see ./arch/powerpc/cpu/ppc4xx/start.S */
#define CONFIG_SYS_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of OCM		*/
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM	*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET      CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 * Taken from PPCBoot board/icecube/icecube.h
 */

/* see ./arch/powerpc/cpu/ppc4xx/cpu_init.c ./cpu/ppc4xx/ndfc.c */
#define CONFIG_SYS_EBC_PB0AP		0x04002480
/* AMD NOR flash - this corresponds to FLASH_BASE so may be correct */
#define CONFIG_SYS_EBC_PB0CR		0xFFC5A000
#define CONFIG_SYS_EBC_PB1AP           0x04005480
#define CONFIG_SYS_EBC_PB1CR           0x60018000
#define CONFIG_SYS_EBC_PB2AP           0x00000000
#define CONFIG_SYS_EBC_PB2CR           0x00000000
#define CONFIG_SYS_EBC_PB3AP           0x00000000
#define CONFIG_SYS_EBC_PB3CR           0x00000000
#define CONFIG_SYS_EBC_PB4AP           0x00000000
#define CONFIG_SYS_EBC_PB4CR           0x00000000

/*-----------------------------------------------------------------------
 * Definitions for GPIO setup (PPC405EP specific)
 *
 * Taken in part from PPCBoot board/icecube/icecube.h
 */
/* see ./arch/powerpc/cpu/ppc4xx/cpu_init.c ./cpu/ppc4xx/start.S */
#define CONFIG_SYS_GPIO0_OSRL		0x55555550
#define CONFIG_SYS_GPIO0_OSRH		0x00000110
#define CONFIG_SYS_GPIO0_ISR1L		0x00000000
#define CONFIG_SYS_GPIO0_ISR1H		0x15555445
#define CONFIG_SYS_GPIO0_TSRL		0x00000000
#define CONFIG_SYS_GPIO0_TSRH		0x00000000
#define CONFIG_SYS_GPIO0_TCR		0xFFFF8097
#define CONFIG_SYS_GPIO0_ODR		0x00000000

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
