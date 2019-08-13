/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019 ZK Research ApS
 *
 * Author: Zoltan Kuscsik <zoltan@zkres.com
 *
 * Configuration settings for the Broadcom BCM7252 SoC
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_NS16550_COM1	0xf040a900

#define CONFIG_SYS_INIT_RAM_ADDR	0x10200000
#define CONFIG_SYS_TEXT_BASE        0x10000000

#define CONFIG_SYS_MALLOC_LEN		((40 * 1024) << 10) /* 40 MiB */

#include "bcmstb.h"

#define BCMSTB_TIMER_LOW	0xf0412008
#define BCMSTB_TIMER_HIGH	0xf041200c
#define BCMSTB_TIMER_FREQUENCY	0xf0412020
#define BCMSTB_HIF_MSPI_BASE	0xf0203c00
#define BCMSTB_BSPI_BASE	0xf0203a00
#define BCMSTB_HIF_SPI_INTR2	0xf0201a00
#define BCMSTB_CS_REG		0xf0200920
/*
 * Environment configuration for eMMC.
 */
#define CONFIG_ENV_OFFSET	(0x400 * 512)
#define CONFIG_SYS_MMC_ENV_DEV	0
#define CONFIG_SYS_MMC_ENV_PART	2

#define CONFIG_CMD_GPT
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
     "set_bootargs=setenv bootargs root=/dev/mmcblk0gp2 rootwait rw mmc_core.removable=0 vmalloc=380m bmem=496m@1552m bmem=644m@2428m\0" \
     "bootcmd_flash1=run set_bootargs; " \
          " mmc dev 0 5 ; mmc read 0x8000 0x22 0x3000; " \
          " bootz 0x8000 - 0x7818000\0" \
     "bootcmd=run bootcmd_flash1\0" \

#endif	/* __CONFIG_H */
