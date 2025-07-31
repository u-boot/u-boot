// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 */

#include <config.h>
#include <debug_uart.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <linux/bitops.h>

#if IS_ENABLED(CONFIG_STM32MP13X)
#define RCC_MP_APB1ENSETR (STM32_RCC_BASE + 0x0700)
#define RCC_MP_AHB4ENSETR (STM32_RCC_BASE + 0x0768)
#elif IS_ENABLED(CONFIG_STM32MP15X)
#define RCC_MP_APB1ENSETR (STM32_RCC_BASE + 0x0A00)
#define RCC_MP_AHB4ENSETR (STM32_RCC_BASE + 0x0A28)
#endif

#define GPIOA_BASE 0x50002000
#define GPIOG_BASE 0x50008000

void board_debug_uart_init(void)
{
	if (CONFIG_DEBUG_UART_BASE != STM32_UART4_BASE)
		return;

	/* UART4 clock enable */
	setbits_le32(RCC_MP_APB1ENSETR, BIT(16));

	if (IS_ENABLED(CONFIG_STM32MP13X)) {
		/* GPIOA clock enable */
		writel(BIT(0), RCC_MP_AHB4ENSETR);
		/* GPIO configuration for DH boards: Uart4 TX = A9 */
		writel(0xfffbffff, GPIOA_BASE + 0x00);
		writel(0x00000080, GPIOA_BASE + 0x24);
	} else if (IS_ENABLED(CONFIG_STM32MP15X)) {
		/* GPIOG clock enable */
		writel(BIT(6), RCC_MP_AHB4ENSETR);
		/* GPIO configuration for ST boards: Uart4 TX = G11 */
		writel(0xffbfffff, GPIOG_BASE + 0x00);
		writel(0x00006000, GPIOG_BASE + 0x24);
	}
}
