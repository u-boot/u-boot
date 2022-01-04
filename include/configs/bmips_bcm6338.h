/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#ifndef __CONFIG_BMIPS_BCM6338_H
#define __CONFIG_BMIPS_BCM6338_H

#include <linux/sizes.h>

/* CPU */
#define CONFIG_SYS_MIPS_TIMER_FREQ	120000000

/* RAM */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

/* U-Boot */

#if defined(CONFIG_BMIPS_BOOT_RAM)
#define CONFIG_SYS_INIT_SP_OFFSET	SZ_8K
#endif

#define CONFIG_SYS_FLASH_BASE			0xbfc00000
#define CONFIG_SYS_FLASH_EMPTY_INFO

#endif /* __CONFIG_BMIPS_BCM6338_H */
