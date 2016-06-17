/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/rcc.h>
#include <asm/arch/stm32.h>
#include <asm/arch/stm32_periph.h>

void clock_setup(int peripheral)
{
	switch (peripheral) {
	case USART1_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_APB2ENR, RCC_ENR_USART1EN);
		break;
	case GPIO_A_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_A_EN);
		break;
	case GPIO_B_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_B_EN);
		break;
	case GPIO_C_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_C_EN);
		break;
	case GPIO_D_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_D_EN);
		break;
	case GPIO_E_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_E_EN);
		break;
	case GPIO_F_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_F_EN);
		break;
	case GPIO_G_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_G_EN);
		break;
	case GPIO_H_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_H_EN);
		break;
	case GPIO_I_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_I_EN);
		break;
	case GPIO_J_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_J_EN);
		break;
	case GPIO_K_CLOCK_CFG:
		setbits_le32(RCC_BASE + RCC_AHB1ENR, RCC_ENR_GPIO_K_EN);
		break;
	default:
		break;
	}
}
