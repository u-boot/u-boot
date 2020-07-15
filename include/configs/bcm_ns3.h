/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 *
 */

#ifndef __BCM_NS3_H
#define __BCM_NS3_H

#include <linux/sizes.h>

#define CONFIG_HOSTNAME			"NS3"

/* Physical Memory Map */
#define V2M_BASE			0x80000000
#define PHYS_SDRAM_1			V2M_BASE

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_LOAD_ADDR		(PHYS_SDRAM_1 + 0x80000)

/*
 * Initial SP before reloaction is placed at end of first DRAM bank,
 * which is 0x1_0000_0000.
 * Just before re-loaction, new SP is updated and re-location happens.
 * So pointing the initial SP to end of 2GB DDR is not a problem
 */
#define CONFIG_SYS_INIT_SP_ADDR		(PHYS_SDRAM_1 + 0x80000000)
/* 12MB Malloc size */
#define CONFIG_SYS_MALLOC_LEN		(SZ_8M + SZ_4M)

/* console configuration */
#define CONFIG_SYS_NS16550_CLK		25000000

#define CONFIG_SYS_CBSIZE		SZ_1K
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#endif /* __BCM_NS3_H */
