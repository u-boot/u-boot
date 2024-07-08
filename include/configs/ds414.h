/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_SYNOLOGY_DS414_H
#define _CONFIG_SYNOLOGY_DS414_H

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

#define CFG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE

/*
 * Memory layout while starting into the bin_hdr via the
 * BootROM:
 *
 * 0x4000.4000 - 0x4003.4000	headers space (192KiB)
 * 0x4000.4030			bin_hdr start address
 * 0x4003.4000 - 0x4004.7c00	BootROM memory allocations (15KiB)
 * 0x4007.fffc			BootROM stack top
 *
 * The address space between 0x4007.fffc and 0x400f.fff is not locked in
 * L2 cache thus cannot be used.
 */

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define RELOCATION_LIMITS_ENV_SETTINGS  \
	"fdt_high=0x10000000\0"         \
	"initrd_high=0x10000000\0"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#ifndef CONFIG_SPL_BUILD

#define KERNEL_ADDR_R	__stringify(0x1000000)
#define FDT_ADDR_R	__stringify(0x2000000)
#define RAMDISK_ADDR_R	__stringify(0x2200000)
#define SCRIPT_ADDR_R	__stringify(0x1800000)
#define PXEFILE_ADDR_R	__stringify(0x1900000)

#define EXTRA_ENV_SETTINGS_LEGACY \
	"bootargs_legacy=console=ttyS0,115200 ip=off initrd=0x8000040,8M " \
		"root=/dev/md0 rw syno_hw_version=DS414r1 ihd_num=4 netif_num=2 " \
		"flash_size=8 SataLedSpecial=1 HddHotplug=1\0" \
	"bootcmd_legacy=sf probe; sf read ${loadaddr} 0xd0000 0x2d0000; " \
		"sf read ${ramdisk_addr_r} 0x3a0000 0x430000; " \
		"setenv bootargs $bootargs_legacy; " \
		"bootm ${loadaddr} ${ramdisk_addr_r}\0"	\
	"usb0Mode=host\0usb1Mode=host\0usb2Mode=device\0" \
	"ethmtu=1500\0eth1mtu=1500\0" \
	"update_uboot=sf probe; dhcp; "	\
		"mw.b ${loadaddr} 0x0 0xd0000; " \
		"tftpboot ${loadaddr} u-boot-with-spl.kwb; " \
		"sf update ${loadaddr} 0x0 0xd0000\0"

#define LOAD_ADDRESS_ENV_SETTINGS \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0"

#define CFG_EXTRA_ENV_SETTINGS \
	RELOCATION_LIMITS_ENV_SETTINGS \
	LOAD_ADDRESS_ENV_SETTINGS \
	EXTRA_ENV_SETTINGS_LEGACY \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"console=ttyS0,115200\0"

#endif /* CONFIG_SPL_BUILD */

#endif /* _CONFIG_SYNOLOGY_DS414_H */
