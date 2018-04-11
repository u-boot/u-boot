/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Amlogic P212
 *
 * Copyright (C) 2017 Baylibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MISC_INIT_R

/* Serial setup */

#define MESON_FDTFILE_SETTING "fdtfile=amlogic/meson-gxl-s905x-p212.dtb\0"

#include <configs/meson-gx-common.h>

#endif /* __CONFIG_H */
