/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Allied Telesis
 */

#ifndef __X250_H_
#define __X250_H_

/*
 * High Level Configuration Options (easy to change)
 */
#define CFG_SYS_TCLK		250000000	/* 250MHz */

/* additions for new ARM relocation support */
#define CFG_SYS_SDRAM_BASE	0x00000000

#define BOOT_TARGETS	"usb scsi pxe dhcp"

#define CFG_EXTRA_ENV_SETTINGS \
	"scriptaddr=0x6d00000\0"        \
	"pxefile_addr_r=0x6e00000\0"    \
	"fdt_addr_r=0x6f00000\0"        \
	"kernel_addr_r=0x7000000\0"     \
	"ramdisk_addr_r=0xa000000\0"    \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"boot_targets=" BOOT_TARGETS "\0"

#endif /* __X250_H_ */
