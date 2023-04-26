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
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* default to no extra bootparams, we need an empty define for stringification*/
#ifndef EXTRA_BOOTPARAMS
#define EXTRA_BOOTPARAMS
#endif

#define CFG_EXTRA_ENV_SETTINGS \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"fdtfile=imx7d-smegw01.dtb\0" \
	"fdt_addr=0x83000000\0" \
	"bootm_size=0x10000000\0" \
	"mmcdev=1\0" \
	"mmcpart=1\0" \
	"mmcpart_committed=1\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/mmcblk${mmcdev}p${gpt_partition_entry} rootwait rw " \
		__stringify(EXTRA_BOOTPARAMS) " SM_ROOT_DEV=${mmcdev} SM_ROOT_PART=${gpt_partition_entry} SM_BOOT_PART=${boot_part}\0" \
	"commit_mmc=if test \"${ustate}\" = 1 -a \"${mmcpart}\" != \"${mmcpart_committed}\"; then " \
	              "setenv mmcpart_committed ${mmcpart};" \
								"saveenv;" \
						  "fi;\0" \
	"bootlimit=3\0" \
	"loadimage=load mmc ${mmcdev}#rootfs-${mmcpart_committed} ${loadaddr} boot/${image}\0" \
	"loadfdt=load mmc ${mmcdev}#rootfs-${mmcpart_committed} ${fdt_addr} boot/${fdtfile}\0" \
	"loadpart=gpt setenv mmc ${mmcdev} rootfs-${mmcpart_committed}\0" \
	"loadbootpart=mmc partconf 1 boot_part\0" \
	"mmcboot=echo Booting from mmc ...; " \
	  "run commit_mmc; " \
		"run loadpart; " \
		"run loadbootpart; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"if bootz ${loadaddr} - ${fdt_addr}; then " \
				"; " \
			"else " \
				"run altbootcmd; " \
			"fi;" \
		"else " \
			"run altbootcmd; " \
		"fi;\0" \
	"altbootcmd=echo Performing rollback...; " \
		"if test \"${mmcpart_committed}\" = 1; then " \
			"setenv mmcpart 2; " \
			"setenv mmcpart_committed 2;" \
		"else " \
			"setenv mmcpart 1; " \
			"setenv mmcpart_committed 1;" \
		"fi; setenv bootcount 0; setenv upgrade_available; setenv ustate 3; saveenv; " \
		"run bootcmd;\0"

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif
