/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM642 SoC family
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 */

#ifndef __CONFIG_AM642_EVM_H
#define __CONFIG_AM642_EVM_H

#include <linux/sizes.h>
#include <config_distro_bootcmd.h>
#include <environment/ti/mmc.h>
#include <asm/arch/am64_hardware.h>
#include <environment/ti/k3_dfu.h>

/* DDR Configuration */
#define CONFIG_SYS_SDRAM_BASE1		0x880000000

#define PARTS_DEFAULT \
	/* Linux partitions */ \
	"name=rootfs,start=0,size=-,uuid=${uuid_gpt_rootfs}\0"

/* U-Boot general configuration */
#define EXTRA_ENV_AM642_BOARD_SETTINGS					\
	"findfdt="							\
		"if test $board_name = am64x_gpevm; then " \
			"setenv fdtfile k3-am642-evm.dtb; fi; " \
		"if test $board_name = am64x_skevm; then " \
			"setenv fdtfile k3-am642-sk.dtb; fi;" \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine device tree to use; fi; \0" \
	"name_kern=Image\0"						\
	"console=ttyS2,115200n8\0"					\
	"args_all=setenv optargs earlycon=ns16550a,mmio32,0x02800000 "	\
		"${mtdparts}\0"						\
	"run_kern=booti ${loadaddr} ${rd_spec} ${fdtaddr}\0"

/* U-Boot MMC-specific configuration */
#define EXTRA_ENV_AM642_BOARD_SETTINGS_MMC				\
	"boot=mmc\0"							\
	"mmcdev=1\0"							\
	"bootpart=1:2\0"						\
	"bootdir=/boot\0"						\
	"rd_spec=-\0"							\
	"init_mmc=run args_all args_mmc\0"				\
	"get_fdt_mmc=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
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

#define EXTRA_ENV_AM642_BOARD_SETTING_USBMSC				\
	"args_usb=run finduuid;setenv bootargs console=${console} "	\
		"${optargs} "						\
		"root=PARTUUID=${uuid} rw "				\
		"rootfstype=${mmcrootfstype}\0"				\
	"init_usb=run args_all args_usb\0"				\
	"get_fdt_usb=load usb ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"get_overlay_usb="						\
		"fdt address ${fdtaddr};"				\
		"fdt resize 0x100000;"					\
		"for overlay in $name_overlays;"			\
		"do;"							\
		"load usb ${bootpart} ${dtboaddr} ${bootdir}/${overlay} && "	\
		"fdt apply ${dtboaddr};"				\
		"done;\0"						\
	"get_kern_usb=load usb ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_fit_usb=load usb ${bootpart} ${addr_fit} "			\
		"${bootdir}/${name_fit}\0"				\
	"usbboot=setenv boot usb;"					\
		"setenv bootpart 0:2;"					\
		"usb start;"						\
		"run findfdt;"						\
		"run init_usb;"						\
		"run get_kern_usb;"					\
		"run get_fdt_usb;"					\
		"run run_kern\0"

#define EXTRA_ENV_DFUARGS \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_OSPI

/* Incorporate settings into the U-Boot environment */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	DEFAULT_LINUX_BOOT_ENV						\
	DEFAULT_MMC_TI_ARGS						\
	EXTRA_ENV_AM642_BOARD_SETTINGS					\
	EXTRA_ENV_AM642_BOARD_SETTINGS_MMC				\
	EXTRA_ENV_DFUARGS						\
	EXTRA_ENV_AM642_BOARD_SETTING_USBMSC

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM642_EVM_H */
