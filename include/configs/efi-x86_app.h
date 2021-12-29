/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#undef CONFIG_TPM_TIS_BASE_ADDRESS

/*
 * Select the output device: Put an 'x' prefix before one of these to disable it
 */

/*
 * Video output - can normally continue after exit_boot_services has been
 * called, since output to the display does not require EFI services at that
 * point. U-Boot sets up the console memory and does its own drawing.
 */
#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial\0" \
					"stdout=vidconsole\0" \
					"stderr=vidconsole\0"

/*
 * Serial output with no console. Run qemu with:
 *
 *    -display none -serial mon:stdio
 *
 * This will hang or fail to output on the console after exit_boot_services is
 * called.
 */
#define xCONFIG_STD_DEVICES_SETTINGS	"stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#undef CONFIG_BOOTCOMMAND

#define CONFIG_BOOTCOMMAND "part list efi 0; fatls efi 0:1"

#endif
