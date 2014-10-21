/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PH1_XXX_H
#define __PH1_XXX_H

/*
 * Support Card Select
 *
 *  CONFIG_PFC_MICRO_SUPPORT_CARD - Original Micro Support Card made by PFC.
 *  CONFIG_DCC_MICRO_SUPPORT_CARD - DCC version Micro Support Card.
 *                       CPLD is re-programmed for ARIMA board compatibility.
 *  No define                     - No support card.
 */

#if 0
#define CONFIG_PFC_MICRO_SUPPORT_CARD
#else
#define CONFIG_DCC_MICRO_SUPPORT_CARD
#endif

/*
 * Serial Configuration
 *   SoC UART     : enable CONFIG_UNIPHIER_SERIAL
 *   On-board UART: enable CONFIG_SYS_NS16550_SERIAL
 */
#if 1
#define CONFIG_UNIPHIER_SERIAL
#else
#define CONFIG_SYS_NS16550_SERIAL
#endif

#define CONFIG_SYS_UNIPHIER_UART_CLK    80000000

#define CONFIG_SMC911X

#define CONFIG_DDR_NUM_CH0 1
#define CONFIG_DDR_NUM_CH1 1

#define CONFIG_DDR_FREQ 1333

/* #define CONFIG_DDR_STANDARD */

/*
 * Memory Size & Mapping
 */
/* Physical start address of SDRAM */
#define CONFIG_SDRAM0_BASE	0x80000000
#define CONFIG_SDRAM0_SIZE	0x10000000
#define CONFIG_SDRAM1_BASE	0x90000000
#define CONFIG_SDRAM1_SIZE	0x10000000

#define CONFIG_SPL_TEXT_BASE 0x40000

#include "uniphier-common.h"

#endif /* __PH1_XXX_H */
