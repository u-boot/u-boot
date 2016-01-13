/*
 * Configuration header file for TI's k2g-evm
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_K2G_EVM_H
#define __CONFIG_K2G_EVM_H

/* Platform type */
#define CONFIG_SOC_K2G
#define CONFIG_K2G_EVM

/* U-Boot general configuration */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DEFAULT_MMC_TI_ARGS						\
	"console=ttyS0,115200n8\0"					\
	"bootpart=0:2\0"						\
	"bootdir=/boot\0"						\
	"addr_mon=0x0c040000\0"						\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=ubifs,2048\0"	\
	"name_fdt=k2g-evm.dtb\0"				\
	"name_mon=skern-k2g.bin\0"					\
	"name_ubi=k2g-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2g-evm.gph\0"				\
	"init_mmc=run args_all args_mmc\0"				\
	"get_fdt_mmc=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${name_fdt}\0"\
	"get_kern_mmc=load mmc ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_mon_mmc=load mmc ${bootpart} ${addr_mon} ${bootdir}/${name_mon}\0"\

#include <configs/ti_armv7_keystone2.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c080000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* Network */
#define CONFIG_KSNET_NETCP_V1_5
#define CONFIG_KSNET_CPSW_NUM_PORTS	2
#define CONFIG_KSNET_MDIO_PHY_CONFIG_ENABLE
#define CONFIG_PHY_MICREL

/* MMC/SD */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_CMD_MMC

#define CONFIG_SF_DEFAULT_BUS		1
#define CONFIG_SF_DEFAULT_CS		0

#endif /* __CONFIG_K2G_EVM_H */
