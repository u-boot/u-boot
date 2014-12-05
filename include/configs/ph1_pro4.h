/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PH1_XXX_H
#define __PH1_XXX_H

/*
 * Serial Configuration
 *   SoC UART     : enable CONFIG_UNIPHIER_SERIAL
 *   On-board UART: enable CONFIG_SYS_NS16550_SERIAL
 */
#if 0
#define CONFIG_SYS_NS16550_SERIAL
#endif

#define CONFIG_SMC911X

#define CONFIG_DDR_NUM_CH0 2
#define CONFIG_DDR_NUM_CH1 2

/*
 * Memory Size & Mapping
 */
/* Physical start address of SDRAM */
#define CONFIG_SDRAM0_BASE	0x80000000
#define CONFIG_SDRAM0_SIZE	0x20000000
#define CONFIG_SDRAM1_BASE	0xa0000000
#define CONFIG_SDRAM1_SIZE	0x20000000

#define CONFIG_SPL_TEXT_BASE 0x100000

#include "uniphier-common.h"

#endif /* __PH1_XXX_H */
