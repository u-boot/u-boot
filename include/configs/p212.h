/*
 * Configuration for Amlogic P212
 *
 * Copyright (C) 2017 Baylibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MISC_INIT_R

#define CONFIG_PHY_ADDR 		8

/* Serial setup */
#define CONFIG_CONS_INDEX		0

#define MESON_FDTFILE_SETTING "fdtfile=amlogic/meson-gxl-s905x-p212.dtb\0"

#include <configs/meson-gxbb-common.h>

#endif /* __CONFIG_H */
