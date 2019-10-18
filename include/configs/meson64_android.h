/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Android Amlogic Meson 64bits SoCs
 *
 * Copyright (C) 2019 Baylibre, SAS
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __MESON64_ANDROID_CONFIG_H
#define __MESON64_ANDROID_CONFIG_H

#define CONFIG_SYS_MMC_ENV_DEV	2
#define CONFIG_SYS_MMC_ENV_PART	1
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_OFFSET	(-0x10000)


#define BOOTENV_DEV_FASTBOOT(devtypeu, devtypel, instance) \
	"bootcmd_fastboot=" \
		"sm reboot_reason reason;" \
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
		"if test \"${reason}\" = \"bootloader\" -o " \
			"\"${reason}\" = \"fastboot\"; then " \
			"echo Fastboot asked by reboot reason;" \
			"setenv run_fastboot 1;" \
		"fi;" \
		"if test \"${skip_fastboot}\" -eq 1; then " \
			"echo Fastboot skipped by environment;" \
			"setenv run_fastboot 0;" \
		"fi;" \
		"if test \"${force_fastboot}\" -eq 1; then " \
			"echo Fastboot forced by environment;" \
			"setenv run_fastboot 1;" \
		"fi;" \
		"if test \"${run_fastboot}\" -eq 1; then " \
			"echo Running Fastboot...;" \
			"fastboot 0;" \
		"fi\0"

#define BOOTENV_DEV_NAME_FASTBOOT(devtypeu, devtypel, instance)	\
		"fastboot "

/* TOFIX: Run actual recovery instead of fastboot */
#define BOOTENV_DEV_RECOVERY(devtypeu, devtypel, instance) \
	"bootcmd_recovery=" \
		"pinmux dev pinctrl@14;" \
		"pinmux dev pinctrl@40;" \
		"sm reboot_reason reason;" \
		"setenv run_recovery 0;" \
		"if run check_button; then " \
			"echo Recovery button is pressed;" \
			"setenv run_recovery 1;" \
		"elif test \"${reason}\" = \"recovery\" -o " \
			  "\"${reason}\" = \"update\"; then " \
			"echo Recovery asked by reboot reason;" \
			"setenv run_recovery 1;" \
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
			"fastboot 0;" \
		"fi\0"

#define BOOTENV_DEV_NAME_RECOVERY(devtypeu, devtypel, instance)	\
		"recovery "

#define BOOTENV_DEV_SYSTEM(devtypeu, devtypel, instance) \
	"bootcmd_system=" \
		"echo Loading Android boot partition...;" \
		"mmc dev ${mmcdev};" \
		"setenv bootargs ${bootargs} console=${console} androidboot.serialno=${serial#};" \
		"part start mmc ${mmcdev} ${bootpart} boot_start;" \
		"part size mmc ${mmcdev} ${bootpart} boot_size;" \
		"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
			"echo Running Android...;" \
			"bootm ${loadaddr};" \
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
	"mmc dev ${mmcdev};" \
	"part start mmc ${mmcdev} ${logopart} boot_start;" \
	"part size mmc ${mmcdev} ${logopart} boot_size;" \
	"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
			"bmp display ${loadaddr} m m;" \
	"fi;"

#define CONFIG_EXTRA_ENV_SETTINGS                                     \
	"partitions=" PARTS_DEFAULT "\0"                              \
	"mmcdev=2\0"                                                  \
	"bootpart=1\0"                                                \
	"logopart=2\0"                                                \
	"gpio_recovery=88\0"                                          \
	"check_button=gpio input ${gpio_recovery};test $? -eq 0;\0"   \
	"load_logo=" PREBOOT_LOAD_LOGO "\0"			      \
	"console=/dev/ttyAML0\0"                                      \
	"bootargs=no_console_suspend\0"                               \
	"stdin=" STDIN_CFG "\0"                                       \
	"stdout=" STDOUT_CFG "\0"                                     \
	"stderr=" STDOUT_CFG "\0"                                     \
	"loadaddr=0x01000000\0"                                       \
	"fdt_addr_r=0x01000000\0"                                     \
	"scriptaddr=0x08000000\0"                                     \
	"kernel_addr_r=0x01080000\0"                                  \
	"pxefile_addr_r=0x01080000\0"                                 \
	"ramdisk_addr_r=0x13000000\0"                                 \
	"fdtfile=amlogic/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" BOOTENV

#include <configs/meson64.h>

#endif /* __MESON64_ANDROID_CONFIG_H */
