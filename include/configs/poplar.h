/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Linaro
 *
 * Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 *
 * Configuration for Poplar 96boards CE. Parts were derived from other ARM
 * configurations.
 */

#ifndef _POPLAR_H_
#define _POPLAR_H_

#include <linux/sizes.h>

/* DRAM banks */

/* SYS */
#define CONFIG_SYS_BOOTM_LEN			SZ_64M
#define CONFIG_SYS_INIT_SP_ADDR			0x200000

/* ATF bl33.bin load address (must match) */

/* USB configuration */
#define CONFIG_USB_MAX_CONTROLLER_COUNT		2

/*****************************************************************************
 *  Initial environment variables
 *****************************************************************************/

#define BOOT_TARGET_DEVICES(func)					\
					func(USB, usb, 0)		\
					func(MMC, mmc, 0)		\
					func(DHCP, dhcp, na)
#ifndef CONFIG_SPL_BUILD
#include <config_distro_bootcmd.h>
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
			"loader_mmc_blknum=0x0\0"			\
			"loader_mmc_nblks=0x780\0"			\
			"env_mmc_blknum=0xf80\0"			\
			"env_mmc_nblks=0x80\0"				\
			"kernel_addr_r=0x30000000\0"			\
			"pxefile_addr_r=0x32000000\0"			\
			"scriptaddr=0x32000000\0"			\
			"fdt_addr_r=0x32200000\0"			\
			"fdtfile=hisilicon/hi3798cv200-poplar.dtb\0"	\
			"ramdisk_addr_r=0x32400000\0"			\
			BOOTENV

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512
#define CONFIG_SYS_MAXARGS		64

#endif /* _POPLAR_H_ */
