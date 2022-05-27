/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SPL definitions for the i.MX7 SPL
 *
 * (C) Copyright 2017 CompuLab, Ltd. http://www.compulab.com
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#ifndef __IMX7_SPL_CONFIG_H
#define __IMX7_SPL_CONFIG_H

#ifdef CONFIG_SPL

/* MMC support */
#if defined(CONFIG_SPL_MMC)
#define CONFIG_SYS_MONITOR_LEN			409600	/* 400 KB */
#endif

#endif /* CONFIG_SPL */

#endif /* __IMX7_SPL_CONFIG_H */
