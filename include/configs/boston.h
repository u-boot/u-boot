/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#ifndef __CONFIGS_BOSTON_H__
#define __CONFIGS_BOSTON_H__

/*
 * General board configuration
 */

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

/*
 * Console
 */

/*
 * Environment
 */

#endif /* __CONFIGS_BOSTON_H__ */
