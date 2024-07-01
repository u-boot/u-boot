/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for HMIBSC
 *
 * (C) Copyright 2024 Sumit Garg <sumit.garg@linaro.org>
 */

#ifndef __CONFIGS_HMIBSC_H
#define __CONFIGS_HMIBSC_H

/* PHY needs a longer aneg time */

#define CFG_ENV_FLAGS_LIST_STATIC "BOOT_A_LEFT:dw,BOOT_B_LEFT:dw,BOOT_ORDER:sw"

#endif
