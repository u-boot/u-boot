/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/rk322x_common.h>


/* Store env in emmc */
#define CONFIG_SYS_MMC_ENV_DEV          0
#define CONFIG_SYS_MMC_ENV_PART         0
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#ifndef CONFIG_SPL_BUILD
/* Enable gpt partition table */
#undef PARTS_DEFAULT
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=loader_a,start=4M,size=4M,uuid=${uuid_gpt_loader};" \
	"name=loader_b,size=4M,uuid=${uuid_gpt_reserved};" \
	"name=trust_a,size=4M,uuid=${uuid_gpt_reserved};" \
	"name=trust_b,size=4M,uuid=${uuid_gpt_reserved};" \
	"name=misc,size=4M,uuid=${uuid_gpt_misc};" \
	"name=metadata,size=16M,uuid=${uuid_gpt_metadata};" \
	"name=boot_a,size=32M,uuid=${uuid_gpt_boot_a};" \
	"name=boot_b,size=32M,uuid=${uuid_gpt_boot_b};" \
	"name=system_a,size=818M,uuid=${uuid_gpt_system_a};" \
	"name=system_b,size=818M,uuid=${uuid_gpt_system_b};" \
	"name=vendor_a,size=50M,uuid=${uuid_gpt_vendor_a};" \
	"name=vendor_b,size=50M,uuid=${uuid_gpt_vendor_b};" \
	"name=cache,size=100M,uuid=${uuid_gpt_cache};" \
	"name=persist,size=4M,uuid=${uuid_gpt_persist};" \
	"name=userdata,size=-,uuid=${uuid_gpt_userdata};\0" \

#define CONFIG_PREBOOT

#define CONFIG_SYS_BOOT_RAMDISK_HIGH

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
	"mmc read 0x61000000 0x8000 0x5000;" \
	"bootm 0x61000000" \

/* Enable atags */
#define CONFIG_SYS_BOOTPARAMS_LEN	(64*1024)
#define CONFIG_INITRD_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG

#endif

#endif
