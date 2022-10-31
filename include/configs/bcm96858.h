/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2022 Broadcom Ltd.
 */

#ifndef __BCM96858_H
#define __BCM96858_H

#define CONFIG_SYS_SDRAM_BASE		0x00000000

#ifdef CONFIG_MTD_RAW_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#endif /* CONFIG_MTD_RAW_NAND */

#endif
