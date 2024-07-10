/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Bootmethod for Android
 *
 * Copyright (C) 2024 BayLibre, SAS
 * Written by Mattijs Korpershoek <mkorpershoek@baylibre.com>
 */

enum android_boot_mode {
	ANDROID_BOOT_MODE_NORMAL = 0,

	/*
	 * Android "recovery" is a special boot mode that uses another ramdisk.
	 * It can be used to "factory reset" a board or to flash logical partitions
	 * It operates in 2 modes: adb or fastbootd
	 * To enter recovery from Android, we can do:
	 * $ adb reboot recovery
	 * $ adb reboot fastboot
	 */
	ANDROID_BOOT_MODE_RECOVERY,

	/*
	 * Android "bootloader" is for accessing/reflashing physical partitions
	 * Typically, this will launch a fastboot process in U-Boot.
	 * To enter "bootloader" from Android, we can do:
	 * $ adb reboot bootloader
	 */
	ANDROID_BOOT_MODE_BOOTLOADER,
};
