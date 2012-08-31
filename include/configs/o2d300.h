/*
 * (C) Copyright 2012
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFC000000   boot low boot high (standard configuration)
 * 0x00100000   boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xfc000000	/* Standard: boot low */
#endif

/* Board specific flash config */
#define CONFIG_SYS_FLASH_BASE		0xfc000000
#define CONFIG_SYS_FLASH_SIZE		0x04000000      /* maximum 64MB */
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	512

/*
 * Include common defines for all ifm boards
 */
#include "o2dnt-common.h"

/*
 * GPIO configuration:
 * CS1 SDRAM activate + no CAN + no PCI
 */
#define CONFIG_SYS_GPS_PORT_CONFIG      0x8000A004

/* Other board specific configs */
#define CONFIG_SYS_BOOTCS_CFG		0x00057d01
#define CONFIG_SYS_RESET_ADDRESS	0xfc000000

#define CONFIG_SYS_MEMTEST_START	0x00100000      /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x07f00000	/* 1 - 127 MB in DRAM */

/* Use redundant environment */
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)

#define CONFIG_BOARD_NAME		"o2d300"
#define CONFIG_BOARD_BOOTCMD		"run dhcp_boot"
#define CONFIG_BOARD_MEM_LIMIT		xstr(126)
#define BOARD_POST_CRC32_END		xstr(0x02000000)

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_IFM_DEFAULT_ENV_SETTINGS					\
	CONFIG_IFM_DEFAULT_ENV_OLD					\
	CONFIG_IFM_DEFAULT_ENV_NEW					\
	"autoload=no\0"							\
	"dhcp_boot=run dhcpcmd;run flash_mtd\0"				\
	"flash_mtd=run mtd_args addip addmem;"				\
		"bootm ${kernel_addr}\0"				\
	"mtd_args=setenv bootargs root=/dev/mtdblock4 "			\
		"rw rootfstype=cramfs\0"				\
	"linbot=fc080000\0"						\
	"lintop=fc17ffff\0"						\
	"rambot=fc180000\0"						\
	"ramtop=fc57ffff\0"						\
	"jffbot=fc580000\0"						\
	"jfftop=fd39ffff\0"						\
	"ubobot=" xstr(CONFIG_SYS_FLASH_BASE) "\0"			\
	"ubotop=fc03ffff\0"						\
	"halname="CONFIG_BOARD_NAME"/"CONFIG_BOARD_NAME"_halcon\0"	\
	"halbot=fd3a0000\0"						\
	"haltop=fdf9ffff\0"						\
	"progHal=tftp 200000 ${halname};erase ${halbot} ${haltop};"	\
		"cp.b ${fileaddr} ${halbot} ${filesize}\0"		\
	"kernel_addr=0xfc060000\0"					\
	"ramdisk_addr=0xfc160000\0"					\
	"master=mw f0000b00 0x8005A006;mw f0000b0c ${IOpin};"		\
		"mw f0000b04 ${IOpin};mw f0000b10 0x20\0"		\
	"netretry=once\0"						\
	"protcmd=protect on ${linbot} ${lintop};"			\
		"protect on ${rambot} ${ramtop}\0"			\
	"o2derror=def_env\0"						\
	"sensorType=O2D300AA\0"
