/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
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
	GPIO_A_CLOCK_CFG,
	GPIO_B_CLOCK_CFG,
	GPIO_C_CLOCK_CFG,
	GPIO_D_CLOCK_CFG,
	GPIO_E_CLOCK_CFG,
	GPIO_F_CLOCK_CFG,
	GPIO_G_CLOCK_CFG,
	GPIO_H_CLOCK_CFG,
	GPIO_I_CLOCK_CFG,
	GPIO_J_CLOCK_CFG,
	GPIO_K_CLOCK_CFG,
};

#endif /* __ASM_ARM_ARCH_PERIPH_H */
