/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2020 Marek Vasut <marex@denx.de>
 *
 * Configuration settings for the DH STM32MP15x SoMs
 */

#ifndef __CONFIG_STM32MP15_DH_DHSOM_H__
#define __CONFIG_STM32MP15_DH_DHSOM_H__

/* PHY needs a longer autoneg timeout */

#ifdef CONFIG_XPL_BUILD
#define CFG_EXTRA_ENV_SETTINGS						\
	"dfu_alt_info_ram=u-boot.itb ram "				\
			__stringify(CONFIG_SPL_LOAD_FIT_ADDRESS)	\
			" 0x800000\0"
#endif

/* Add the search for AB partitons */
#define SCAN_DEV_FOR_BOOT_PARTS						\
	"run dh_check_if_ab; "						\
	"if test -z \"${devplist}\"; "					\
		"then "							\
		"part list ${devtype} ${devnum} -bootable devplist; "	\
	"fi; "

#define STM32MP_BOARD_EXTRA_ENV						\
	"altbootcmd= "							\
	"setenv dh_ab_get_partnames "					\
		"'setenv dh_ab_partnames ${dh_ab_partname_secondary} "	\
			"${dh_ab_partname_primary}' && "		\
		"run bootcmd\0"						\
	"dh_check_if_ab= " /* Sets devplist if AB partitions*/		\
		"echo test for AB on ${devtype} ${devnum} && "		\
		"run dh_ab_get_partnames && "				\
		"setenv devplist && "					\
		"for partname in ${dh_ab_partnames}; do "		\
			"setenv partnum && "				\
			"if part number ${devtype} ${devnum} ${partname} partnum; "\
				"then "					\
				"setenv devplist \"${devplist} ${partnum}\" && "\
				"setenv bootretry 60 ;"			\
			"fi; "						\
		"done ; "						\
		"if test -n \"${devplist}\"; "				\
			"then echo AB partitions found! ; "		\
		"fi\0"							\
	"dh_ab_get_partnames= " /* Sets dh_ab_partnames */		\
		"setenv dh_ab_partnames ${dh_ab_partname_primary} "	\
			"${dh_ab_partname_secondary}\0"			\
	"dh_ab_partname_primary=rootfs-a\0" /* Names of AB partitions */\
	"dh_ab_partname_secondary=rootfs-b\0"				\
	"dh_preboot="							\
		"run dh_testbench_backward_compat\0"			\
	"dh_update_sd_to_emmc=" /* Install U-Boot from SD to eMMC */	\
		"setexpr loadaddr1 ${loadaddr} + 0x1000000 && "		\
		"load mmc 0:4 ${loadaddr1} boot/u-boot-spl.stm32 && "	\
		"setexpr sblkcnt ${filesize} + 0x1ff && "		\
		"setexpr sblkcnt ${sblkcnt} / 0x200 && "		\
		"load mmc 0:4 ${loadaddr} boot/u-boot.itb && "		\
		"setexpr ublkcnt ${filesize} + 0x1ff && "		\
		"setexpr ublkcnt ${ublkcnt} / 0x200 && "		\
		"mmc partconf 1 1 1 1 && mmc dev 1 1 && "		\
		"mmc write ${loadaddr1} 0 ${sblkcnt} && "		\
		"mmc dev 1 2 && "					\
		"mmc write ${loadaddr1} 0 ${sblkcnt} && "		\
		"mmc dev 1 && "						\
		"gpt write mmc 1 'name=ssbl,size=2MiB' && "		\
		"mmc write ${loadaddr} 0x22 ${ublkcnt} && "		\
		"mmc partconf 1 1 1 0 && "				\
		"setenv loadaddr1 && "					\
		"setenv sblkcnt && "					\
		"setenv ublkcnt\0"					\
	"dh_update_block_to_sf=" /* Erase SPI NOR and install U-Boot from block device */ \
		"setexpr loadaddr1 ${loadaddr} + 0x1000000 && "		\
		"load ${dh_update_iface} ${dh_update_dev} "		\
			"${loadaddr1} /boot/u-boot-spl.stm32 && "	\
		"env set filesize1 ${filesize} && "			\
		"load ${dh_update_iface} ${dh_update_dev} "		\
			"${loadaddr} /boot/u-boot.itb && "		\
		"sf probe && sf erase 0 0x200000 && "			\
		"sf update ${loadaddr1} 0 ${filesize1} && "		\
		"sf update ${loadaddr1} 0x40000 ${filesize1} && "	\
		"sf update ${loadaddr} 0x80000 ${filesize} && "		\
		"env set filesize1 && env set loadaddr1\0"		\
	"dh_update_sd_to_sf=" /* Erase SPI NOR and install U-Boot from SD */ \
		"setenv dh_update_iface mmc && "			\
		"setenv dh_update_dev 0:4 && "				\
		"run dh_update_block_to_sf\0"				\
	"dh_update_emmc_to_sf=" /* Erase SPI NOR and install U-Boot from eMMC */ \
		"setenv dh_update_iface mmc && "			\
		"setenv dh_update_dev 1:4 && "				\
		"run dh_update_block_to_sf\0"				\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"stderr=serial\0"						\
	"update_sf=run dh_update_sd_to_sf\0"				\
	"usb_pgood_delay=1000\0"					\
	/* Old testbench-only backward compatibility environment */	\
	"dh_testbench_backward_compat="					\
		"test ${board_name} = \"dh,stm32mp15xx-dhcor-testbench\" && " \
		"run load_bootenv importbootenv\0"			\
	"importbootenv="						\
		"echo Importing environment from DHupdate.ini...;"	\
		"env import -t ${loadaddr} ${filesize}\0"		\
	"load_bootenv="							\
		"usb reset && "						\
		"load usb ${usbdev}:${usbpart} ${loadaddr} DHupdate.ini && " \
		"echo \"--> Update: found DHupdate.ini (${filesize} bytes)\"\0" \
	"usbdev=0\0"							\
	"usbpart=1\0"

#include <configs/stm32mp15_common.h>

#endif
