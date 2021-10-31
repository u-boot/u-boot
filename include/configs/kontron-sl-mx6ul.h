/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Kontron Electronics GmbH
 *
 * Configuration settings for the Kontron i.MX6UL boards/SoMs.
 */
#ifndef __KONTRON_MX6UL_CONFIG_H
#define __KONTRON_MX6UL_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#include "mx6_common.h"
#ifdef CONFIG_SPL_BUILD
#include "imx6_spl.h"
#endif

/* RAM */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* Board and environment settings */
#define CONFIG_MXC_UART_BASE		UART4_BASE
#define CONFIG_HOSTNAME			"kontron-mx6ul"
#define CONFIG_ETHPRIME			"eth0"

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#endif

/* Boot order for distro boot */
#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(UBIFS, ubifs, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/* MMC Configs */
#ifdef CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC1_BASE_ADDR
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x82000000\0" \
	"ramdisk_addr_r=0x88080000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"scriptaddr=0x80100000\0" \
	"bootdelay=3\0" \
	"ethact=" CONFIG_ETHPRIME "\0" \
	"hostname=" CONFIG_HOSTNAME "\0" \
	BOOTENV

#endif /* __KONTRON_MX6UL_CONFIG_H */
