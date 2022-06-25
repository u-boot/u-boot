/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 J721S2 EVM
 *
 * Copyright (C) 2021 Texas Instruments Incorporated - https://www.ti.com/
 *	David Huang <d-huang@ti.com>
 */

#ifndef __CONFIG_J721S2_EVM_H
#define __CONFIG_J721S2_EVM_H

#include <linux/sizes.h>
#include <config_distro_bootcmd.h>
#include <environment/ti/mmc.h>
#include <environment/ti/k3_rproc.h>
#include <environment/ti/ufs.h>
#include <environment/ti/k3_dfu.h>

/* DDR Configuration */
#define CONFIG_SYS_SDRAM_BASE1		0x880000000

/* SPL Loader Configuration */
#if defined(CONFIG_TARGET_J721S2_A72_EVM) || defined(CONFIG_TARGET_J7200_A72_EVM)
#define CONFIG_SYS_UBOOT_BASE		0x50280000
/* Image load address in RAM for DFU boot*/
#else
#define CONFIG_SYS_UBOOT_BASE		0x50080000
#endif

/* U-Boot general configuration */
#define EXTRA_ENV_J721S2_BOARD_SETTINGS					\
	"default_device_tree=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"	\
	"findfdt="							\
		"setenv name_fdt ${default_device_tree};"		\
		"setenv fdtfile ${name_fdt}\0"				\
	"name_kern=Image\0"						\
	"console=ttyS2,115200n8\0"					\
	"args_all=setenv optargs earlycon=ns16550a,mmio32,0x02880000 "	\
		"${mtdparts}\0"						\
	"run_kern=booti ${loadaddr} ${rd_spec} ${fdtaddr}\0"

#define PARTS_DEFAULT \
	/* Linux partitions */ \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=rootfs,start=0,size=-,uuid=${uuid_gpt_rootfs}\0"

#ifdef CONFIG_SYS_K3_SPL_ATF
#if defined(CONFIG_TARGET_J721S2_R5_EVM)
#define EXTRA_ENV_R5_SPL_RPROC_FW_ARGS_MMC				\
	"addr_mcur5f0_0load=0x89000000\0"				\
	"name_mcur5f0_0fw=/lib/firmware/j7-mcu-r5f0_0-fw\0"
#elif defined(CONFIG_TARGET_J7200_R5_EVM)
#define EXTRA_ENV_R5_SPL_RPROC_FW_ARGS_MMC				\
	"addr_mcur5f0_0load=0x89000000\0"				\
	"name_mcur5f0_0fw=/lib/firmware/j7200-mcu-r5f0_0-fw\0"
#endif /* CONFIG_TARGET_J721S2_R5_EVM */
#else
#define EXTRA_ENV_R5_SPL_RPROC_FW_ARGS_MMC ""
#endif /* CONFIG_SYS_K3_SPL_ATF */

/* U-Boot MMC-specific configuration */
#define EXTRA_ENV_J721S2_BOARD_SETTINGS_MMC				\
	"boot=mmc\0"							\
	"mmcdev=1\0"							\
	"bootpart=1:2\0"						\
	"bootdir=/boot\0"						\
	EXTRA_ENV_R5_SPL_RPROC_FW_ARGS_MMC				\
	"rd_spec=-\0"							\
	"init_mmc=run args_all args_mmc\0"				\
	"get_fdt_mmc=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${name_fdt}\0" \
	"get_overlay_mmc="						\
		"fdt address ${fdtaddr};"				\
		"fdt resize 0x100000;"					\
		"for overlay in $name_overlays;"			\
		"do;"							\
		"load mmc ${bootpart} ${dtboaddr} ${bootdir}/${overlay} && "	\
		"fdt apply ${dtboaddr};"				\
		"done;\0"						\
	"partitions=" PARTS_DEFAULT					\
	"get_kern_mmc=load mmc ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_fit_mmc=load mmc ${bootpart} ${addr_fit} "			\
		"${bootdir}/${name_fit}\0"				\
	"partitions=" PARTS_DEFAULT

/* Set the default list of remote processors to boot */
#if defined(CONFIG_TARGET_J721S2_A72_EVM) || defined(CONFIG_TARGET_J7200_A72_EVM)
#ifdef DEFAULT_RPROCS
#undef DEFAULT_RPROCS
#endif
#endif

#ifdef CONFIG_TARGET_J721S2_A72_EVM
#define DEFAULT_RPROCS	""						\
		"2 /lib/firmware/j721s2-main-r5f0_0-fw "			\
		"3 /lib/firmware/j721s2-main-r5f0_1-fw "			\
		"4 /lib/firmware/j721s2-main-r5f1_0-fw "			\
		"5 /lib/firmware/j721s2-main-r5f1_1-fw "			\
		"6 /lib/firmware/j721s2-c71_0-fw "				\
		"7 /lib/firmware/j721s2-c71_1-fw "
#endif /* CONFIG_TARGET_J721S2_A72_EVM */

#ifdef CONFIG_TARGET_J7200_A72_EVM
#define EXTRA_ENV_CONFIG_MAIN_CPSW0_QSGMII_PHY				\
	"do_main_cpsw0_qsgmii_phyinit=1\0"				\
	"init_main_cpsw0_qsgmii_phy=gpio set gpio@22_17;"		\
		 "gpio clear gpio@22_16\0"				\
	"main_cpsw0_qsgmii_phyinit="					\
	"if test ${do_main_cpsw0_qsgmii_phyinit} -eq 1 && test ${dorprocboot} -eq 1 && " \
			"test ${boot} = mmc; then "			\
		"run init_main_cpsw0_qsgmii_phy;"			\
	"fi;\0"
#define DEFAULT_RPROCS ""						\
		"2 /lib/firmware/j7200-main-r5f0_0-fw "			\
		"3 /lib/firmware/j7200-main-r5f0_1-fw "
#endif /* CONFIG_TARGET_J7200_A72_EVM */

#ifndef EXTRA_ENV_CONFIG_MAIN_CPSW0_QSGMII_PHY
#define EXTRA_ENV_CONFIG_MAIN_CPSW0_QSGMII_PHY
#endif

/* set default dfu_bufsiz to 128KB (sector size of OSPI) */
#define EXTRA_ENV_DFUARGS \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_OSPI

/* Incorporate settings into the U-Boot environment */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	DEFAULT_LINUX_BOOT_ENV						\
	DEFAULT_MMC_TI_ARGS						\
	DEFAULT_FIT_TI_ARGS						\
	EXTRA_ENV_J721S2_BOARD_SETTINGS					\
	EXTRA_ENV_J721S2_BOARD_SETTINGS_MMC				\
	EXTRA_ENV_RPROC_SETTINGS					\
	EXTRA_ENV_DFUARGS						\
	DEFAULT_UFS_TI_ARGS						\
	EXTRA_ENV_CONFIG_MAIN_CPSW0_QSGMII_PHY

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

/* MMC ENV related defines */

#endif /* __CONFIG_J721S2_EVM_H */
