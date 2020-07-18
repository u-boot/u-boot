/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-2020
 * Marvell <www.marvell.com>
 */

#ifndef __OCTEON_COMMON_H__
#define __OCTEON_COMMON_H__

/* No DDR init yet -> run in L2 cache with limited resources */
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)
#define CONFIG_SYS_SDRAM_BASE		0xffffffff80000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + (1 << 20))

#define CONFIG_SYS_INIT_SP_OFFSET	0x180000

#endif /* __OCTEON_COMMON_H__ */
