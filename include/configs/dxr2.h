/*
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/include/configs/am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_DXR2_H
#define __CONFIG_DXR2_H

#define CONFIG_SIEMENS_DXR2
#define MACH_TYPE_DXR2			4315
#define CONFIG_SIEMENS_MACH_TYPE	MACH_TYPE_DXR2

#include "siemens-am33x-common.h"

#define CONFIG_SYS_MPUCLK	275
#define DXR2_IOCTRL_VAL	0x18b
#define DDR_PLL_FREQ	303
#undef CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC

#define BOARD_DFU_BUTTON_GPIO	27
#define BOARD_DFU_BUTTON_LED	64

#undef CONFIG_DOS_PARTITION
#undef CONFIG_CMD_FAT


 /* Physical Memory Map */
#define CONFIG_MAX_RAM_BANK_SIZE	(1024 << 20)	/* 1GB */

/* I2C Configuration */
#define CONFIG_SYS_I2C_SPEED		100000

#define CONFIG_SYS_I2C_EEPROM_ADDR              0x50
#define EEPROM_ADDR_DDR3 0x90
#define EEPROM_ADDR_CHIP 0x120

#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x300

#undef CONFIG_SPL_NET_SUPPORT
#undef CONFIG_SPL_NET_VCI_STRING
#undef CONFIG_SPL_ETH_SUPPORT

#undef CONFIG_MII
#undef CONFIG_PHY_GIGE
#define CONFIG_PHY_ADDR			0
#define CONFIG_PHY_SMSC

#define CONFIG_FACTORYSET

/* Watchdog */
#define CONFIG_OMAP_WATCHDOG

#ifndef CONFIG_SPL_BUILD

/* Default env settings */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"hostname=dxr2\0" \
	"nand_img_size=0x400000\0" \
	"optargs=\0" \
	CONFIG_COMMON_ENV_SETTINGS

#ifndef CONFIG_RESTORE_FLASH
/* set to negative value for no autoboot */
#define CONFIG_BOOTDELAY		3

#define CONFIG_BOOTCOMMAND \
"if dfubutton; then " \
	"run dfu_start; " \
	"reset; " \
"fi;" \
"run nand_boot;" \
"reset;"


#else
#define CONFIG_BOOTDELAY		0

#define CONFIG_BOOTCOMMAND			\
	"setenv autoload no; "			\
	"dhcp; "				\
	"if tftp 80000000 debrick.scr; then "	\
		"source 80000000; "		\
	"fi"
#endif
#endif	/* CONFIG_SPL_BUILD */
#endif	/* ! __CONFIG_DXR2_H */
