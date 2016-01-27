/*
 * (C) Copyright 2016
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARM_ARCH_PERIPH_H
#define __ASM_ARM_ARCH_PERIPH_H

/*
 * Peripherals required for pinmux configuration. List will
 * grow with support for more devices getting added.
 * Numbering based on interrupt table.
 *
 */
enum periph_id {
	UART1_GPIOA_9_10 = 0,
	UART2_GPIOD_5_6,
};

enum periph_clock {
	USART1_CLOCK_CFG = 0,
	USART2_CLOCK_CFG,
};

#endif /* __ASM_ARM_ARCH_PERIPH_H */
