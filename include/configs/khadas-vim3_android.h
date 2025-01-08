/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for the khadas VIM3 Android
 *
 * Copyright (C) 2021 Baylibre, SAS
 * Author: Guillaume LA ROQUE <glaroque@baylibre.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define LOGO_UUID "43a3305d-150f-4cc9-bd3b-38fca8693846;"
#define ROOT_UUID "ddb8c3f6-d94d-4394-b633-3134139cc2e0;"

#if defined(CONFIG_CMD_BCB) && defined(CONFIG_ANDROID_AB)
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=logo,start=512K,size=2M,uuid=" LOGO_UUID \
	"name=misc,size=512K,uuid=${uuid_gpt_misc};" \
	"name=dtbo_a,size=8M,uuid=${uuid_gpt_dtbo_a};" \
	"name=dtbo_b,size=8M,uuid=${uuid_gpt_dtbo_b};" \
	"name=vbmeta_a,size=512K,uuid=${uuid_gpt_vbmeta_a};" \
	"name=vbmeta_b,size=512K,uuid=${uuid_gpt_vbmeta_b};" \
	"name=boot_a,size=64M,bootable,uuid=${uuid_gpt_boot_a};" \
	"name=boot_b,size=64M,bootable,uuid=${uuid_gpt_boot_b};" \
	"name=super,size=3072M,uuid=${uuid_gpt_super};" \
	"name=userdata,size=11218M,uuid=${uuid_gpt_userdata};" \
	"name=rootfs,size=-,uuid=" ROOT_UUID
#else
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=logo,start=512K,size=2M,uuid=" LOGO_UUID \
	"name=misc,size=512K,uuid=${uuid_gpt_misc};" \
	"name=dtbo,size=8M,uuid=${uuid_gpt_dtbo};" \
	"name=vbmeta,size=512K,uuid=${uuid_gpt_vbmeta};" \
	"name=boot,size=64M,bootable,uuid=${uuid_gpt_boot};" \
	"name=recovery,size=64M,uuid=${uuid_gpt_recovery};" \
	"name=cache,size=256M,uuid=${uuid_gpt_cache};" \
	"name=super,size=1792M,uuid=${uuid_gpt_super};" \
	"name=userdata,size=12722M,uuid=${uuid_gpt_userdata};" \
	"name=rootfs,size=-,uuid=" ROOT_UUID
#endif

#define CFG_EXTRA_ENV_SETTINGS                                    \
	"board=vim3\0"                                               \
	"board_name=vim3\0"                                          \
	"bootmeths=android\0"                                         \
	"bootcmd=bootflow scan\0"                                     \
	"adtb_idx=3\0"                                                \
	"partitions=" PARTS_DEFAULT "\0"                              \
	"mmcdev=2\0"                                                  \
	"fastboot_raw_partition_bootloader=0x1 0xfff mmcpart 1\0"     \
	"fastboot_raw_partition_bootenv=0x0 0xfff mmcpart 2\0"        \
	"stdin=" STDIN_CFG "\0"                                       \
	"stdout=" STDOUT_CFG "\0"                                     \
	"stderr=" STDOUT_CFG "\0"                                     \
	"dtboaddr=0x08200000\0"                                       \
	"loadaddr=0x01080000\0"                                       \
	"fdt_addr_r=0x01000000\0"                                     \
	"scriptaddr=0x08000000\0"                                     \
	"kernel_addr_r=0x01080000\0"                                  \
	"pxefile_addr_r=0x01080000\0"                                 \
	"ramdisk_addr_r=0x13000000\0"                                 \

#include <configs/meson64.h>

#endif /* __CONFIG_H */
