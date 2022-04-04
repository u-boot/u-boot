/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef __CONFIG_MT7620_H
#define __CONFIG_MT7620_H

#define CONFIG_SYS_MIPS_TIMER_FREQ	290000000

#define CONFIG_SYS_BOOTPARAMS_LEN	0x20000

#define CONFIG_SYS_SDRAM_BASE		0x80000000

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

#define CONFIG_SYS_BOOTM_LEN		0x1000000

#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_CBSIZE		1024

/* SPL */

#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE
#define CONFIG_SPL_BSS_START_ADDR	0x80010000
#define CONFIG_SPL_BSS_MAX_SIZE		0x10000
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_PAD_TO		0

/* Dummy value */
#define CONFIG_SYS_UBOOT_BASE		0

#endif /* __CONFIG_MT7620_H */
