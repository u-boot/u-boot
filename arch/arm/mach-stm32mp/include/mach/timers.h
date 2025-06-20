/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 * Author: Cheick Traore <cheick.traore@foss.st.com>
 *
 * Originally based on the Linux kernel v6.1 include/linux/mfd/stm32-timers.h.
 */

#ifndef __STM32_TIMERS_H
#define __STM32_TIMERS_H

#include <clk.h>

#define TIM_CR1		0x00	/* Control Register 1      */
#define TIM_CR2		0x04	/* Control Register 2      */
#define TIM_SMCR	0x08	/* Slave mode control reg  */
#define TIM_DIER	0x0C	/* DMA/interrupt register  */
#define TIM_SR		0x10	/* Status register	   */
#define TIM_EGR		0x14	/* Event Generation Reg    */
#define TIM_CCMR1	0x18	/* Capt/Comp 1 Mode Reg    */
#define TIM_CCMR2	0x1C	/* Capt/Comp 2 Mode Reg    */
#define TIM_CCER	0x20	/* Capt/Comp Enable Reg    */
#define TIM_CNT		0x24	/* Counter		   */
#define TIM_PSC		0x28	/* Prescaler               */
#define TIM_ARR		0x2c	/* Auto-Reload Register    */
#define TIM_CCRx(x)	(0x34 + 4 * ((x) - 1))	/* Capt/Comp Register x (x ∈ {1, .. 4})	*/
#define TIM_BDTR	0x44	/* Break and Dead-Time Reg */
#define TIM_DCR		0x48	/* DMA control register    */
#define TIM_DMAR	0x4C	/* DMA register for transfer */
#define TIM_TISEL	0x68	/* Input Selection         */

#define TIM_HWCFGR2	0x3EC	/* hardware configuration 2 Reg (MP25)	*/
#define TIM_HWCFGR1	0x3F0	/* hardware configuration 1 Reg (MP25)	*/
#define TIM_IPIDR	0x3F8	/* IP identification Reg (MP25)		*/

#define TIM_CR1_CEN	BIT(0)	/* Counter Enable	   */
#define TIM_CR1_ARPE	BIT(7)
#define TIM_CCER_CCXE	(BIT(0) | BIT(4) | BIT(8) | BIT(12))
#define TIM_CCER_CC1E	BIT(0)
#define TIM_CCER_CC1P	BIT(1)	/* Capt/Comp 1  Polarity   */
#define TIM_CCER_CC1NE	BIT(2)	/* Capt/Comp 1N out Ena    */
#define TIM_CCER_CC1NP	BIT(3)	/* Capt/Comp 1N Polarity   */
#define TIM_CCMR_PE	BIT(3)	/* Channel Preload Enable  */
#define TIM_CCMR_M1	(BIT(6) | BIT(5))  /* Channel PWM Mode 1 */
#define TIM_BDTR_MOE	BIT(15)	/* Main Output Enable      */
#define TIM_EGR_UG	BIT(0)	/* Update Generation       */
#define TIM_HWCFGR2_CNT_WIDTH	GENMASK(15, 8)	/* Counter width */
#define TIM_HWCFGR1_NB_OF_DT	GENMASK(7, 4)	/* Complementary outputs & dead-time generators */

#define MAX_TIM_PSC		0xFFFF

#define STM32MP25_TIM_IPIDR	0x00120002

struct stm32_timers_plat {
	void __iomem *base;
	u32 ipidr;
};

struct stm32_timers_priv {
	u32 max_arr;
	ulong rate;
};

#endif
