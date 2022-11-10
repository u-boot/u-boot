/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(VIRTIO, virtio, 0) \
	func(IDE, ide, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#include <configs/x86-common.h>

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial,i8042-kbd\0" \
					"stdout=serial,vidconsole\0" \
					"stderr=serial,vidconsole\0"

/*
 * ATA/SATA support for QEMU x86 targets
 *   - Only legacy IDE controller is supported for QEMU '-M pc' target
 *   - AHCI controller is supported for QEMU '-M q35' target
 */

#endif	/* __CONFIG_H */
