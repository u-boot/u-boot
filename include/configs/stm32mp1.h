/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STM32MP15x CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <linux/sizes.h>
#include <asm/arch/stm32.h>

#define CONFIG_PREBOOT

/*
 * Number of clock ticks in 1 sec
 */
#define CONFIG_SYS_HZ				1000

/* PSCI support */
#define CONFIG_ARMV7_PSCI_1_0
#define CONFIG_ARMV7_SECURE_BASE		STM32_SYSRAM_BASE
#define CONFIG_ARMV7_SECURE_MAX_SIZE		STM32_SYSRAM_SIZE

/*
 * malloc() pool size
 */
#define CONFIG_SYS_MALLOC_LEN			SZ_32M

/*
 * Configuration of the external SRAM memory used by U-Boot
 */
#define CONFIG_SYS_SDRAM_BASE			STM32_DDR_BASE
#define CONFIG_SYS_INIT_SP_ADDR			CONFIG_SYS_TEXT_BASE

/*
 * Console I/O buffer size
 */
#define CONFIG_SYS_CBSIZE			SZ_1K

/*
 * Needed by "loadb"
 */
#define CONFIG_SYS_LOAD_ADDR			STM32_DDR_BASE

/*
 * Env parameters
 */
#define CONFIG_ENV_SIZE				SZ_4K

/* ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* SPL support */
#ifdef CONFIG_SPL
/* BOOTROM load address */
#define CONFIG_SPL_TEXT_BASE		0x2FFC2500
/* SPL use DDR */
#define CONFIG_SPL_BSS_START_ADDR	0xC0200000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000
#define CONFIG_SYS_SPL_MALLOC_START	0xC0300000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00100000

/* limit SYSRAM usage to first 128 KB */
#define CONFIG_SPL_MAX_SIZE		0x00020000
#define CONFIG_SPL_STACK		(STM32_SYSRAM_BASE + \
					 STM32_SYSRAM_SIZE)
#endif /* #ifdef CONFIG_SPL */

/*MMC SD*/
#define CONFIG_SYS_MMC_MAX_DEVICE	3
#define CONFIG_SUPPORT_EMMC_BOOT

#if !defined(CONFIG_SPL) || !defined(CONFIG_SPL_BUILD)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 2)

#include <config_distro_bootcmd.h>

#define STM32MP_PREBOOT	\
	"echo \"Boot over ${boot_device}${boot_instance}!\"; " \
	"if test \"${boot_device}\" = \"mmc\"; then " \
		"env set boot_targets \"mmc${boot_instance}\"; "\
	"fi;"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"scriptaddr=0xC0000000\0" \
	"pxefile_addr_r=0xC0000000\0" \
	"kernel_addr_r=0xC1000000\0" \
	"fdt_addr_r=0xC4000000\0" \
	"ramdisk_addr_r=0xC4100000\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"preboot=" STM32MP_PREBOOT "\0" \
	BOOTENV

#endif /* ifndef CONFIG_SPL_BUILD */

#endif /* __CONFIG_H */
