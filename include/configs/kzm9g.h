/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2012 Renesas Solutions Corp.
 */

#ifndef __KZM9G_H
#define __KZM9G_H

#define CONFIG_SH73A0

#include <asm/arch/rmobile.h>

/* MEMORY */
#define KZM_SDRAM_BASE	(0x40000000)
#define PHYS_SDRAM		KZM_SDRAM_BASE
#define PHYS_SDRAM_SIZE		(512 * 1024 * 1024)

/* NOR Flash */
#define KZM_FLASH_BASE	(0x00000000)
#define CONFIG_SYS_FLASH_BASE		(KZM_FLASH_BASE)

/* prompt */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }

/* SCIF */

#undef  CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_INIT_RAM_ADDR	(0xE5600000) /* on MERAM */
#define CONFIG_SYS_INIT_RAM_SIZE	(0x10000)
#define LOW_LEVEL_MERAM_STACK		(CONFIG_SYS_INIT_RAM_ADDR - 4)
#define CONFIG_SDRAM_OFFSET_FOR_RT	(16 * 1024 * 1024)
#define CONFIG_SYS_SDRAM_BASE	(KZM_SDRAM_BASE + CONFIG_SDRAM_OFFSET_FOR_RT)
#define CONFIG_SYS_SDRAM_SIZE	(PHYS_SDRAM_SIZE - CONFIG_SDRAM_OFFSET_FOR_RT)

#define CONFIG_SYS_BOOTMAPSZ	(8 * 1024 * 1024)

#define CONFIG_STANDALONE_LOAD_ADDR	0x41000000

/* FLASH */
#define FLASH_SECTOR_SIZE	(256 * 1024)	/* 256 KB sectors */

/* Timeout for Flash erase operations (in ms) */
/* Timeout for Flash write operations (in ms) */
/* Timeout for Flash set sector lock bit operations (in ms) */
/* Timeout for Flash clear lock bit operations (in ms) */

/* GPIO / PFC */
#define CONFIG_SH_GPIO_PFC

/* Clock */
#define CONFIG_GLOBAL_TIMER
#define CONFIG_SYS_CPU_CLK	(1196000000)
#define TMU_CLK_DIVIDER		(4)	/* 4 (default), 16, 64, 256 or 1024 */

#endif /* __KZM9G_H */
