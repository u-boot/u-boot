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

#include <asm/arch/imx-regs.h>

 /* High Level Configuration Options */
#define CONFIG_ARM1136		1    /* This is an arm1136 CPU core */
#define CONFIG_MX31		1    /* in a mx31 */
#define CONFIG_MX31_HCLK_FREQ	26000000
#define CONFIG_MX31_CLK32	32000

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_TEXT_BASE	0xa0000000

#define CONFIG_MACH_TYPE	MACH_TYPE_MX31LITE

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
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE
#define CONFIG_MXC_GPIO

#define CONFIG_HARD_SPI		1
#define CONFIG_MXC_SPI		1
#define CONFIG_DEFAULT_SPI_BUS	1
#define CONFIG_DEFAULT_SPI_MODE	(SPI_MODE_0 | SPI_CS_HIGH)

/* PMIC Controller */
#define CONFIG_PMIC
#define CONFIG_PMIC_SPI
#define CONFIG_PMIC_FSL
#define CONFIG_FSL_PMIC_BUS	1
#define CONFIG_FSL_PMIC_CS	0
#define CONFIG_FSL_PMIC_CLK	1000000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CONFIG_FSL_PMIC_BITLEN	32
#define CONFIG_RTC_MC13XXX

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SPI
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NAND

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


#define CONFIG_SMC911X		1
#define CONFIG_SMC911X_BASE	(CS4_BASE + 0x00020000)
#define CONFIG_SMC911X_32_BIT	1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_PROMPT		"uboot> "
#define CONFIG_SYS_CBSIZE		256  /* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16          /* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE  /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0  /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x10000

#define CONFIG_SYS_LOAD_ADDR		0 /* default load address */

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
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE		IRAM_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		CS0_BASE
#define CONFIG_SYS_MAX_FLASH_BANKS	1           /* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	128	     /* max number of sectors on one chip */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE /* Monitor at beginning of flash */

#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x001f0000)
#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	(64 * 1024)
#define CONFIG_ENV_SIZE		(64 * 1024)

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
#define CONFIG_SYS_FLASH_CFI		1	/* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use drivers/cfi_flash.c */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* Use buffered writes (~10x faster) */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* Use hardware sector protection */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(100*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(100*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/*
 * JFFS2 partitions
 */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV	"nor0"

/*
 * NAND flash
 */
#define CONFIG_NAND_MXC
#define CONFIG_MXC_NAND_REGS_BASE	NFC_BASE_ADDR
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		NFC_BASE_ADDR
#define CONFIG_MXC_NAND_HWECC

#endif /* __CONFIG_H */
