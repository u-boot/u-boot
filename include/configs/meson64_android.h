/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Android Amlogic Meson 64bits SoCs
 *
 * Copyright (C) 2019 Baylibre, SAS
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __MESON64_ANDROID_CONFIG_H
#define __MESON64_ANDROID_CONFIG_H

#include <linux/sizes.h>

#define CONFIG_SYS_MALLOC_LEN	       SZ_128M

#ifndef BOOT_PARTITION
#define BOOT_PARTITION "boot"
#endif

#ifndef LOGO_PARTITION
#define LOGO_PARTITION "logo"
#endif

#ifndef CONTROL_PARTITION
#define CONTROL_PARTITION "misc"
#endif

#if defined(CONFIG_CMD_AVB)
#define AVB_VERIFY_CHECK \
	"if test \"${force_avb}\" -eq 1; then " \
		"if run avb_verify; then " \
			"echo AVB verification OK.;" \
			"setenv bootargs \"$bootargs $avb_bootargs\";" \
		"else " \
			"echo AVB verification failed.;" \
		"exit; fi;" \
	"else " \
		"setenv bootargs \"$bootargs androidboot.verifiedbootstate=orange\";" \
		"echo Running without AVB...; "\
	"fi;"

#define AVB_VERIFY_CMD "avb_verify=avb init ${mmcdev}; avb verify $slot_suffix;\0"
#else
#define AVB_VERIFY_CHECK ""
#define AVB_VERIFY_CMD ""
#endif

#if defined(CONFIG_CMD_AB_SELECT)
#define ANDROIDBOOT_GET_CURRENT_SLOT_CMD "get_current_slot=" \
	"if part number mmc ${mmcdev} " CONTROL_PARTITION " control_part_number; " \
	"then " \
		"echo " CONTROL_PARTITION \
			" partition number:${control_part_number};" \
		"ab_select current_slot mmc ${mmcdev}:${control_part_number};" \
	"else " \
		"echo " CONTROL_PARTITION " partition not found;" \
	"fi;\0"

#define AB_SELECT_SLOT \
	"run get_current_slot; " \
	"if test -e \"${current_slot}\"; " \
	"then " \
		"setenv slot_suffix _${current_slot}; " \
	"else " \
		"echo current_slot not found;" \
		"exit;" \
	"fi;"

#define AB_SELECT_ARGS \
	"setenv bootargs_ab androidboot.slot_suffix=${slot_suffix}; " \
	"echo A/B cmdline addition: ${bootargs_ab};" \
	"setenv bootargs ${bootargs} ${bootargs_ab};"

#define AB_BOOTARGS " androidboot.force_normal_boot=1"
#define RECOVERY_PARTITION "boot"
#else
#define AB_SELECT_SLOT ""
#define AB_SELECT_ARGS " "
#define ANDROIDBOOT_GET_CURRENT_SLOT_CMD ""
#define AB_BOOTARGS " "
#define RECOVERY_PARTITION "recovery"
#endif

#define BOOT_CMD "bootm ${loadaddr};"

#define BOOTENV_DEV_FASTBOOT(devtypeu, devtypel, instance) \
	"bootcmd_fastboot=" \
		"setenv run_fastboot 0;" \
		"if test \"${boot_source}\" = \"usb\"; then " \
			"echo Fastboot forced by usb rom boot;" \
			"setenv run_fastboot 1;" \
		"fi;" \
		"if gpt verify mmc ${mmcdev} ${partitions}; then; " \
		"else " \
			"echo Broken MMC partition scheme;" \
			"setenv run_fastboot 1;" \
		"fi;" \
		"if bcb load " __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) " " \
		CONTROL_PARTITION "; then " \
			"if bcb test command = bootonce-bootloader; then " \
				"echo BCB: Bootloader boot...; " \
				"bcb clear command; bcb store; " \
				"setenv run_fastboot 1;" \
			"fi; " \
			"if bcb test command = boot-fastboot; then " \
				"echo BCB: fastboot userspace boot...; " \
				"setenv force_recovery 1;" \
			"fi; " \
		"else " \
			"echo Warning: BCB is corrupted or does not exist; " \
		"fi;" \
		"if test \"${run_fastboot}\" -eq 1; then " \
			"echo Running Fastboot...;" \
			"fastboot " __stringify(CONFIG_FASTBOOT_USB_DEV) "; " \
		"fi\0"

#define BOOTENV_DEV_NAME_FASTBOOT(devtypeu, devtypel, instance)	\
		"fastboot "

#define BOOTENV_DEV_RECOVERY(devtypeu, devtypel, instance) \
	"bootcmd_recovery=" \
		"pinmux dev pinctrl@14;" \
		"pinmux dev pinctrl@40;" \
		"setenv run_recovery 0;" \
		"if run check_button; then " \
			"echo Recovery button is pressed;" \
			"setenv run_recovery 1;" \
		"fi; " \
		"if bcb load " __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) " " \
		CONTROL_PARTITION "; then " \
			"if bcb test command = boot-recovery; then " \
				"echo BCB: Recovery boot...; " \
				"setenv run_recovery 1;" \
			"fi;" \
		"else " \
			"echo Warning: BCB is corrupted or does not exist; " \
		"fi;" \
		"if test \"${skip_recovery}\" -eq 1; then " \
			"echo Recovery skipped by environment;" \
			"setenv run_recovery 0;" \
		"fi;" \
		"if test \"${force_recovery}\" -eq 1; then " \
			"echo Recovery forced by environment;" \
			"setenv run_recovery 1;" \
		"fi;" \
		"if test \"${run_recovery}\" -eq 1; then " \
			"echo Running Recovery...;" \
			"mmc dev ${mmcdev};" \
			"setenv bootargs \"${bootargs} androidboot.serialno=${serial#}\";" \
			AB_SELECT_SLOT \
			AB_SELECT_ARGS \
			AVB_VERIFY_CHECK \
			"part start mmc ${mmcdev} " RECOVERY_PARTITION "${slot_suffix} boot_start;" \
			"part size mmc ${mmcdev} " RECOVERY_PARTITION "${slot_suffix} boot_size;" \
			"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
				"echo Running Android Recovery...;" \
				BOOT_CMD \
			"fi;" \
			"echo Failed to boot Android...;" \
			"reset;" \
		"fi\0"

#define BOOTENV_DEV_NAME_RECOVERY(devtypeu, devtypel, instance)	\
		"recovery "

#define BOOTENV_DEV_SYSTEM(devtypeu, devtypel, instance) \
	"bootcmd_system=" \
		"echo Loading Android " BOOT_PARTITION " partition...;" \
		"mmc dev ${mmcdev};" \
		"setenv bootargs ${bootargs} androidboot.serialno=${serial#};" \
		AB_SELECT_SLOT \
		AB_SELECT_ARGS \
		AVB_VERIFY_CHECK \
		"part start mmc ${mmcdev} " BOOT_PARTITION "${slot_suffix} boot_start;" \
		"part size mmc ${mmcdev} " BOOT_PARTITION "${slot_suffix} boot_size;" \
		"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
			"setenv bootargs \"${bootargs} " AB_BOOTARGS "\"  ; " \
			"echo Running Android...;" \
			BOOT_CMD \
		"fi;" \
		"echo Failed to boot Android...;" \
		"reset\0"

#define BOOTENV_DEV_NAME_SYSTEM(devtypeu, devtypel, instance)	\
		"system "

#define BOOT_TARGET_DEVICES(func) \
	func(FASTBOOT, fastboot, na) \
	func(RECOVERY, recovery, na) \
	func(SYSTEM, system, na) \

#define PREBOOT_LOAD_LOGO \
	"if test \"${boot_source}\" != \"usb\" && " \
		"gpt verify mmc ${mmcdev} ${partitions}; then; " \
		"mmc dev ${mmcdev};" \
		"part start mmc ${mmcdev} " LOGO_PARTITION " boot_start;" \
		"part size mmc ${mmcdev} " LOGO_PARTITION " boot_size;" \
		"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
			"bmp display ${loadaddr} m m;" \
		"fi;" \
	"fi;"

#define CONFIG_EXTRA_ENV_SETTINGS                                     \
	"partitions=" PARTS_DEFAULT "\0"                              \
	"mmcdev=2\0"                                                  \
	ANDROIDBOOT_GET_CURRENT_SLOT_CMD                              \
	AVB_VERIFY_CMD                                                \
	"force_avb=0\0"                                               \
	"gpio_recovery=88\0"                                          \
	"check_button=gpio input ${gpio_recovery};test $? -eq 0;\0"   \
	"load_logo=" PREBOOT_LOAD_LOGO "\0"			      \
	"stdin=" STDIN_CFG "\0"                                       \
	"stdout=" STDOUT_CFG "\0"                                     \
	"stderr=" STDOUT_CFG "\0"                                     \
	"loadaddr=0x01000000\0"                                       \
	"fdt_addr_r=0x01000000\0"                                     \
	"scriptaddr=0x08000000\0"                                     \
	"kernel_addr_r=0x01080000\0"                                  \
	"pxefile_addr_r=0x01080000\0"                                 \
	"ramdisk_addr_r=0x13000000\0"                                 \
	"fdtfile=amlogic/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"        \
	BOOTENV

#include <configs/meson64.h>

#endif /* __MESON64_ANDROID_CONFIG_H */
