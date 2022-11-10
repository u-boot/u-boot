/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7981 SoC
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef __MT7981_H
#define __MT7981_H

/* Uboot definition */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_TEXT_BASE

/* SPL -> Uboot */
#define CONFIG_SYS_UBOOT_START		CONFIG_TEXT_BASE

/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x40000000

#endif
