/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 NXP Semiconductors
 * Copyright (C) 2021 Fabio Estevam <festevam@denx.de>
 *
 * Configuration settings for the smegw01 board.
 */

#ifndef __SMEGW01_CONFIG_H
#define __SMEGW01_CONFIG_H

#include "mx7_common.h"
#include <imximage.h>

#define PHYS_SDRAM_SIZE		SZ_512M

/* MMC Config*/
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#define CONFIG_EXTRA_ENV_SETTINGS \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"fdtfile=imx7d-smegw01.dtb\0" \
	"fdt_addr=0x83000000\0" \
	"bootm_size=0x10000000\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/mmcblk0p${mmcpart} rootwait rw\0" \
	"loadimage=load mmc ${mmcdev}:${mmcpart} ${loadaddr} boot/${image}\0" \
	"loadfdt=load mmc ${mmcdev}:${mmcpart} ${fdt_addr} boot/${fdtfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"bootz ${loadaddr} - ${fdt_addr}; " \
		"fi;\0" \

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#endif
