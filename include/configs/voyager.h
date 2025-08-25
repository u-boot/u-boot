/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Andes Technology Corporation
 * Randolph Lin, Andes Technology Corporation <randolph@andestech.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define RISCV_MMODE_TIMERBASE           0xe6000000
#define RISCV_MMODE_TIMER_FREQ          60000000

#define RISCV_SMODE_TIMER_FREQ          60000000

/* support JEDEC */
#define PHYS_FLASH_1                   0x8000000       /* BANK 0 */
#define CFG_SYS_FLASH_BASE             PHYS_FLASH_1
#define CFG_SYS_FLASH_BANKS_LIST       { PHYS_FLASH_1, }
#define CFG_SYS_FLASH_BANKS_SIZES      { 0x4000000 }

/* Enable distro boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS	\
				"fdt_high=0xffffffffffffffff\0" \
				"initrd_high=0xffffffffffffffff\0" \
				"kernel_addr_r=0x400600000\0" \
				"kernel_comp_addr_r=0x404600000\0" \
				"kernel_comp_size=0x04000000\0" \
				"pxefile_addr_r=0x408600000\0" \
				"scriptaddr=0x408700000\0" \
				"fdt_addr_r=0x408800000\0" \
				"ramdisk_addr_r=0x408900000\0" \
				BOOTENV

#endif /* __CONFIG_H */
