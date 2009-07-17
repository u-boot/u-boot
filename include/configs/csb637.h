/*
 * (C) Copyright 2005 REA Elektronik GmbH <www.rea.de>
 * Anders Larsen <alarsen@rea.de>
 *
 * Configuation settings for the Cogent CSB637 board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/* ARM asynchronous clock */
#define AT91C_MAIN_CLOCK	184320000	/* from 3.6864 MHz crystal (3686400 * 50) */
#define AT91C_MASTER_CLOCK	46080000	/* (AT91C_MAIN_CLOCK/4)	peripheral clock */

#define AT91_SLOW_CLOCK		32768	/* slow clock */

#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define CONFIG_AT91RM9200	1	/* It's an Atmel AT91RM9200 SoC	*/
#define CONFIG_CSB637		1	/* on a CSB637 board		*/
#undef  CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/
#define USE_920T_MMU		1

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_USE_MAIN_OSCILLATOR		1
/* flash */
#define CONFIG_SYS_EBI_CFGR_VAL	0x00000000
#define CONFIG_SYS_SMC_CSR0_VAL	0x00003284 /* 16bit, 2 TDF, 4 WS */

/* clocks */
#define CONFIG_SYS_PLLAR_VAL	0x2031BE01 /* 184.320000 MHz for PCK */
#define CONFIG_SYS_PLLBR_VAL	0x128A3E19 /* 47.996928 MHz (divider by 2 for USB) */
#define CONFIG_SYS_MCKR_VAL	0x00000302 /* PCK/4 = MCK Master Clock = 46.080000 MHz from PLLA */

/* sdram */
#define CONFIG_SYS_PIOC_ASR_VAL	0xFFFF0000 /* Configure PIOC as peripheral (D16/D31) */
#define CONFIG_SYS_PIOC_BSR_VAL	0x00000000
#define CONFIG_SYS_PIOC_PDR_VAL	0xFFFF0000
#define CONFIG_SYS_EBI_CSA_VAL	0x00000002 /* CS1=CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRC_CR_VAL	0x21914159 /* set up the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM	0x20000000 /* address of the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM1	0x20000080 /* address of the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM_VAL	0x00000000 /* value written to CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRC_MR_VAL	0x00000002 /* Precharge All */
#define CONFIG_SYS_SDRC_MR_VAL1	0x00000004 /* refresh */
#define CONFIG_SYS_SDRC_MR_VAL2	0x00000003 /* Load Mode Register */
#define CONFIG_SYS_SDRC_MR_VAL3	0x00000000 /* Normal Mode */
#define CONFIG_SYS_SDRC_TR_VAL	0x000002E0 /* Write refresh rate */
#else
#define CONFIG_SKIP_RELOCATE_UBOOT
#endif	/* CONFIG_SKIP_LOWLEVEL_INIT */
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_BAUDRATE 115200

#define CONFIG_SYS_AT91C_BRGR_DIVISOR	75	/* hardcode so no __divsi3 : AT91C_MASTER_CLOCK / baudrate / 16 */

/*
 * Hardware drivers
 */

/* define one of these to choose the DBGU, USART0  or USART1 as console */
#define CONFIG_AT91RM9200_USART
#define CONFIG_DBGU
#undef CONFIG_USART0
#undef CONFIG_USART1

#undef	CONFIG_HWFLOW			/* don't include RTS/CTS flow control support	*/

#undef	CONFIG_MODEM_SUPPORT		/* disable modem initialization stuff */

#define CONFIG_BOOTDELAY      3
/* #define CONFIG_ENV_OVERWRITE	1 */


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
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_PING


#define CONFIG_NR_DRAM_BANKS 1
#define PHYS_SDRAM			0x20000000
#define PHYS_SDRAM_SIZE			0x4000000  /* 64 megs */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			CONFIG_SYS_MEMTEST_START + PHYS_SDRAM_SIZE - 512*1024 - 4
#define CONFIG_SYS_ALT_MEMTEST			1
#define CONFIG_SYS_MEMTEST_SCRATCH		CONFIG_SYS_MEMTEST_START + PHYS_SDRAM_SIZE - 4

#define CONFIG_DRIVER_ETHER
#define CONFIG_NET_RETRY_COUNT		20
#undef CONFIG_AT91C_USE_RMII

#undef CONFIG_HAS_DATAFLASH
#define CONFIG_SYS_SPI_WRITE_TOUT		(5*CONFIG_SYS_HZ)
#define CONFIG_SYS_MAX_DATAFLASH_BANKS		0
#define CONFIG_SYS_MAX_DATAFLASH_PAGES		16384
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* Logical adress for CS0 */
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* Logical adress for CS3 */

/*
 * FLASH Device configuration
 */
#define PHYS_FLASH_1			0x10000000
#define PHYS_FLASH_SIZE			0x800000  /* 8 megs main flash */
#define CONFIG_SYS_FLASH_BASE			PHYS_FLASH_1
#define CONFIG_SYS_FLASH_CFI		1	/* flash is CFI conformant	*/
#define CONFIG_FLASH_CFI_DRIVER	1	/* use common cfi driver	*/
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max # of memory banks	*/
#define CONFIG_SYS_FLASH_INCREMENT	0	/* there is only one bank	*/
#define CONFIG_SYS_FLASH_PROTECTION	1	/* hardware flash protection	*/
#define CONFIG_SYS_MAX_FLASH_SECT		64

#define CONFIG_SYS_JFFS2_FIRST_BANK	0
#define CONFIG_SYS_JFFS2_FIRST_SECTOR	3
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

#undef	CONFIG_ENV_IS_IN_DATAFLASH

#ifdef CONFIG_ENV_IS_IN_DATAFLASH
#define CONFIG_ENV_OFFSET			0x20000
#define CONFIG_ENV_ADDR			(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE			0x2000  /* 0x8000 */
#else
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_ADDR			(PHYS_FLASH_1 + 0x20000)  /* after u-boot.bin */
#define CONFIG_ENV_SIZE			0x20000 /* sectors are 128K here */
#endif	/* CONFIG_ENV_IS_IN_DATAFLASH */


#define CONFIG_SYS_LOAD_ADDR		0x21000000  /* default load address */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 57600, 38400, 19200, 9600 }

#define CONFIG_SYS_PROMPT		"U-Boot> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */

#define CONFIG_SYS_HZ 1000
#define CONFIG_SYS_HZ_CLOCK AT91C_MASTER_CLOCK/2	/* AT91C_TC0_CMR is implicitly set to */
						/* AT91C_TC_TIMER_DIV1_CLOCK */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif
