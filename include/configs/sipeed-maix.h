/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */

#ifndef CONFIGS_SIPEED_MAIX_H
#define CONFIGS_SIPEED_MAIX_H

#include <linux/sizes.h>

/* Start just below the second bank so we don't clobber it during reloc */
#define CONFIG_SYS_INIT_SP_ADDR 0x803FFFFF

#define CONFIG_SYS_SDRAM_BASE 0x80000000
#define CONFIG_SYS_SDRAM_SIZE SZ_8M

#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80060000\0" \
	"fdt_addr_r=0x80400000\0" \
	"scriptaddr=0x80020000\0" \
	"kernel_addr_r=0x80060000\0" \
	"fdtfile=k210/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"k210_bootcmd=load mmc 0:1 $loadaddr /uImage && " \
		"load mmc 0:1 $fdt_addr_r /k210.dtb && " \
		"bootm $loadaddr - $fdt_addr_r\0"
#endif

#endif /* CONFIGS_SIPEED_MAIX_H */
