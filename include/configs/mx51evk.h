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
#define CONFIG_SKIP_RELOCATE_UBOOT

#define CONFIG_MX51_HCLK_FREQ		24000000	/* RedBoot says 26MHz */
#define CONFIG_MX51_CLK32		32768
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_L2_OFF

/*
 * Disabled for now due to build problems under Debian and a significant
 * increase in the final file size: 144260 vs. 109536 Bytes.
 */

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_REVISION_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

#define BOARD_LATE_INIT

/*
 * Hardware drivers
 */
#define CONFIG_MXC_UART
#define CONFIG_SYS_MX51_UART1

/*
 * SPI Configs
 * */
#define CONFIG_CMD_SPI

#define CONFIG_MXC_SPI

#define CONFIG_FSL_PMIC
#define CONFIG_FSL_PMIC_BUS	0
#define CONFIG_FSL_PMIC_CS	0
#define CONFIG_FSL_PMIC_CLK	2500000
#define CONFIG_FSL_PMIC_MODE	(SPI_CPOL | SPI_CS_HIGH)

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
#define CONFIG_NET_MULTI
#define CONFIG_MII
#define CONFIG_DISCOVER_PHY

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR	0x1F

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET

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

#define CONFIG_BOOTDELAY	3

#define CONFIG_PRIME	"FEC0"

#define CONFIG_LOADADDR		0x90800000	/* loadaddr env var */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
		"netdev=eth0\0"						\
		"uboot_addr=0xa0000000\0"				\
		"uboot=u-boot.bin\0"			\
		"loadaddr=0x90800000\0"			\
		"bootargs_base=setenv bootargs console=tty "\
			"console=ttymxc0,${baudrate}\0"\
		"bootargs_nfs=setenv bootargs ${bootargs} root=/dev/nfs "\
			"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0"\
		"bootcmd=run bootcmd_net\0"				\
		"bootcmd_net=run bootargs_base bootargs_nfs; "		\
			"tftpboot ${loadaddr} ${kernel}; bootm\0"

#define CONFIG_ARP_TIMEOUT	200UL

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_PROMPT		"MX51EVK U-Boot > "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START       0x90000000
#define CONFIG_SYS_MEMTEST_END         0x10000

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

#define CONFIG_SYS_DDR_CLKSEL	0
#define CONFIG_SYS_CLKTL_CBCDR	0x59E35100

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SECT_SIZE    (128 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_IS_NOWHERE

#endif
