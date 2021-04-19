/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Chen-Yu Tsai <wens@csie.org>
 *
 * Configuration settings for the Allwinner A23 (sun8i) CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * A23 specific configuration
 */

#ifdef SUNXI_SRAM_A2_SIZE
/*
 * If the SoC has enough SRAM A2, use that for the secure monitor.
 * Skip the first 16 KiB of SRAM A2, which is not usable, as only certain bytes
 * are writable. Reserve the last 17 KiB for the resume shim and SCP firmware.
 */
#define CONFIG_ARMV7_SECURE_BASE	(SUNXI_SRAM_A2_BASE + 16 * 1024)
#define CONFIG_ARMV7_SECURE_MAX_SIZE	(SUNXI_SRAM_A2_SIZE - 33 * 1024)
#endif

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#endif /* __CONFIG_H */
