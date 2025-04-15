/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024, Thomas Bonnefille <thomas.bonnefille@bootlin.com>
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <config_distro_bootcmd.h>

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#define CFG_SYS_SDRAM_BASE         0x80000000

#define CFG_EXTRA_ENV_SETTINGS  "consoledev=ttyS0\0" \
								"baudrate=115200\0" \
								"fdt_addr_r=0x82000000\0" \
								"kernel_addr_r=0x81000000\0" \
								"scriptaddr=0x80c00000\0" \
								BOOTENV

#endif /* __CONFIG_H */
