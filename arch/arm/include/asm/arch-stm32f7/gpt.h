/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STM32_GPT_H
#define _STM32_GPT_H

#include <asm/arch/stm32.h>

struct gpt_regs {
	u32 cr1;
	u32 cr2;
	u32 smcr;
	u32 dier;
	u32 sr;
	u32 egr;
	u32 ccmr1;
	u32 ccmr2;
	u32 ccer;
	u32 cnt;
	u32 psc;
	u32 arr;
	u32 reserved;
	u32 ccr1;
	u32 ccr2;
	u32 ccr3;
	u32 ccr4;
	u32 reserved1;
	u32 dcr;
	u32 dmar;
	u32 tim2_5_or;
};

struct gpt_regs *const gpt1_regs_ptr =
	(struct gpt_regs *)TIM2_BASE;

/* Timer control1 register  */
#define GPT_CR1_CEN			BIT(0)
#define GPT_MODE_AUTO_RELOAD		BIT(7)

/* Auto reload register for free running config */
#define GPT_FREE_RUNNING		0xFFFFFFFF

/* Timer, HZ specific defines */
#define CONFIG_STM32_HZ			1000

/* Timer Event Generation registers */
#define TIM_EGR_UG			BIT(0)

#endif
