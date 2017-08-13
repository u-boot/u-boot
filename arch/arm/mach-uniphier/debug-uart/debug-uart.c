/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <linux/io.h>
#include <linux/serial_reg.h>

#include "../soc-info.h"
#include "debug-uart.h"

#define UNIPHIER_UART_TX		0x00
#define UNIPHIER_UART_LCR_MCR		0x10
#define UNIPHIER_UART_LSR		0x14
#define UNIPHIER_UART_LDR		0x24

static void _debug_uart_putc(int c)
{
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;

	while (!(readl(base + UNIPHIER_UART_LSR) & UART_LSR_THRE))
		;

	writel(c, base + UNIPHIER_UART_TX);
}

void _debug_uart_init(void)
{
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;
	unsigned int divisor;

	switch (uniphier_get_soc_id()) {
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	case UNIPHIER_LD4_ID:
		divisor = uniphier_ld4_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	case UNIPHIER_PRO4_ID:
		divisor = uniphier_pro4_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case UNIPHIER_SLD8_ID:
		divisor = uniphier_sld8_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case UNIPHIER_PRO5_ID:
		divisor = uniphier_pro5_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	case UNIPHIER_PXS2_ID:
		divisor = uniphier_pxs2_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case UNIPHIER_LD6B_ID:
		divisor = uniphier_ld6b_debug_uart_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11) || defined(CONFIG_ARCH_UNIPHIER_LD20)
	case UNIPHIER_LD11_ID:
	case UNIPHIER_LD20_ID:
		divisor = uniphier_ld20_debug_uart_init();
		break;
#endif
	default:
		return;
	}

	writel(UART_LCR_WLEN8 << 8, base + UNIPHIER_UART_LCR_MCR);

	writel(divisor, base + UNIPHIER_UART_LDR);
}
DEBUG_UART_FUNCS
