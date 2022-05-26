/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 */
#ifndef __IMX6_SPL_CONFIG_H
#define __IMX6_SPL_CONFIG_H

#ifdef CONFIG_SPL

/* MMC support */
#if defined(CONFIG_SPL_MMC)
#define CONFIG_SYS_MONITOR_LEN			409600	/* 400 KB */
#endif

/* SATA support */
#if defined(CONFIG_SPL_SATA)
#define CONFIG_SYS_SATA_FAT_BOOT_PARTITION	1
#endif

#if defined(CONFIG_MX6SX) || defined(CONFIG_MX6SL) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
#define CONFIG_SPL_BSS_START_ADDR      0x88200000
#define CONFIG_SYS_SPL_MALLOC_START    0x88300000
#define CONFIG_SYS_SPL_MALLOC_SIZE     0x100000		/* 1 MB */
#else
#define CONFIG_SPL_BSS_START_ADDR	0x18200000
#define CONFIG_SYS_SPL_MALLOC_START	0x18300000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000	/* 1 MB */
#endif
#endif

#endif
