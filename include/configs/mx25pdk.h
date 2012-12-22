/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
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
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

/* High Level Configuration Options */

#define CONFIG_MX25
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_TEXT_BASE		0x81200000
#define CONFIG_MXC_GPIO

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_MACH_TYPE	MACH_TYPE_MX25_3DS

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)

/* Physical Memory Map */

#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		0x80000000
#define PHYS_SDRAM_1_SIZE	(64 * 1024 * 1024)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	IMX_RAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE	IMX_RAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Memory Test */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE/2)
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)

/* Serial Info */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE
#define CONFIG_CONS_INDEX	1	/* use UART0 for console */
#define CONFIG_BAUDRATE		115200	/* Default baud rate */

/* No NOR flash present */
#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#define CONFIG_ENV_SIZE        (8 * 1024)

#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 0

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT	"MX25PDK U-Boot > "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size  */
/* Print buffer sz */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
		sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

/* U-Boot commands */
#include <config_cmd_default.h>
#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

/* Ethernet */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0x1f
#define CONFIG_MII
#define CONFIG_CMD_NET
#define CONFIG_ENV_OVERWRITE

/* ESDHC driver */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	1

/* PMIC Configs */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_FSL
#define CONFIG_PMIC_FSL_MC34704
#define CONFIG_SYS_FSL_PMIC_I2C_ADDR	0x54

#define CONFIG_DOS_PARTITION

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_HARD_I2C
#define CONFIG_I2C_MXC
#define CONFIG_SYS_I2C_BASE		IMX_I2C_BASE
#define CONFIG_SYS_I2C_SPEED		100000

/* Ethernet Configs */

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET

#define CONFIG_BOOTDELAY	1

#define CONFIG_LOADADDR		0x81000000	/* loadaddr env var */
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"uimage=uImage\0" \
	"netargs=setenv bootargs console=ttymxc0,${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"bootcmd=run netargs; dhcp ${uimage}; bootm\0" \

#endif /* __CONFIG_H */
