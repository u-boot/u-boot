/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 */

#ifndef __PCM058_CONFIG_H
#define __PCM058_CONFIG_H

#ifdef CONFIG_SPL
#include "imx6_spl.h"
#endif

#include "mx6_common.h"

#define PHYS_SDRAM_SIZE		(1u * 1024 * 1024 * 1024)

/* Enable NAND support */
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define ENV_MMC \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"fitpart=1\0" \
	"mmcrootfstype=ext4\0" \
	"fitname=fitImage\0" \
	"mmcloadfit=load mmc ${mmcdev}:${fitpart} ${loadaddr} ${fitname}\0" \
	"mmcargs=setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcpart} " \
		"rootfstype=${mmcrootfstype} ${optargs}\0" \
	"mmcboot=run mmcloadfit;run mmcargs;bootm ${loadaddr}\0"

#define ENV_NAND \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"nandroot=ubi0:root ubi.mtd=rootfs\0" \
	"nandrootfstype=ubifs\0" \
	"nandargs=setenv bootargs root=${nandroot} " \
		"rootfstype=${nandrootfstype} ${mtdparts} ${optargs}\0" \
	"nandloadfit=ubi part rootfs;ubi readvol ${loadaddr} fit\0" \
	"nandboot=run nandloadfit;run nandargs;bootm ${loadaddr}\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x30000000\0" \
	"optargs=rw rootwait\0" \
	ENV_MMC \
	ENV_NAND
#endif
