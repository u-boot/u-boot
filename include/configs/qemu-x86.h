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

#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE			SZ_256K

#define CONFIG_PREBOOT "pci enum"

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial,i8042-kbd\0" \
					"stdout=serial,vidconsole\0" \
					"stderr=serial,vidconsole\0"

/*
 * ATA/SATA support for QEMU x86 targets
 *   - Only legacy IDE controller is supported for QEMU '-M pc' target
 *   - AHCI controller is supported for QEMU '-M q35' target
 */
#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_IDE_MAXDEVICE	4
#define CONFIG_SYS_ATA_BASE_ADDR	0
#define CONFIG_SYS_ATA_DATA_OFFSET	0
#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_ALT_OFFSET	0
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x1f0
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x170
#define CONFIG_ATAPI

#define CONFIG_SPL_BOARD_LOAD_IMAGE

#endif	/* __CONFIG_H */
