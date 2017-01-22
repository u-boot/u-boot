/*
 * (C) Copyright 2016
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STM32_RCC_H
#define _STM32_RCC_H

/*
 * RCC AHB1ENR specific definitions
 */
#define RCC_AHB1ENR_GPIO_A_EN		BIT(0)
#define RCC_AHB1ENR_GPIO_B_EN		BIT(1)
#define RCC_AHB1ENR_GPIO_C_EN		BIT(2)
#define RCC_AHB1ENR_GPIO_D_EN		BIT(3)
#define RCC_AHB1ENR_GPIO_E_EN		BIT(4)
#define RCC_AHB1ENR_GPIO_F_EN		BIT(5)
#define RCC_AHB1ENR_GPIO_G_EN		BIT(6)
#define RCC_AHB1ENR_GPIO_H_EN		BIT(7)
#define RCC_AHB1ENR_GPIO_I_EN		BIT(8)
#define RCC_AHB1ENR_GPIO_J_EN		BIT(9)
#define RCC_AHB1ENR_GPIO_K_EN		BIT(10)
#define RCC_AHB1ENR_ETHMAC_EN		BIT(25)
#define RCC_AHB1ENR_ETHMAC_TX_EN	BIT(26)
#define RCC_AHB1ENR_ETHMAC_RX_EN	BIT(27)
#define RCC_AHB1ENR_ETHMAC_PTP_EN	BIT(28)

/*
 * RCC AHB3ENR specific definitions
 */
#define RCC_AHB3ENR_FMC_EN		BIT(0)
#define RCC_AHB3ENR_QSPI_EN             BIT(1)

/*
 * RCC APB1ENR specific definitions
 */
#define RCC_APB1ENR_TIM2EN		BIT(0)
#define RCC_APB1ENR_USART2EN		BIT(17)
#define RCC_APB1ENR_USART3EN		BIT(18)
#define RCC_APB1ENR_PWREN		BIT(28)

/*
 * RCC APB2ENR specific definitions
 */
#define RCC_APB2ENR_USART1EN		BIT(4)
#define RCC_APB2ENR_USART6EN		BIT(5)
#define RCC_APB2ENR_SYSCFGEN		BIT(14)

#endif
