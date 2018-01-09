/*
 * (C) Copyright 2016
 * Michael Kurz, michi.kurz@gmail.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STM32_SYSCFG_H
#define _STM32_SYSCFG_H

struct stm32_syscfg_regs {
	u32 memrmp;
	u32 pmc;
	u32 exticr1;
	u32 exticr2;
	u32 exticr3;
	u32 exticr4;
	u32 cmpcr;
};

/*
 * SYSCFG registers base
 */
#define STM32_SYSCFG		((struct stm32_syscfg_regs *)STM32_SYSCFG_BASE)

/* SYSCFG memory remap register */
#define SYSCFG_MEMRMP_MEM_BOOT	BIT(0)
#define SYSCFG_MEMRMP_SWP_FMC	BIT(10)

/* SYSCFG peripheral mode configuration register */
#define SYSCFG_PMC_ADCXDC2	BIT(16)
#define SYSCFG_PMC_MII_RMII_SEL	BIT(23)

/* Compensation cell control register */
#define SYSCFG_CMPCR_CMP_PD	BIT(0)
#define SYSCFG_CMPCR_READY	BIT(8)

#endif
