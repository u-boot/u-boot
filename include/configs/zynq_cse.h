/*
 * (C) Copyright 2013 - 2017 Xilinx.
 *
 * Configuration settings for the Xilinx Zynq CSE board.
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_CSE_H
#define __CONFIG_ZYNQ_CSE_H

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF

#include <configs/zynq-common.h>

/* Undef unneeded configs */
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOARD_LATE_INIT
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_ENV_SIZE
#undef CONFIG_CMDLINE_EDITING
#undef CONFIG_AUTO_COMPLETE
#undef CONFIG_ZLIB
#undef CONFIG_GZIP

#undef CONFIG_SYS_LONGHELP

#undef CONFIG_SYS_CBSIZE
#undef CONFIG_BOOTM_VXWORKS
#undef CONFIG_BOOTM_LINUX

#define CONFIG_SYS_CBSIZE	1024

#define CONFIG_ENV_SIZE		400
#undef CONFIG_SYS_INIT_RAM_ADDR
#undef CONFIG_SYS_INIT_RAM_SIZE
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFDE000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#undef CONFIG_SPL_BSS_START_ADDR
#undef CONFIG_SPL_BSS_MAX_SIZE
#define CONFIG_SPL_BSS_START_ADDR	0x20000
#define CONFIG_SPL_BSS_MAX_SIZE		0x8000

#endif /* __CONFIG_ZYNQ_CSE_H */
