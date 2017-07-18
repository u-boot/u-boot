/*
 * (C) Copyright 2016
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STM32_RCC_H
#define _STM32_RCC_H

#include <dt-bindings/mfd/stm32f7-rcc.h>

/*
 * RCC AHB1ENR specific definitions
 */
#define RCC_AHB1ENR_ETHMAC_EN		BIT(25)
#define RCC_AHB1ENR_ETHMAC_TX_EN	BIT(26)
#define RCC_AHB1ENR_ETHMAC_RX_EN	BIT(27)

/*
 * RCC APB1ENR specific definitions
 */
#define RCC_APB1ENR_TIM2EN		BIT(0)
#define RCC_APB1ENR_PWREN		BIT(28)

/*
 * RCC APB2ENR specific definitions
 */
#define RCC_APB2ENR_SYSCFGEN		BIT(14)

#endif
