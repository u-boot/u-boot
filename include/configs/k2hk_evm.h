/*
 * Configuration header file for TI's k2hk-evm
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_K2HK_EVM_H
#define __CONFIG_K2HK_EVM_H

/* Platform type */
#define CONFIG_SOC_K2HK

/* U-Boot general configuration */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	"boot=ubi\0"							\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=ubifs,2048\0"	\
	"name_fdt=keystone-k2hk-evm.dtb\0"				\
	"name_mon=skern-k2hk.bin\0"					\
	"name_ubi=k2hk-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2hk-evm.gph\0"				\
	"name_fs=arago-console-image-k2hk-evm.cpio.gz\0"

#include <configs/ti_armv7_keystone2.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c200000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* Network */
#define CONFIG_KSNET_NETCP_V1_0
#define CONFIG_KSNET_CPSW_NUM_PORTS	5

#define CONFIG_DDR_SPD

#endif /* __CONFIG_K2HK_EVM_H */
