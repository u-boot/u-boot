/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _ROCKCHIP_COMMON_H_
#define _ROCKCHIP_COMMON_H_
#include <linux/sizes.h>

#ifndef CONFIG_SPL_BUILD
#include <config_distro_defaults.h>

/* First try to boot from SD (index 0), then eMMC (index 1 */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1)

 /* Enable gpt partition table */
#define CONFIG_CMD_GPT
#define CONFIG_RANDOM_UUID
#define CONFIG_PARTITION_UUIDS
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=loader1,start=32K,size=4000K,uuid=${uuid_gpt_loader1};" \
	"name=reserved1,size=64K,uuid=${uuid_gpt_reserved1};" \
	"name=reserved2,size=4M,uuid=${uuid_gpt_reserved2};" \
	"name=loader2,size=4MB,uuid=${uuid_gpt_loader2};" \
	"name=atf,size=4M,uuid=${uuid_gpt_atf};" \
	"name=boot,size=112M,bootable,uuid=${uuid_gpt_boot};" \
	"name=rootfs,size=-,uuid=${uuid_gpt_rootfs};\0" \

#endif

#endif /* _ROCKCHIP_COMMON_H_ */
