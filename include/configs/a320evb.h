/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * Configuation settings for the Faraday A320 board.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/a320.h>

/*
 * mach-type definition
 */
#define MACH_TYPE_FARADAY	758
#define CONFIG_MACH_TYPE	MACH_TYPE_FARADAY

/*
 * Linux kernel tagged list
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS

/*
 * CPU and Board Configuration Options
 */
#undef CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Power Management Unit
 */
#define CONFIG_FTPMU010_POWER

/*
 * Timer
 */
#define CONFIG_SYS_HZ		1000	/* timer ticks per second */

/*
 * Real Time Clock
 */
#define CONFIG_RTC_FTRTC010

/*
 * Serial console configuration
 */

/* FTUART is a high speed NS 16C550A compatible UART */
#define CONFIG_BAUDRATE			38400
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_COM1		0x98200000
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_CLK		18432000

/*
 * Ethernet
 */
#define CONFIG_FTMAC100

#define CONFIG_BOOTDELAY	3

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PING

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_PROMPT	"A320 # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* max number of command args */
#define CONFIG_SYS_MAXARGS	16

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)

/*
 * SDRAM controller configuration
 */
#define CONFIG_SYS_FTSDMC020_TP0	(FTSDMC020_TP0_TRAS(2) |	\
					 FTSDMC020_TP0_TRP(1)  |	\
					 FTSDMC020_TP0_TRCD(1) |	\
					 FTSDMC020_TP0_TRF(3)  |	\
					 FTSDMC020_TP0_TWR(1)  |	\
					 FTSDMC020_TP0_TCL(2))

#define CONFIG_SYS_FTSDMC020_TP1	(FTSDMC020_TP1_INI_PREC(4) |	\
					 FTSDMC020_TP1_INI_REFT(8) |	\
					 FTSDMC020_TP1_REF_INTV(0x180))

#define CONFIG_SYS_FTSDMC020_BANK0_BSR	(FTSDMC020_BANK_ENABLE   |	\
					 FTSDMC020_BANK_DDW_X16  |	\
					 FTSDMC020_BANK_DSZ_256M |	\
					 FTSDMC020_BANK_MBW_32   |	\
					 FTSDMC020_BANK_SIZE_64M)

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x10000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000	/* 64 MB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
					GENERATED_GBL_DATA_SIZE)

/*
 * Load address and memory test area should agree with
 * board/faraday/a320/config.mk. Be careful not to overwrite U-boot itself.
 */
#define CONFIG_SYS_LOAD_ADDR		(PHYS_SDRAM_1 + 0x2000000)

/* memtest works on 63 MB in DRAM */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + 0x3F00000)

#define CONFIG_SYS_TEXT_BASE		0

/*
 * Static memory controller configuration
 */

#define CONFIG_FTSMC020
#include <faraday/ftsmc020.h>

#define FTSMC020_BANK0_CONFIG	(FTSMC020_BANK_ENABLE             |	\
				 FTSMC020_BANK_BASE(PHYS_FLASH_1) |	\
				 FTSMC020_BANK_SIZE_1M            |	\
				 FTSMC020_BANK_MBW_8)

#define FTSMC020_BANK0_TIMING	(FTSMC020_TPR_RBE      |	\
				 FTSMC020_TPR_AST(3)   |	\
				 FTSMC020_TPR_CTW(3)   |	\
				 FTSMC020_TPR_ATI(0xf) |	\
				 FTSMC020_TPR_AT2(3)   |	\
				 FTSMC020_TPR_WTC(3)   |	\
				 FTSMC020_TPR_AHT(3)   |	\
				 FTSMC020_TPR_TRNA(0xf))

#define FTSMC020_BANK1_CONFIG	(FTSMC020_BANK_ENABLE             |	\
				 FTSMC020_BANK_BASE(PHYS_FLASH_2) |	\
				 FTSMC020_BANK_SIZE_32M           |	\
				 FTSMC020_BANK_MBW_32)

#define FTSMC020_BANK1_TIMING	(FTSMC020_TPR_AST(3)   |	\
				 FTSMC020_TPR_CTW(3)   |	\
				 FTSMC020_TPR_ATI(0xf) |	\
				 FTSMC020_TPR_AT2(3)   |	\
				 FTSMC020_TPR_WTC(3)   |	\
				 FTSMC020_TPR_AHT(3)   |	\
				 FTSMC020_TPR_TRNA(0xf))

#define CONFIG_SYS_FTSMC020_CONFIGS	{			\
	{ FTSMC020_BANK0_CONFIG, FTSMC020_BANK0_TIMING, },	\
	{ FTSMC020_BANK1_CONFIG, FTSMC020_BANK1_TIMING, },	\
}

/*
 * FLASH and environment organization
 */

/* use CFI framework */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER

/* support JEDEC */
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_FLASH_LEGACY_512Kx8

#define PHYS_FLASH_1			0x00000000
#define PHYS_FLASH_2			0x00400000
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1, PHYS_FLASH_2, }

#define CONFIG_SYS_MONITOR_BASE		PHYS_FLASH_1

/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS	2

/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	512

#undef CONFIG_SYS_FLASH_EMPTY_INFO

/* environments */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR			(PHYS_FLASH_1 + 0x60000)
#define CONFIG_ENV_SIZE			0x20000

#endif	/* __CONFIG_H */
