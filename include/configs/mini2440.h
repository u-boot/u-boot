/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2009-2010
 * Michel Pollet <buserror@gmail.com>
 *
 * (C) Copyright 2012
 * Gabriel Huau <contact@huau-gabriel.fr>
 *
 * Configuation settings for the MINI2440 board.
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

#define CONFIG_SYS_TEXT_BASE 0x0
#define CONFIG_S3C2440_GPIO

/*
 * High Level Configuration Options
 */
#define CONFIG_ARM920T			/* This is an ARM920T Core	*/
#define CONFIG_S3C24X0			/* in a SAMSUNG S3C24X0 SoC */
#define CONFIG_S3C2440			/* in a SAMSUNG S3C2440 SoC */
#define CONFIG_MINI2440			/* on a MIN2440 Board       */

#define MACH_TYPE_MINI2440	1999
#define CONFIG_MACH_TYPE	MACH_TYPE_MINI2440

/*
 * We don't use lowlevel_init
 */
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

/*
 * input clock of PLL
 */
/* MINI2440 has 12.0000MHz input clock */
#define CONFIG_SYS_CLK_FREQ	12000000

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 2048*1024)

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_DM9000
#define CONFIG_DRIVER_DM9000_NO_EEPROM
#define CONFIG_DM9000_BASE				0x20000300
#define DM9000_IO		CONFIG_DM9000_BASE
#define DM9000_DATA		(CONFIG_DM9000_BASE+4)

/*
 * select serial console configuration
 */
#define CONFIG_S3C24X0_SERIAL
#define CONFIG_SERIAL1

/*
 * allow to overwrite serial and ethaddr
 */
#define CONFIG_ENV_OVERWRITE

/*
 * Command definition
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES

/*
 * Miscellaneous configurable options
 */
#define CONFIG_LONGHELP
#define CONFIG_SYS_PROMPT	"MINI2440 => "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	32
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x30000000
#define CONFIG_SYS_MEMTEST_END		0x34000000	/* 64MB in DRAM	*/

/* default load address	*/
#define CONFIG_SYS_LOAD_ADDR		0x32000000

/* boot parameters address */
#define CONFIG_BOOT_PARAM_ADDR		0x30000100

/*
 * the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need
 * it to wrap 100 times (total 1562500) to get 1 sec.
 */
#define CONFIG_SYS_HZ			1562500

/*
 * valid baudrates
 */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE		115200

/*
 * Stack sizes
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(8*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS        1          /* we have 1 bank of DRAM */
#define PHYS_SDRAM_SIZE             (64*1024*1024) /* 64MB of DRAM */
#define CONFIG_SYS_SDRAM_BASE       0x30000000
#define CONFIG_SYS_FLASH_BASE		0x0

/*
 * Stack should be on the SRAM because
 * DRAM is not init
 */
#define CONFIG_SYS_INIT_SP_ADDR		(0x40001000 - GENERATED_GBL_DATA_SIZE)

/*
 * NOR FLASH organization
 * Now uses the standard CFI interface
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_MONITOR_BASE		0x0
/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
/* 512 * 4096 sectors, or 32 * 64k blocks */
#define CONFIG_SYS_MAX_FLASH_SECT	512
#define CONFIG_FLASH_SHOW_PROGRESS  1

/*
 * Config for NOR flash
 */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_MY_ENV_OFFSET	0x40000
/* addr of environment */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_MY_ENV_OFFSET)
/* 16k Total Size of Environment Sector */
#define CONFIG_ENV_SIZE		0x4000

/* ATAG configuration */
#define CONFIG_INITRD_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE

#endif	/* __CONFIG_H */
