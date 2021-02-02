/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Xilinx ZynqMP Flash utility
 *
 * (C) Copyright 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * Siva Durga Prasad Paladugu <sivadur@xilinx.com>
 */

#ifndef __CONFIG_ZYNQMP_MINI_H
#define __CONFIG_ZYNQMP_MINI_H

#define CONFIG_EXTRA_ENV_SETTINGS

#include <configs/xilinx_zynqmp.h>

/* Undef unneeded configs */
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_SYS_INIT_SP_ADDR

/* BOOTP options */
#undef CONFIG_BOOTP_BOOTFILESIZE
#undef CONFIG_BOOTP_MAY_FAIL
#undef CONFIG_SYS_CBSIZE
#define CONFIG_SYS_CBSIZE		1024

#endif /* __CONFIG_ZYNQMP_MINI_H */
