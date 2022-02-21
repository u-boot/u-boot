/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <config.h>
#include <linux/sizes.h>

/* For booting Linux, use the first 16MB of memory */
#define CONFIG_SYS_BOOTMAPSZ		SZ_16M

#define CONFIG_SYS_FLASH_BASE		0x08000000
#define CONFIG_SYS_INIT_SP_ADDR		0x30040000
#define CONFIG_STANDALONE_LOAD_ADDR 0x24000000

#define CONFIG_SYS_HZ_CLOCK		1000000

#define CONFIG_SYS_MAXARGS		16

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS				\
			"fdtfile=stm32h743i-nucleo.dtb\0"	\
			BOOTENV

#endif /* __CONFIG_H */
