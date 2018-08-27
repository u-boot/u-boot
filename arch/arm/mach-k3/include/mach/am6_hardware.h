/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: AM6 SoC definitions, structures etc.
 *
 * (C) Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 */
#ifndef __ASM_ARCH_AM6_HARDWARE_H
#define __ASM_ARCH_AM6_HARDWARE_H

#include <config.h>

#define CTRL_MMR0_BASE					0x00100000
#define CTRLMMR_MAIN_DEVSTAT				(CTRL_MMR0_BASE + 0x30)

#define CTRLMMR_MAIN_DEVSTAT_BOOTMODE_MASK		GENMASK(3, 0)
#define CTRLMMR_MAIN_DEVSTAT_BOOTMODE_SHIFT		0
#define CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_MASK		GENMASK(6, 4)
#define CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT	4

/* MCU SCRATCHPAD usage */
#define K3_BOOT_PARAM_TABLE_INDEX_VAL	CONFIG_SYS_K3_MCU_SCRATCHPAD_BASE

#endif /* __ASM_ARCH_AM6_HARDWARE_H */
