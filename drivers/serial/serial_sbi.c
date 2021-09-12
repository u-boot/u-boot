// SPDX-License-Identifier: GPL-2.0+

#include <debug_uart.h>
#include <asm/sbi.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int c)
{
	if (CONFIG_IS_ENABLED(RISCV_SMODE))
		sbi_console_putchar(c);
}

DEBUG_UART_FUNCS
