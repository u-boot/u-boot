/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for the SEI510
 *
 * Copyright (C) 2019 Baylibre, SAS
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_MMC_ENV_DEV	2
#define CONFIG_SYS_MMC_ENV_PART	1
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_OFFSET	(-0x10000)

#define CACHE_UUID "99207ae6-5207-11e9-999e-6f77a3612069;"
#define SYSTEM_UUID "99f9b7ac-5207-11e9-8507-c3c037e393f3;"
#define VENDOR_UUID "9d082802-5207-11e9-954c-cbbce08ba108;"
#define USERDATA_UUID "9b976e42-5207-11e9-8f16-ff47ac594b22;"
#define ROOT_UUID "ddb8c3f6-d94d-4394-b633-3134139cc2e0;"

#define PARTS_DEFAULT                                        \
	"uuid_disk=${uuid_gpt_disk};"  			\
	"name=boot,size=64M,bootable,uuid=${uuid_gpt_boot};" \
	"name=cache,size=256M,uuid=" CACHE_UUID             \
	"name=system,size=1536M,uuid=" SYSTEM_UUID           \
	"name=vendor,size=256M,uuid=" VENDOR_UUID            \
	"name=userdata,size=4746M,uuid=" USERDATA_UUID	\
	"name=rootfs,size=-,uuid=" ROOT_UUID

#define BOOTENV_DEV_FASTBOOT(devtypeu, devtypel, instance) \
	"bootcmd_fastboot=" \
		"sm reboot_reason reason;" \
		"setenv run_fastboot 0;" \
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
	func(ROMUSB, romusb, na)  \
	func(FASTBOOT, fastboot, na) \
	func(RECOVERY, recovery, na) \
	func(SYSTEM, system, na) \

#define CONFIG_EXTRA_ENV_SETTINGS                                     \
	"partitions=" PARTS_DEFAULT "\0"                              \
	"mmcdev=2\0"                                                  \
	"bootpart=1\0"                                                \
	"gpio_recovery=88\0"                                          \
	"check_button=gpio input ${gpio_recovery};test $? -eq 0;\0"   \
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

#endif /* __CONFIG_H */
