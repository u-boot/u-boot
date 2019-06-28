/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef _MACH_DEBUG_UART_H
#define _MACH_DEBUG_UART_H

unsigned int uniphier_ld4_debug_uart_init(void);
unsigned int uniphier_pro4_debug_uart_init(void);
unsigned int uniphier_sld8_debug_uart_init(void);
unsigned int uniphier_pro5_debug_uart_init(void);
unsigned int uniphier_pxs2_debug_uart_init(void);
unsigned int uniphier_ld6b_debug_uart_init(void);

#endif /* _MACH_DEBUG_UART_H */
