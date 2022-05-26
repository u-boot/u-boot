/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Collabora Ltd.
 */

#ifndef __IMX8MN_VAR_SOM_H
#define __IMX8MN_VAR_SOM_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_BOOTM_LEN		(32 * SZ_1M)

#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define CONFIG_SPL_BSS_START_ADDR	0x950000
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(MMC, mmc, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na) \

#include <config_distro_bootcmd.h>

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x43800000\0" \
	"fdt_addr_r=0x43000000\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fastboot_partition_alias_all=" \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".0:0\0" \
	"fastboot_partition_alias_bootloader=" \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".1:0\0" \
	"emmc_dev=" __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) "\0" \
	"emmc_ack=1\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV

/* Link Definitions */

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	SZ_512K

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			SZ_1G /* 1GB DDR */

#define CONFIG_MXC_UART_BASE		UART_BASE_ADDR(4)

/* USDHC */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#endif /* __IMX8MN_VAR_SOM_H */
