/*
 * (C) Copyright 2004
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Kshitij Gupta <kshitij@ti.com>
 *
 * Configuration settings for the LogicPD i.MX31 Litekit board.
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

#include <asm/arch/mx31-regs.h>

 /* High Level Configuration Options */
#define CONFIG_ARM1136		1    /* This is an arm1136 CPU core */
#define CONFIG_MX31		1    /* in a mx31 */
#define CONFIG_MX31_HCLK_FREQ	26000000
#define CONFIG_MX31_CLK32	32000

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Temporarily disabled */
#if 0
#define CONFIG_OF_LIBFDT		1
#define CONFIG_FIT			1
#define CONFIG_FIT_VERBOSE		1
#endif

#define CONFIG_CMDLINE_TAG		1    /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128 * 1024)
#define CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

#define CONFIG_MX31_UART	1
#define CFG_MX31_UART1		1

#define CONFIG_HARD_SPI		1
#define CONFIG_MXC_SPI		1
#define CONFIG_DEFAULT_SPI_BUS	1
#define CONFIG_DEFAULT_SPI_MODE	(SPI_MODE_2 | SPI_CS_HIGH)

#define CONFIG_RTC_MC13783	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SPI
#define CONFIG_CMD_DATE

#define CONFIG_BOOTDELAY	3

#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.23.168
#define CONFIG_SERVERIP		192.168.23.2

#define	CONFIG_EXTRA_ENV_SETTINGS											\
	"bootargs_base=setenv bootargs console=ttySMX0,115200\0"							\
	"bootargs_nfs=setenv bootargs $(bootargs) root=/dev/nfs ip=dhcp nfsroot=$(serverip):$(nfsrootfs),v3,tcp\0"	\
	"bootcmd=run bootcmd_net\0"											\
	"bootcmd_net=run bootargs_base bootargs_mtd bootargs_nfs; tftpboot 0x80000000 uImage-mx31; bootm\0"		\
	"prg_uboot=tftpboot 0x80000000 u-boot-imx31_litekit.bin; protect off all; erase 0xa00d0000 0xa01effff; cp.b 0x80000000 0xa00d0000 $(filesize)\0"


#define CONFIG_DRIVER_SMC911X		1
#define CONFIG_DRIVER_SMC911X_BASE	(CS4_BASE + 0x00020000)
#define CONFIG_DRIVER_SMC911X_32_BIT	1

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		/* undef to save memory */
#define CFG_PROMPT		"uboot> "
#define CFG_CBSIZE		256  /* Console I/O Buffer Size */
/* Print Buffer Size */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS		16          /* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE  /* Boot Argument Buffer Size */

#define CFG_MEMTEST_START	0  /* memtest works on */
#define CFG_MEMTEST_END		0x10000

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0 /* default load address */

#define CFG_HZ			32000

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
#define CFG_FLASH_BASE		CS0_BASE
#define CFG_MAX_FLASH_BANKS	1           /* max number of memory banks */
#define CFG_MAX_FLASH_SECT	128	     /* max number of sectors on one chip */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE /* Monitor at beginning of flash */

#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x001f0000)
#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SECT_SIZE	(64 * 1024)
#define CFG_ENV_SIZE		(64 * 1024)

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
#define CFG_FLASH_CFI		1	/* Flash memory is CFI compliant */
#define CFG_FLASH_CFI_DRIVER	1	/* Use drivers/cfi_flash.c */
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* Use buffered writes (~10x faster) */
#define CFG_FLASH_PROTECTION	1	/* Use hardware sector protection */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(100*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(100*CFG_HZ) /* Timeout for Flash Write */

/*
 * JFFS2 partitions
 */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV	"nor0"

#endif /* __CONFIG_H */
