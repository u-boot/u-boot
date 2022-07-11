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

#endif

#endif
