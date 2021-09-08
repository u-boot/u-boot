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
#define CONFIG_SYS_FLASH_CFI_WIDTH	(FLASH_CFI_16BIT)
#define CONFIG_SYS_MAX_FLASH_BANKS	(1)
#define CONFIG_SYS_MAX_FLASH_SECT	(512)

/* prompt */
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }

/* SCIF */
#define CONFIG_CONS_SCIF4

#undef  CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_INIT_RAM_ADDR	(0xE5600000) /* on MERAM */
#define CONFIG_SYS_INIT_RAM_SIZE	(0x10000)
#define LOW_LEVEL_MERAM_STACK		(CONFIG_SYS_INIT_RAM_ADDR - 4)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SDRAM_OFFSET_FOR_RT	(16 * 1024 * 1024)
#define CONFIG_SYS_SDRAM_BASE	(KZM_SDRAM_BASE + CONFIG_SDRAM_OFFSET_FOR_RT)
#define CONFIG_SYS_SDRAM_SIZE	(PHYS_SDRAM_SIZE - CONFIG_SDRAM_OFFSET_FOR_RT)

#define CONFIG_SYS_MONITOR_BASE	(KZM_FLASH_BASE)
#define CONFIG_SYS_BOOTMAPSZ	(8 * 1024 * 1024)

#define CONFIG_STANDALONE_LOAD_ADDR	0x41000000

/* FLASH */
#undef  CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define FLASH_SECTOR_SIZE	(256 * 1024)	/* 256 KB sectors */

/* Timeout for Flash erase operations (in ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(3 * 1000)
/* Timeout for Flash write operations (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(3 * 1000)
/* Timeout for Flash set sector lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_LOCK_TOUT		(3 * 1000)
/* Timeout for Flash clear lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	(3 * 1000)

#undef  CONFIG_SYS_DIRECT_FLASH_TFTP

/* GPIO / PFC */
#define CONFIG_SH_GPIO_PFC

/* Clock */
#define CONFIG_GLOBAL_TIMER
#define CONFIG_SYS_CLK_FREQ	(48000000)
#define CONFIG_SYS_CPU_CLK	(1196000000)
#define CONFIG_SH_SCIF_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define TMU_CLK_DIVIDER		(4)	/* 4 (default), 16, 64, 256 or 1024 */

#define CONFIG_NFS_TIMEOUT 10000UL

#endif /* __KZM9G_H */
