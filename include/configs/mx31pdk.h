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

#include <asm/arch/mx31-regs.h>

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

#if defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT
#endif

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(2*CONFIG_ENV_SIZE + 2 * 128 * 1024)
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

#define CONFIG_FSL_PMIC
#define CONFIG_FSL_PMIC_BUS	1
#define CONFIG_FSL_PMIC_CS	2
#define CONFIG_FSL_PMIC_CLK	1000000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_2 | SPI_CS_HIGH)
#define CONFIG_RTC_MC13783	1

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
#define CONFIG_CMD_NAND

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
		"tftpboot 0x81000000 uImage-mx31; bootm\0"		\
	"prg_uboot=tftpboot 0x81000000 u-boot-nand.bin; "		\
		"nand erase 0x0 0x40000; "				\
		"nand write 0x81000000 0x0 0x40000\0"

#define CONFIG_NET_MULTI
#define CONFIG_SMC911X		1
#define CONFIG_SMC911X_BASE	0xB6000000
#define CONFIG_SMC911X_32_BIT	1

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

#define CONFIG_ENV_IS_IN_NAND		1
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_OFFSET_REDUND	0x60000
#define CONFIG_ENV_SIZE			(128 * 1024)

/*
 * NAND driver
 */
#define CONFIG_NAND_MXC
#define CONFIG_MXC_NAND_REGS_BASE      NFC_BASE_ADDR
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#define CONFIG_SYS_NAND_BASE           NFC_BASE_ADDR
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_NAND_LARGEPAGE

/* NAND configuration for the NAND_SPL */

/* Start copying real U-boot from the second page */
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x800
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x30000
/* Load U-Boot to this address */
#define CONFIG_SYS_NAND_U_BOOT_DST	0x87f00000
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST

#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_SIZE		(256 * 1024 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0


/* Configuration of lowlevel_init.S (clocks and SDRAM) */
#define CCM_CCMR_SETUP		0x074B0BF5
#define CCM_PDR0_SETUP_532MHZ	(PDR0_CSI_PODF(0x1ff) | PDR0_PER_PODF(7) | \
				 PDR0_HSP_PODF(3) | PDR0_NFC_PODF(5) |     \
				 PDR0_IPG_PODF(1) | PDR0_MAX_PODF(3) |     \
				 PDR0_MCU_PODF(0))
#define CCM_MPCTL_SETUP_532MHZ	(PLL_PD(0) | PLL_MFD(51) | PLL_MFI(10) |   \
				 PLL_MFN(12))

#define ESDMISC_MDDR_SETUP	0x00000004
#define ESDMISC_MDDR_RESET_DL	0x0000000c
#define ESDCFG0_MDDR_SETUP	0x006ac73a

#define ESDCTL_ROW_COL		(ESDCTL_SDE | ESDCTL_ROW(2) | ESDCTL_COL(2))
#define ESDCTL_SETTINGS		(ESDCTL_ROW_COL | ESDCTL_SREFR(3) | \
				 ESDCTL_DSIZ(2) | ESDCTL_BL(1))
#define ESDCTL_PRECHARGE	(ESDCTL_ROW_COL | ESDCTL_CMD_PRECHARGE)
#define ESDCTL_AUTOREFRESH	(ESDCTL_ROW_COL | ESDCTL_CMD_AUTOREFRESH)
#define ESDCTL_LOADMODEREG	(ESDCTL_ROW_COL | ESDCTL_CMD_LOADMODEREG)
#define ESDCTL_RW		ESDCTL_SETTINGS

#endif /* __CONFIG_H */
