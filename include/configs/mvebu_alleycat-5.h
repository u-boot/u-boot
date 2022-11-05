/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marvell International Ltd
 */

#ifndef _CONFIG_MVEBU_ALLEYCAY_5_H
#define _CONFIG_MVEBU_ALLEYCAY_5_H

#include <asm/arch/soc.h>

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE   0x200000000

#define CONFIG_SYS_BAUDRATE_TABLE   { 9600, 19200, 38400, 57600, \
				      115200, 230400, 460800, 921600 }

/* Default Env vars */
#define CONFIG_IPADDR           0.0.0.0 /* In order to cause an error */
#define CONFIG_SERVERIP         0.0.0.0 /* In order to cause an error */
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_GATEWAYIP        0.0.0.0
#define CONFIG_ROOTPATH                 "/srv/nfs/" /* Default Dir for NFS */

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS   \
	BOOTENV \
	"kernel_addr_r=0x202000000\0" \
	"fdt_addr_r=0x201000000\0"    \
	"ramdisk_addr_r=0x206000000\0"    \
	"fdtfile=marvell/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SYS_TCLK     325000000

#endif /* _CONFIG_MVEBU_ALLEYCAY_5_H */
