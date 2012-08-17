/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX51EVK Board
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

#define CONFIG_MX51	/* in a mx51 */

#define CONFIG_SYS_MX5_HCLK	24000000
#define CONFIG_SYS_MX5_CLK32		32768
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_TEXT_BASE	0x97800000

#include <asm/arch/imx-regs.h>
/*
 * Disabled for now due to build problems under Debian and a significant
 * increase in the final file size: 144260 vs. 109536 Bytes.
 */

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_OF_LIBFDT

#define CONFIG_MACH_TYPE	MACH_TYPE_MX51_BABBAGE
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)

#define CONFIG_BOARD_LATE_INIT

/*
 * Hardware drivers
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE
#define CONFIG_MXC_GPIO

/*
 * SPI Configs
 * */
#define CONFIG_CMD_SPI

#define CONFIG_MXC_SPI

/* PMIC Controller */
#define CONFIG_PMIC
#define CONFIG_PMIC_SPI
#define CONFIG_PMIC_FSL
#define CONFIG_FSL_PMIC_BUS	0
#define CONFIG_FSL_PMIC_CS	0
#define CONFIG_FSL_PMIC_CLK	2500000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CONFIG_FSL_PMIC_BITLEN	32
#define CONFIG_RTC_MC13XXX

/*
 * MMC Configs
 * */
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	2

#define CONFIG_MMC

#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

/*
 * Eth Configs
 */
#define CONFIG_HAS_ETH1
#define CONFIG_MII
#define CONFIG_DISCOVER_PHY

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR	0x1F

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX5
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	PORT_PTS_ULPI
#define CONFIG_MXC_USB_FLAGS	MXC_EHCI_POWER_PINS_ENABLED

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#undef CONFIG_CMD_IMLS

#define CONFIG_CMD_DATE

#define CONFIG_BOOTDELAY	3

#define CONFIG_ETHPRIME		"FEC0"

#define CONFIG_LOADADDR		0x90800000	/* loadaddr env var */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"uimage=uImage\0" \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"mmcroot=/dev/mmcblk0p3 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"mmcargs=setenv bootargs console=ttymxc0,${baudrate} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loaduimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${uimage}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm\0" \
	"netargs=setenv bootargs console=ttymxc0,${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"dhcp ${uimage}; bootm\0" \

#define CONFIG_BOOTCOMMAND \
	"if mmc rescan ${mmcdev}; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run netboot; " \
			"fi; " \
		"fi; " \
	"else run netboot; fi"

#define CONFIG_ARP_TIMEOUT	200UL

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"MX51EVK U-Boot > "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START       0x90000000
#define CONFIG_SYS_MEMTEST_END         0x90010000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_HZ		1000
#define CONFIG_CMDLINE_EDITING

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 * 1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(512 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_DDR_CLKSEL	0
#define CONFIG_SYS_CLKTL_CBCDR	0x59E35100

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#define CONFIG_ENV_SIZE        (8 * 1024)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 0

#endif
