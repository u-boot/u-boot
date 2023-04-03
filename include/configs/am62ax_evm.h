/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM62Ax SoC family
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_AM62AX_EVM_H
#define __CONFIG_AM62AX_EVM_H

#include <linux/sizes.h>
#include <environment/ti/mmc.h>
#include <environment/ti/k3_dfu.h>

/* DDR Configuration */
#define CFG_SYS_SDRAM_BASE1		0x880000000

#define PARTS_DEFAULT \
	/* Linux partitions */ \
	"name=rootfs,start=0,size=-,uuid=${uuid_gpt_rootfs}\0" \

/* U-Boot general configuration */
#define EXTRA_ENV_AM62A7_BOARD_SETTINGS					\
	"default_device_tree=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"	\
	"findfdt="							\
		"setenv name_fdt ${default_device_tree};"		\
		"setenv fdtfile ${name_fdt}\0"				\
	"name_kern=Image\0"						\
	"console=ttyS2,115200n8\0"					\
	"args_all=setenv optargs earlycon=ns16550a,mmio32,0x02800000 "	\
		"${mtdparts}\0"						\
	"run_kern=booti ${loadaddr} ${rd_spec} ${fdtaddr}\0"

/* U-Boot MMC-specific configuration */
#define EXTRA_ENV_AM62A7_BOARD_SETTINGS_MMC				\
	"boot=mmc\0"							\
	"mmcdev=1\0"							\
	"bootpart=1:2\0"						\
	"bootdir=/boot\0"						\
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
	"get_kern_mmc=load mmc ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_fit_mmc=load mmc ${bootpart} ${addr_fit} "			\
		"${bootdir}/${name_fit}\0"				\
	"partitions=" PARTS_DEFAULT

#define BOOTENV_DEV_TI_MMC(devtypeu, devtypel, instance)		\
	DEFAULT_MMC_TI_ARGS						\
	EXTRA_ENV_AM62A7_BOARD_SETTINGS_MMC				\
	"bootcmd_ti_mmc="						\
		"run findfdt; run envboot; run init_mmc;"		\
		"if test ${boot_fit} -eq 1; then;"			\
			"run get_fit_mmc; run get_overlaystring;"	\
			"run run_fit;"					\
		"else;"							\
			"run get_kern_mmc; run get_fdt_mmc;"		\
			"run get_overlay_mmc;"				\
			"run run_kern;"					\
		"fi;\0"

#define BOOTENV_DEV_NAME_TI_MMC(devtyeu, devtypel, instance)		\
	"ti_mmc "

#if IS_ENABLED(CONFIG_CMD_MMC)
	#define BOOT_TARGET_MMC(func)					\
		func(TI_MMC, ti_mmc, na)
#else
	#define BOOT_TARGET_MMC(func)
#endif /* IS_ENABLED(CONFIG_CMD_MMC) */

#define BOOT_TARGET_DEVICES(func)					\
	BOOT_TARGET_MMC(func)

#include <config_distro_bootcmd.h>

/* Incorporate settings into the U-Boot environment */
#define CFG_EXTRA_ENV_SETTINGS					\
	BOOTENV

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM62A7_EVM_H */
