/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG/GOOGLE PEACH-PIT board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_PEACH_PIT_H
#define __CONFIG_PEACH_PIT_H

#include <configs/exynos5-dt.h>

#include <configs/exynos5420.h>

#undef CONFIG_DEFAULT_DEVICE_TREE
#define CONFIG_DEFAULT_DEVICE_TREE	exynos5420-peach-pit

/* select serial console configuration */
#define CONFIG_SERIAL3		/* use SERIAL 3 */

#define CONFIG_SYS_PROMPT	"Peach # "
#define CONFIG_IDENT_STRING	" for Peach"

#endif	/* __CONFIG_PEACH_PIT_H */
