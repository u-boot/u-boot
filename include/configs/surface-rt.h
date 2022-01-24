/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * Copyright (c) 2021, Open Surface RT
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra30-common.h"

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING		"Microsoft Surface RT"

#define SURFACE_RT_BOOTMENU \
	"bootmenu_0=mount internal storage=usb start && ums 0 mmc 0; bootmenu\0" \
	"bootmenu_1=mount external storage=usb start && ums 0 mmc 1; bootmenu\0" \
	"bootmenu_2=fastboot=echo Starting Fastboot protocol ...; fastboot usb 0; bootmenu\0" \
	"bootmenu_3=boot from USB=usb reset; usb start; bootflow scan\0" \
	"bootmenu_4=reboot RCM=enterrcm\0" \
	"bootmenu_5=reboot=reset\0" \
	"bootmenu_6=power off=poweroff\0" \
	"bootmenu_delay=-1\0"

#define BOARD_EXTRA_ENV_SETTINGS \
	"button_cmd_0_name=Volume Down\0" \
	"button_cmd_0=bootmenu\0" \
	"button_cmd_1_name=Hall Sensor\0" \
	"button_cmd_1=poweroff\0" \
	"partitions=name=emmc,start=0,size=-,uuid=${uuid_gpt_rootfs}\0" \
	SURFACE_RT_BOOTMENU

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
