/*
 * Configuration header file for TI's k2l-evm
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_K2L_EVM_H
#define __CONFIG_K2L_EVM_H

/* Platform type */
#define CONFIG_SOC_K2L
#define CONFIG_K2L_EVM

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT		"K2L EVM # "

#define KS2_ARGS_UBI   "args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "\
		       "root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,4096\0"

#define KS2_FDT_NAME	"name_fdt=k2l-evm.dtb\0"
#define KS2_ADDR_MON	"addr_mon=0x0c140000\0"
#define KS2_NAME_MON	"name_mon=skern-k2l-evm.bin\0"
#define NAME_UBOOT	"name_uboot=u-boot-spi-k2l-evm.gph\0"
#define NAME_UBI	"name_ubi=k2l-evm-ubifs.ubi\0"

#include <configs/ks2_evm.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c100000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_4K

#endif /* __CONFIG_K2L_EVM_H */
