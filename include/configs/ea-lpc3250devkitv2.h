/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Embedded Artists LPC3250 DevKit v2
 * Copyright (C) 2021  Trevor Woerner <twoerner@gmail.com>
 */

#ifndef __CONFIG_EA_LPC3250DEVKITV2_H__
#define __CONFIG_EA_LPC3250DEVKITV2_H__

#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * SoC and board defines
 */
#define CONFIG_BOARD_SIZE_LIMIT 0x000fffff /* maximum allowable size for full U-Boot binary */

/*
 * RAM
 */
#define CONFIG_SYS_SDRAM_BASE EMC_DYCS0_BASE

/*
 * cmd
 */
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE + SZ_4K - GENERATED_GBL_DATA_SIZE)

/*
 * SoC-specific config
 */
#include <asm/arch/config.h>

#endif
