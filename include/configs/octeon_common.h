/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-2020
 * Marvell <www.marvell.com>
 */

#ifndef __OCTEON_COMMON_H__
#define __OCTEON_COMMON_H__

#if defined(CONFIG_RAM_OCTEON)
#define CONFIG_SYS_MALLOC_LEN		(16 << 20)
#define CONFIG_SYS_INIT_SP_OFFSET	0x20100000
#else
/* No DDR init -> run in L2 cache with limited resources */
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)
#define CONFIG_SYS_INIT_SP_OFFSET	0x00180000
#endif

#define CONFIG_SYS_SDRAM_BASE		0xffffffff80000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + (1 << 20))

#define CONFIG_SYS_BOOTM_LEN		(64 << 20)	/* 64M */

#endif /* __OCTEON_COMMON_H__ */
