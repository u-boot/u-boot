/*
 * Copyright (C) 2017 Armadeus Systems
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MX6UL_OPOS6UL_H__
#define __ARCH_ARM_MX6UL_OPOS6UL_H__

int opos6ul_board_late_init(void);

#ifdef CONFIG_SPL_BUILD
void opos6ul_setup_uart_debug(void);
#endif

#endif
