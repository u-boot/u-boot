/*
 * (C) Copyright 2013 Xilinx, Inc.
 *
 * Configuration settings for the Xilinx Zynq ZC770 board.
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_ZC770_H
#define __CONFIG_ZYNQ_ZC770_H

#define CONFIG_SYS_NO_FLASH

#if defined(CONFIG_ZC770_XM012)
# undef CONFIG_SYS_NO_FLASH

#endif

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_ZC770_H */
