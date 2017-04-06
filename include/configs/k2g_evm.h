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

#include <environment/ti/mmc.h>
#include <environment/ti/spi.h>

/* Platform type */
#define CONFIG_SOC_K2G

/* U-Boot general configuration */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DEFAULT_MMC_TI_ARGS						\
	DEFAULT_PMMC_BOOT_ENV						\
	DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	"boot=mmc\0"							\
	"console=ttyS0,115200n8\0"					\
	"bootpart=0:2\0"						\
	"bootdir=/boot\0"						\
	"rd_spec=-\0"							\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=ubifs,2048\0"	\
	"name_fdt=keystone-k2g-evm.dtb\0"				\
	"name_mon=skern-k2g.bin\0"					\
	"name_ubi=k2g-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2g-evm.gph\0"				\
	"init_mmc=run args_all args_mmc\0"				\
	"init_fw_rd_mmc=load mmc ${bootpart} ${rdaddr} "		\
		"${bootdir}/${name_fw_rd}; run set_rd_spec\0"		\
	"soc_variant=k2g\0"						\
	"get_fdt_mmc=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${name_fdt}\0"\
	"get_kern_mmc=load mmc ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_mon_mmc=load mmc ${bootpart} ${addr_mon} ${bootdir}/${name_mon}\0"\
	"name_fs=arago-base-tisdk-image-k2g-evm.cpio\0"

#define CONFIG_BOOTCOMMAND						\
	"run envboot; "							\
	"run set_name_pmmc init_${boot} init_fw_rd_${boot} "		\
	"get_pmmc_${boot} run_pmmc get_mon_${boot} run_mon "		\
	"get_fdt_${boot} get_kern_${boot} run_kern"

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
#define PHY_ANEG_TIMEOUT	10000 /* PHY needs longer aneg time */

#undef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_IS_IN_FAT
#define FAT_ENV_INTERFACE		"mmc"
#define FAT_ENV_DEVICE_AND_PART		"0:1"
#define FAT_ENV_FILE			"uboot.env"

#define CONFIG_SF_DEFAULT_BUS		1
#define CONFIG_SF_DEFAULT_CS		0

#ifndef CONFIG_SPL_BUILD
#define CONFIG_CADENCE_QSPI
#define CONFIG_CQSPI_REF_CLK 384000000
#define CONFIG_CQSPI_DECODER 0x0
#define CONFIG_BOUNCE_BUFFER
#endif

#define SPI_MTD_PARTS	KEYSTONE_SPI1_MTD_PARTS
#endif /* __CONFIG_K2G_EVM_H */
