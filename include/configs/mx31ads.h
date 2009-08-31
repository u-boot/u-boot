/*
 * Copyright (C) 2008, Guennadi Liakhovetski <lg@denx.de>
 *
 * Configuration settings for the MX31ADS Freescale board.
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
#define CONFIG_ARM1136		1		/* This is an arm1136 CPU core */
#define CONFIG_MX31		1		/* in a mx31 */
#define CONFIG_MX31_HCLK_FREQ	26000000	/* RedBoot says 26MHz */
#define CONFIG_MX31_CLK32	32768

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/*
 * Disabled for now due to build problems under Debian and a significant increase
 * in the final file size: 144260 vs. 109536 Bytes.
 */
#if 0
#define CONFIG_OF_LIBFDT		1
#define CONFIG_FIT			1
#define CONFIG_FIT_VERBOSE		1
#endif

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART	1
#define CONFIG_SYS_MX31_UART1		1

#define CONFIG_HARD_SPI		1
#define CONFIG_MXC_SPI		1
#define CONFIG_DEFAULT_SPI_BUS	1
#define CONFIG_DEFAULT_SPI_MODE	(SPI_MODE_2 | SPI_CS_HIGH)

#define CONFIG_RTC_MC13783	1
/* MC13783 connected to CSPI2 and SS0 */
#define CONFIG_MC13783_SPI_BUS	1
#define CONFIG_MC13783_SPI_CS	0

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SPI
#define CONFIG_CMD_DATE

#define CONFIG_BOOTDELAY	3

#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"uboot_addr=0xa0000000\0"					\
	"uboot=mx31ads/u-boot.bin\0"					\
	"kernel=mx31ads/uImage\0"					\
	"nfsroot=/opt/eldk/arm\0"					\
	"bootargs_base=setenv bootargs console=ttymxc0,115200\0"	\
	"bootargs_nfs=setenv bootargs ${bootargs} root=/dev/nfs "	\
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0"	\
	"bootcmd=run bootcmd_net\0"					\
	"bootcmd_net=run bootargs_base bootargs_nfs; "			\
		"tftpboot ${loadaddr} ${kernel}; bootm\0"		\
	"prg_uboot=tftpboot ${loadaddr} ${uboot}; "			\
		"protect off ${uboot_addr} 0xa003ffff; "		\
		"erase ${uboot_addr} 0xa003ffff; "			\
		"cp.b ${loadaddr} ${uboot_addr} ${filesize}; "		\
		"setenv filesize; saveenv\0"

#define CONFIG_NET_MULTI
#define CONFIG_CS8900
#define CONFIG_CS8900_BASE	0xb4020300
#define CONFIG_CS8900_BUS16		1	/* follow the Linux driver */

/*
 * The MX31ADS board seems to have a hardware "peculiarity" confirmed under
 * U-Boot, RedBoot and Linux: the ethernet Rx signal is reaching the CS8900A
 * controller inverted. The controller is capable of detecting and correcting
 * this, but it needs 4 network packets for that. Which means, at startup, you
 * will not receive answers to the first 4 packest, unless there have been some
 * broadcasts on the network, or your board is on a hub. Reducing the ARP
 * timeout from default 5 seconds to 200ms we speed up the initial TFTP
 * transfer, should the user wish one, significantly.
 */
#define CONFIG_ARP_TIMEOUT	200UL

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0		/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x10000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_HZ			1000

#define CONFIG_CMDLINE_EDITING	1

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
#define PHYS_SDRAM_1		CSD0_BASE
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		CS0_BASE
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	262		/* max number of sectors on one chip */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE	/* Monitor at beginning of flash */
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256KiB */

#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	(32 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

/* S29WS256N NOR flash has 4 32KiB small sectors at the beginning and at the end.
 * The rest of 32MiB is in 128KiB big sectors. U-Boot occupies the low 4 sectors,
 * if we put environment next to it, we will have to occupy 128KiB for it.
 * Putting it at the top of flash we use only 32KiB. */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_ENV_SECT_SIZE)

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
#define CONFIG_SYS_FLASH_CFI			1 /* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER		1 /* Use drivers/cfi_flash.c */
#define CONFIG_FLASH_SPANSION_S29WS_N	1 /* A non-standard buffered write algorithm */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1 /* Use buffered writes (~10x faster) */
#define CONFIG_SYS_FLASH_PROTECTION		1 /* Use hardware sector protection */

/*
 * JFFS2 partitions
 */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV	"nor0"

#endif /* __CONFIG_H */
