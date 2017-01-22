/*
 * (C) Copyright 2016
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STM32_RCC_H
#define _STM32_RCC_H

#define RCC_CR		0x00	/* clock control */
#define RCC_PLLCFGR	0x04	/* PLL configuration */
#define RCC_CFGR	0x08	/* clock configuration */
#define RCC_CIR		0x0C	/* clock interrupt */
#define RCC_AHB1RSTR	0x10	/* AHB1 peripheral reset */
#define RCC_AHB2RSTR	0x14	/* AHB2 peripheral reset */
#define RCC_AHB3RSTR	0x18	/* AHB3 peripheral reset */
#define RCC_APB1RSTR	0x20	/* APB1 peripheral reset */
#define RCC_APB2RSTR	0x24	/* APB2 peripheral reset */
#define RCC_AHB1ENR	0x30	/* AHB1 peripheral clock enable */
#define RCC_AHB2ENR	0x34	/* AHB2 peripheral clock enable */
#define RCC_AHB3ENR	0x38	/* AHB3 peripheral clock enable */
#define RCC_APB1ENR	0x40	/* APB1 peripheral clock enable */
#define RCC_APB2ENR	0x44	/* APB2 peripheral clock enable */
#define RCC_AHB1LPENR	0x50	/* periph clk enable in low pwr mode */
#define RCC_AHB2LPENR	0x54	/* AHB2 periph clk enable in low pwr mode */
#define RCC_AHB3LPENR	0x58	/* AHB3 periph clk enable in low pwr mode */
#define RCC_APB1LPENR	0x60	/* APB1 periph clk enable in low pwr mode */
#define RCC_APB2LPENR	0x64	/* APB2 periph clk enable in low pwr mode */
#define RCC_BDCR	0x70	/* Backup domain control */
#define RCC_CSR		0x74	/* clock control & status */
#define RCC_SSCGR	0x80	/* spread spectrum clock generation */
#define RCC_PLLI2SCFGR	0x84	/* PLLI2S configuration */
#define RCC_PLLSAICFG	0x88	/* PLLSAI configuration */
#define RCC_DCKCFG1	0x8C	/* dedicated clocks configuration register */
#define RCC_DCKCFG2	0x90	/* dedicated clocks configuration register */
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
