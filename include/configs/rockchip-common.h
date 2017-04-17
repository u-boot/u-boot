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
#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dchp, na)
#else
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(PXE, pxe, na) \
	func(DHCP, dchp, na)
#endif

#define CONFIG_RANDOM_UUID
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

#ifdef CONFIG_ROCKCHIP_SPL_BACK_TO_BROM
/* SPL @ 32k for 34k
 * u-boot directly after @ 68k for 400k or so
 * ENV @ 992k
 */
#define CONFIG_ENV_OFFSET ((1024-32) * 1024)
#else
/* SPL @ 32k for ~36k
 * ENV @ 96k
 * u-boot @ 128K
 */
#define CONFIG_ENV_OFFSET (96 * 1024)
#endif

#endif /* _ROCKCHIP_COMMON_H_ */
