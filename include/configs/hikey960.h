/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019 Linaro
 * Author: Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#ifndef __HIKEY_H
#define __HIKEY_H

#include <linux/sizes.h>

#define CONFIG_SYS_BOOTM_LEN		SZ_64M

/* Physical Memory Map */

/* CONFIG_SYS_TEXT_BASE needs to align with where ATF loads bl33.bin */

#define PHYS_SDRAM_1			0x00000000
#define PHYS_SDRAM_1_SIZE		0xC0000000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#define CONFIG_SYS_INIT_RAM_SIZE	0x1000

#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		19000000

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			0xe82b1000
#define GICC_BASE			0xe82b2000

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)
#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
				"image=Image\0"	\
				"fdtfile=hi3660-hikey960.dtb\0" \
				"fdt_addr_r=0x10000000\0" \
				"kernel_addr_r=0x11000000\0" \
				"scriptaddr=0x00020000\0" \
				"fdt_high=0xffffffffffffffff\0" \
				"initrd_high=0xffffffffffffffff\0" \
				BOOTENV

/* TODO: Remove this once the SD clock is fixed */
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	1024

#endif /* __HIKEY_H */
