/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Allied Telesis
 */

#ifndef __X240_H_
#define __X240_H_

#include <asm/arch/soc.h>

/* additions for new ARM relocation support */
#define CFG_SYS_SDRAM_BASE   0x200000000

/* Default Env vars */
#define BOOT_TARGETS "usb dhcp"

#define CFG_EXTRA_ENV_SETTINGS   \
	"kernel_addr_r=0x202000000\0" \
	"fdt_addr_r=0x201000000\0"    \
	"ramdisk_addr_r=0x206000000\0"    \
	"boot_targets=" BOOT_TARGETS "\0" \
	"fdtfile=marvell/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"

/*
 * High Level Configuration Options (easy to change)
 */
#define CFG_SYS_TCLK     325000000

#endif /* __X240_H_ */
