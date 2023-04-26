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

#ifdef CONFIG_SYS_BOOT_LOCKED
#define EXTRA_ENV_FLAGS
#define SETUP_BOOT_MENU "setup_boot_menu=setenv bootmenu_0 eMMC=run bootcmd\0"
#else
#define EXTRA_ENV_FLAGS "mmcdev:dw,"
#define SETUP_BOOT_MENU "setup_boot_menu=" \
	"if test \"${mmcdev}\" = 1; then " \
		"setenv emmc_priority 0;" \
		"setenv sd_priority 1;" \
	"else " \
		"setenv emmc_priority 1;" \
		"setenv sd_priority 0;" \
	"fi;" \
	"setenv bootmenu_${emmc_priority} eMMC=run boot_emmc;" \
	"setenv bootmenu_${sd_priority} SD=run boot_sd;\0"
#endif

#define CFG_ENV_FLAGS_LIST_STATIC \
	"mmcpart:dw," \
	"mmcpart_committed:dw," \
	"ustate:dw," \
	"bootcount:dw," \
	"bootlimit:dw," \
	"upgrade_available:dw," \
	EXTRA_ENV_FLAGS

#define CFG_EXTRA_ENV_SETTINGS \
	"image=fitImage\0" \
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
	"fit_addr=0x88000000\0" \
	"loadimage=load mmc ${mmcdev}:${gpt_partition_entry} ${fit_addr} boot/${image}\0" \
	"loadpart=gpt setenv mmc ${mmcdev} rootfs-${mmcpart_committed}\0" \
	"loadbootpart=mmc partconf 1 boot_part\0" \
	"boot_sd=setenv mmcdev_wanted 0; run persist_mmcdev; run bootcmd;\0" \
	"boot_emmc=setenv mmcdev_wanted 1; run persist_mmcdev; run bootcmd;\0" \
	"persist_mmcdev=" \
		"if test \"${mmcdev}\" != \"${mmcdev_wanted}\"; then " \
			"setenv mmcdev \"${mmcdev_wanted}\";" \
			"saveenv;" \
		"fi;\0" \
	"mmcboot=echo Booting...; " \
		"echo mmcdev: ${mmcdev}; " \
	  "run commit_mmc; " \
		"echo mmcpart: ${mmcpart_committed}; " \
		"run loadpart; " \
		"echo gptpart: ${gpt_partition_entry}; " \
		"run loadbootpart; " \
		"if run loadimage; then " \
			"; " \
		"else " \
			"run altbootcmd; " \
		"fi; " \
		"run mmcargs; " \
		"if bootm ${fit_addr}; then " \
			"; " \
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
		"run bootcmd;\0" \
		SETUP_BOOT_MENU

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif
