/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#ifndef __CONFIGS_BOSTON_H__
#define __CONFIGS_BOSTON_H__

/*
 * General board configuration
 */
#define CONFIG_SYS_BOOTM_LEN		(64 * 1024 * 1024)

/*
 * CPU
 */
#define CONFIG_SYS_MIPS_TIMER_FREQ	30000000

/*
 * PCI
 */

/*
 * Memory map
 */
#ifdef CONFIG_64BIT
# define CONFIG_SYS_SDRAM_BASE		0xffffffff80000000
#else
# define CONFIG_SYS_SDRAM_BASE		0x80000000
#endif

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

/*
 * Console
 */

/*
 * Flash
 */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	1
#define CONFIG_SYS_MAX_FLASH_SECT		1024

/*
 * Environment
 */

#endif /* __CONFIGS_BOSTON_H__ */
