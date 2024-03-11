/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT6735 SoC
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef __MT6735_H
#define __MT6735_H

#include <linux/sizes.h>
#define DEBUG
/* Miscellaneous configurable options */

#define CONFIG_SYS_NONCACHED_MEMORY	SZ_1M

/* Environment */

#define CONFIG_SYS_UBOOT_BASE		0x41e00000
#define CONFIG_SPL_BSS_START_ADDR                0x40800000
#define CFG_SYS_INIT_RAM_ADDR    0x100000
#define CFG_SYS_INIT_RAM_SIZE    0x10000
/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x40000000



#endif
