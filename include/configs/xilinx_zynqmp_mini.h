/*
 * Configuration for Xilinx ZynqMP Flash utility
 *
 * (C) Copyright 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * Siva Durga Prasad Paladugu <sivadur@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_MINI_H
#define __CONFIG_ZYNQMP_MINI_H

#include <configs/xilinx_zynqmp.h>

/* Undef unneeded configs */
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_ENV_SIZE
#undef CONFIG_ZLIB
#undef CONFIG_GZIP
#undef CONFIG_CMD_ENV
#undef CONFIG_MP
#undef CONFIG_SYS_INIT_SP_ADDR
#undef CONFIG_MTD_DEVICE
#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_VXWORKS
#undef CONFIG_BOOTM_LINUX
#undef CONFIG_BOARD_LATE_INIT

/* BOOTP options */
#undef CONFIG_BOOTP_BOOTFILESIZE
#undef CONFIG_BOOTP_MAY_FAIL
#undef CONFIG_CMD_UNZIP

#undef CONFIG_NR_DRAM_BANKS

#endif /* __CONFIG_ZYNQMP_MINI_H */
