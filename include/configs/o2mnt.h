/*
 * (C) Copyright 2012
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFF000000   boot low boot high (standard configuration)
 * 0x00100000   boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xff000000	/* Standard: boot low */
#endif

/* Board specific flash config */
#define CONFIG_SYS_FLASH_BASE		0xff000000
#define CONFIG_SYS_FLASH_SIZE		0x01000000      /* maximum 16MB */
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	128

/*
 * Include common defines for all ifm boards
 */
#include "o2dnt-common.h"

/* GPIO configuration */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x00002004	/* no CAN */

/* Other board specific configs */
#define CONFIG_NETCONSOLE

#define CONFIG_SYS_BOOTCS_CFG		0x00087801
#define CONFIG_SYS_RESET_ADDRESS	0xff000000

#define CONFIG_SYS_MEMTEST_START	0x00100000      /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x03f00000	/* 1 - 63 MB in DRAM  */

#define CONFIG_BOARD_NAME		"o2mnt"
#define CONFIG_BOARD_BOOTCMD		"${newcmd}"
#define CONFIG_BOARD_MEM_LIMIT		xstr(62)
#define BOARD_POST_CRC32_END		xstr(0x01000000)

#ifndef CONFIG_IFM_SENSOR_TYPE
#define CONFIG_IFM_SENSOR_TYPE		"O2M110"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_IFM_DEFAULT_ENV_SETTINGS					\
	CONFIG_IFM_DEFAULT_ENV_OLD					\
	CONFIG_IFM_DEFAULT_ENV_NEW					\
	"linbot=ff060000\0"						\
	"lintop=ff25ffff\0"						\
	"rambot=ff260000\0"						\
	"ramtop=ffc5ffff\0"						\
	"jffbot=ffc60000\0"						\
	"jfftop=ffffffff\0"						\
	"ubobot=" xstr(CONFIG_SYS_FLASH_BASE) "\0"			\
	"ubotop=ff03ffff\0"						\
	"kernel_addr=0xff060000\0"					\
	"ramdisk_addr=0xff260000\0"					\
	"newcmd=run scrprot;run flash_ext2\0"				\
	"scrprot=protect on ${linbot} ${lintop};protect on ${rambot} "	\
		"${ramtop}\0"						\
	"flash_ext2=run ext2args addip addmem;bootm ${kernel_addr}\0"	\
	"ext2args=setenv bootargs root=/dev/mtdblock3 ro "		\
		"rootfstype=ext2\0"					\
	"pwm=mw f0000674 0x10006;mw f0000678 0x30000;"			\
		"mw f0000678 0x30001;mw f0000670 0x3\0"			\
	"master=mw f0000b00 0x00052006;mw f0000b0c $(IOpin);"		\
		"mw f0000b04 $(IOpin);mw f0000b10 0x24;run pwm\0"	\
	"sensortyp="CONFIG_IFM_SENSOR_TYPE"\0"				\
	"srelease=0.00\0"
