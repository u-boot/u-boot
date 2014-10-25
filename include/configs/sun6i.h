/*
 * (C) Copyright 2012-2013 Henrik Nordstrom <henrik@henriknordstrom.net>
 * (C) Copyright 2013 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 * (C) Copyright 2013 Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * Configuration settings for the Allwinner A31 (sun6i) CPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * A31 specific configuration
 */
#define CONFIG_CLK_FULL_SPEED		1008000000

#define CONFIG_SYS_PROMPT		"sun6i# "

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#endif /* __CONFIG_H */
