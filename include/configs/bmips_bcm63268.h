/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_BMIPS_BCM63268_H
#define __CONFIG_BMIPS_BCM63268_H

/* CPU */
#define CONFIG_SYS_MIPS_TIMER_FREQ	200000000

/* RAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x80000000

/* U-Boot */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE + 0x100000

#if defined(CONFIG_BMIPS_BOOT_RAM)
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_INIT_SP_OFFSET	0x2000
#endif

#endif /* __CONFIG_BMIPS_BCM63268_H */
