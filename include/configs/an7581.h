/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for Airoha AN7581
 */

#ifndef __AN7581_H
#define __AN7581_H

#include <linux/sizes.h>

#define CFG_SYS_UBOOT_BASE		CONFIG_TEXT_BASE

#define CFG_SYS_INIT_RAM_ADDR           CONFIG_TEXT_BASE
#define CFG_SYS_INIT_RAM_SIZE           SZ_2M

/* DRAM */
#define CFG_SYS_SDRAM_BASE		0x80000000

#endif
