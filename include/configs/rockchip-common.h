/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef _ROCKCHIP_COMMON_H_
#define _ROCKCHIP_COMMON_H_
#include <linux/sizes.h>

#ifndef CFG_CPUID_OFFSET
#define CFG_CPUID_OFFSET	0x7
#endif

#ifndef CONFIG_SPL_BUILD

/* First try to boot from SD (index 1), then eMMC (index 0) */
#if IS_ENABLED(CONFIG_CMD_MMC)
	#define BOOT_TARGET_MMC(func) \
		func(MMC, mmc, 1) \
		func(MMC, mmc, 0)
#else
	#define BOOT_TARGET_MMC(func)
#endif

#if IS_ENABLED(CONFIG_CMD_NVME)
	#define BOOT_TARGET_NVME(func) func(NVME, nvme, 0)
#else
	#define BOOT_TARGET_NVME(func)
#endif

#if IS_ENABLED(CONFIG_CMD_SCSI)
	#define BOOT_TARGET_SCSI(func) func(SCSI, scsi, 0)
#else
	#define BOOT_TARGET_SCSI(func)
#endif

#if IS_ENABLED(CONFIG_CMD_USB)
	#define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
	#define BOOT_TARGET_USB(func)
#endif

#if CONFIG_IS_ENABLED(CMD_PXE)
	#define BOOT_TARGET_PXE(func) func(PXE, pxe, na)
#else
	#define BOOT_TARGET_PXE(func)
#endif

#if CONFIG_IS_ENABLED(CMD_DHCP)
	#define BOOT_TARGET_DHCP(func) func(DHCP, dhcp, na)
#else
	#define BOOT_TARGET_DHCP(func)
#endif

#if IS_ENABLED(CONFIG_CMD_SF)
	#define BOOT_TARGET_SF(func)	func(SF, sf, 0)
#else
	#define BOOT_TARGET_SF(func)
#endif

#ifdef CONFIG_ROCKCHIP_RK3399
#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_MMC(func) \
	BOOT_TARGET_NVME(func) \
	BOOT_TARGET_SCSI(func) \
	BOOT_TARGET_USB(func) \
	BOOT_TARGET_PXE(func) \
	BOOT_TARGET_DHCP(func) \
	BOOT_TARGET_SF(func)
#define BOOT_TARGETS	"mmc1 mmc0 nvme scsi usb pxe dhcp spi"
#else
#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_MMC(func) \
	BOOT_TARGET_USB(func) \
	BOOT_TARGET_PXE(func) \
	BOOT_TARGET_DHCP(func)
#define BOOT_TARGETS	"mmc1 mmc0 usb pxe dhcp"
#endif

#ifdef CONFIG_ARM64
#define ROOT_UUID "B921B045-1DF0-41C3-AF44-4C6F280D3FAE;\0"
#else
#define ROOT_UUID "69DAD710-2CE4-4E3C-B16C-21A1D49ABED3;\0"
#endif
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=loader1,start=32K,size=4000K,uuid=${uuid_gpt_loader1};" \
	"name=loader2,start=8MB,size=4MB,uuid=${uuid_gpt_loader2};" \
	"name=trust,size=4M,uuid=${uuid_gpt_atf};" \
	"name=boot,size=112M,bootable,uuid=${uuid_gpt_boot};" \
	"name=rootfs,size=-,uuid="ROOT_UUID

#endif

#endif /* _ROCKCHIP_COMMON_H_ */
