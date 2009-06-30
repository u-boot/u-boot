/*
 * (C) Copyright 2008 Magnus Lilja <lilja.magnus@gmail.com>
 *
 * (C) Copyright 2004
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Kshitij Gupta <kshitij@ti.com>
 *
 * Configuration settings for the Freescale i.MX31 PDK board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_ARM1136		1	/* This is an arm1136 CPU core */
#define CONFIG_MX31		1	/* in a mx31 */
#define CONFIG_MX31_HCLK_FREQ	26000000
#define CONFIG_MX31_CLK32	32768

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/* No support for NAND boot for i.MX31 PDK yet, so we rely on some other
 * program to initialize the SDRAM.
 */
#define CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)
/* Bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART		1
#define CONFIG_SYS_MX31_UART1	1

#define CONFIG_HARD_SPI		1
#define CONFIG_MXC_SPI		1
#define CONFIG_DEFAULT_SPI_BUS	1
#define CONFIG_DEFAULT_SPI_MODE	(SPI_MODE_2 | SPI_CS_HIGH)

#define CONFIG_RTC_MC13783	1

/* MC13783 connected to CSPI2 and SS2 */
#define CONFIG_MC13783_SPI_BUS	1
#define CONFIG_MC13783_SPI_CS	2

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SPI
#define CONFIG_CMD_DATE

/*
 * Disabled due to compilation errors in cmd_bootm.c (IMLS seems to require
 * that CFG_NO_FLASH is undefined).
 */
#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY	3

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"bootargs_base=setenv bootargs console=ttymxc0,115200\0"	\
	"bootargs_nfs=setenv bootargs $(bootargs) root=/dev/nfs "	\
		"ip=dhcp nfsroot=$(serverip):$(nfsrootfs),v3,tcp\0"	\
	"bootcmd=run bootcmd_net\0"					\
	"bootcmd_net=run bootargs_base bootargs_mtd bootargs_nfs; "	\
		"tftpboot 0x81000000 uImage-mx31; bootm\0"

#define CONFIG_DRIVER_SMC911X		1
#define CONFIG_DRIVER_SMC911X_BASE	0xB6000000
#define CONFIG_DRIVER_SMC911X_32_BIT	1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_PROMPT	"uboot> "
#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
				sizeof(CONFIG_SYS_PROMPT)+16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS	16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x10000

/* default load address */
#define CONFIG_SYS_LOAD_ADDR		0x81000000

#define CONFIG_SYS_HZ			1000

#define CONFIG_CMDLINE_EDITING	1

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 * 1024) /* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/* No NOR flash present */
#define CONFIG_SYS_NO_FLASH	1

#define CONFIG_ENV_IS_NOWHERE	1

#define CONFIG_ENV_SIZE		(128 * 1024)

#endif /* __CONFIG_H */
