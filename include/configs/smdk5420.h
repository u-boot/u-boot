/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG SMDK5420 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_SMDK5420_H
#define __CONFIG_SMDK5420_H

#include <configs/exynos5-dt.h>

#include <configs/exynos5420.h>

#define CONFIG_SMDK5420			/* which is in a SMDK5420 */

#undef CONFIG_DEFAULT_DEVICE_TREE
#define CONFIG_DEFAULT_DEVICE_TREE	exynos5420-smdk5420

/* select serial console configuration */
#define CONFIG_SERIAL3		/* use SERIAL 3 */

#define CONFIG_SYS_PROMPT	"SMDK5420 # "
#define CONFIG_IDENT_STRING	" for SMDK5420"

#endif	/* __CONFIG_SMDK5420_H */
