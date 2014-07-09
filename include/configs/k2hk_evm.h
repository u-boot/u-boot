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
#define CONFIG_K2HK_EVM

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT               "K2HK EVM # "

#define KS2_ARGS_UBI   "args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "\
		       "root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,2048\0"

#define KS2_FDT_NAME   "name_fdt=k2hk-evm.dtb\0"
#define KS2_ADDR_MON   "addr_mon=0x0c5f0000\0"
#define KS2_NAME_MON   "name_mon=skern-k2hk-evm.bin\0"
#define NAME_UBOOT     "name_uboot=u-boot-spi-k2hk-evm.gph\0"
#define NAME_UBI       "name_ubi=k2hk-evm-ubifs.ubi\0"

#include <configs/ks2_evm.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c200000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* Network */
#define CONFIG_DRIVER_TI_KEYSTONE_NET

#endif /* __CONFIG_K2HK_EVM_H */
