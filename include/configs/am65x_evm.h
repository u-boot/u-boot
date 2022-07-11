/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM654 EVM
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __CONFIG_AM654_EVM_H
#define __CONFIG_AM654_EVM_H

#include <linux/sizes.h>
#include <environment/ti/mmc.h>
#include <environment/ti/k3_rproc.h>
#include <environment/ti/k3_dfu.h>

/* DDR Configuration */
#define CONFIG_SYS_SDRAM_BASE1		0x880000000

#define PARTS_DEFAULT \
	/* Linux partitions */ \
	"name=rootfs,start=0,size=-,uuid=${uuid_gpt_rootfs}\0"

/* U-Boot general configuration */
#define EXTRA_ENV_AM65X_BOARD_SETTINGS					\
	"findfdt="							\
		"setenv name_fdt k3-am654-base-board.dtb;"		\
		"setenv fdtfile ${name_fdt}\0"				\
	"name_kern=Image\0"						\
	"console=ttyS2,115200n8\0"					\
	"stdin=serial,usbkbd\0"						\
	"args_all=setenv optargs earlycon=ns16550a,mmio32,0x02800000 "  \
		"${mtdparts}\0"						\
	"run_kern=booti ${loadaddr} ${rd_spec} ${fdtaddr}\0"		\

/* U-Boot MMC-specific configuration */
#define EXTRA_ENV_AM65X_BOARD_SETTINGS_MMC				\
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
		"load mmc ${bootpart} ${dtboaddr} ${bootdir}/${overlay};"	\
		"fdt apply ${dtboaddr};"				\
		"done;\0"						\
	"get_kern_mmc=load mmc ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_fit_mmc=load mmc ${bootpart} ${addr_fit} "			\
		"${bootdir}/${name_fit}\0"				\
	"partitions=" PARTS_DEFAULT

#ifdef DEFAULT_RPROCS
#undef DEFAULT_RPROCS
#endif
#define DEFAULT_RPROCS	""						\
		"0 /lib/firmware/am65x-mcu-r5f0_0-fw "			\
		"1 /lib/firmware/am65x-mcu-r5f0_1-fw "

#define EXTRA_ENV_AM65X_BOARD_SETTINGS_UBI				\
	"init_ubi=run args_all args_ubi; sf probe; "			\
		"ubi part ospi.rootfs; ubifsmount ubi:rootfs;\0"	\
	"get_kern_ubi=ubifsload ${loadaddr} ${bootdir}/${name_kern}\0"	\
	"get_fdt_ubi=ubifsload ${fdtaddr} ${bootdir}/${name_fdt}\0"	\
	"args_ubi=setenv bootargs console=${console} ${optargs} "	\
		"rootfstype=ubifs root=ubi0:rootfs rw ubi.mtd=ospi.rootfs\0"

#define EXTRA_ENV_DFUARGS						\
	DFU_ALT_INFO_MMC						\
	DFU_ALT_INFO_RAM						\
	DFU_ALT_INFO_EMMC						\
	DFU_ALT_INFO_OSPI

#ifdef CONFIG_TARGET_AM654_A53_EVM
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/* Incorporate settings into the U-Boot environment */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	DEFAULT_LINUX_BOOT_ENV						\
	DEFAULT_MMC_TI_ARGS						\
	DEFAULT_FIT_TI_ARGS						\
	EXTRA_ENV_AM65X_BOARD_SETTINGS					\
	EXTRA_ENV_AM65X_BOARD_SETTINGS_MMC				\
	EXTRA_ENV_AM65X_BOARD_SETTINGS_UBI				\
	EXTRA_ENV_RPROC_SETTINGS					\
	EXTRA_ENV_DFUARGS						\
	BOOTENV

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM654_EVM_H */
