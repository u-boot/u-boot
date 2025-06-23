/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2011-2012
 * Gerald Kerma <dreagle@doukki.net>
 * Luka Perkov <luka@openwrt.org>
 */

#ifndef _CONFIG_IB62x0_H
#define _CONFIG_IB62x0_H

#include "mv-common.h"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#define EXTRA_ENV_SETTINGS_LEGACY \
	"console=console=ttyS0,115200\0"				\
	"kernel=/boot/zImage\0"						\
	"fdt=/boot/ib62x0.dtb\0"					\
	"bootargs_root=ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw\0"

#define KERNEL_ADDR_R	__stringify(0x800000)
#define FDT_ADDR_R	__stringify(0x2c00000)
#define RAMDISK_ADDR_R	__stringify(0x01100000)
#define SCRIPT_ADDR_R	__stringify(0x200000)

#define LOAD_ADDRESS_ENV_SETTINGS \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0"

#define CFG_EXTRA_ENV_SETTINGS \
	EXTRA_ENV_SETTINGS_LEGACY \
	LOAD_ADDRESS_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"

/*
 * SATA driver configuration
 */
#ifdef CONFIG_IDE
#define __io
#endif /* CONFIG_IDE */

#endif /* _CONFIG_IB62x0_H */
