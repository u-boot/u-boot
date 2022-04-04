/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_BOOTMAPSZ		(16 << 20)

#define CONFIG_SYS_TIMER_RATE		(150000000/256)
#define CONFIG_SYS_TIMER_COUNTER	(0xFFF34000 + 0x4)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

#define CONFIG_PL011_CLOCK		150000000

#define CONFIG_SYS_BOOTCOUNT_LE		/* Use little-endian accessors */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_64BIT_LBA

/* Environment data setup
*/
#define CONFIG_SYS_NVRAM_BASE_ADDR	0xfff88000	/* NVRAM base address */
#define CONFIG_SYS_NVRAM_SIZE		0x8000		/* NVRAM size */

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_INIT_SP_ADDR		0x01000000

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"fdt_high=0x20000000\0"					\
	"initrd_high=0x20000000\0"

#endif
