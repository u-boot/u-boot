/*
 * Configuration header file for TI's k2e-evm
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_K2E_EVM_H
#define __CONFIG_K2E_EVM_H

/* Platform type */
#define CONFIG_SOC_K2E
#define CONFIG_K2E_EVM

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT               "K2E EVM # "

#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	"addr_mon=0x0c140000\0"						\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,2048\0"		\
	"name_fdt=uImage-k2e-evm.dtb\0"					\
	"name_mon=skern-k2e-evm.bin\0"					\
	"name_ubi=k2e-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2e-evm.gph\0"				\
	"name_fs=arago-console-image-k2e-evm.cpio.gz\0"

#include <configs/ks2_evm.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE           0x0c100000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* Network */
#define CONFIG_KSNET_NETCP_V1_5
#define CONFIG_KSNET_CPSW_NUM_PORTS	9
#define CONFIG_KSNET_MDIO_PHY_CONFIG_ENABLE

#endif /* __CONFIG_K2E_EVM_H */
