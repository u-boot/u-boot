/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2023
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include "tegra20-common.h"

/* Tegra common post configuration overwrites text env in the board */
#define BOARD_EXTRA_ENV_SETTINGS \
	"stdin=serial,tegra-kbc,button-kbd,cpcap-pwrbutton\0"

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
