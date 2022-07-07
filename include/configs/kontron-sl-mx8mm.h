/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Kontron Electronics GmbH
 *
 * Configuration settings for the Kontron SL/BL i.MX8M-Mini boards and modules (N81xx).
 */
#ifndef __KONTRON_MX8MM_CONFIG_H
#define __KONTRON_MX8MM_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#ifdef CONFIG_SPL_BUILD
#include <config.h>
#endif

/* RAM */
#define PHYS_SDRAM			DDR_CSD1_BASE_ADDR
#define PHYS_SDRAM_SIZE			(SZ_4G)
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x200000

/* Board and environment settings */
#define CONFIG_HOSTNAME			"kontron-mx8mm"

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#endif

/* GUID for capsule updatable firmware image */
#define KONTRON_SL_MX8MM_FIT_IMAGE_GUID \
	EFI_GUID(0xd488e45a, 0x4929, 0x4b55, 0x8c, 0x14, \
		 0x86, 0xce, 0xa2, 0xcd, 0x66, 0x29)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na)
#include <config_distro_bootcmd.h>
/* Do not try to probe USB net adapters for net boot */
#undef BOOTENV_RUN_NET_USB_START
#define BOOTENV_RUN_NET_USB_START

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x930000
#endif

#define ENV_MEM_LAYOUT_SETTINGS \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=0x42000000\0" \
	"fdt_addr_r=0x48000000\0" \
	"fdtoverlay_addr_r=0x49000000\0" \
	"ramdisk_addr_r=0x48080000\0" \
	"scriptaddr=0x40000000\0"\
	"pxefile_addr_r=0x40100000\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"dfu_alt_info=sf 0:0=flash-bin raw 0x400 0x1f0000\0" \
	"bootdelay=3\0" \
	"hostname=" CONFIG_HOSTNAME "\0" \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV

#endif /* __KONTRON_MX8MM_CONFIG_H */
