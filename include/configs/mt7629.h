/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7629 SoC
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#ifndef __MT7629_H
#define __MT7629_H

#include <linux/sizes.h>

/* Miscellaneous configurable options */

/* Environment */

/* Defines for SPL */

#define CONFIG_SPI_ADDR			0x30000000
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPI_ADDR + CONFIG_SPL_PAD_TO)

/* SPL -> Uboot */

/* UBoot -> Kernel */

/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x40000000

/* Ethernet */
#define CONFIG_IPADDR			192.168.1.1
#define CONFIG_SERVERIP			192.168.1.2

#endif
