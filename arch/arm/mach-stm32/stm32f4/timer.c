/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <stm32_rcc.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>

DECLARE_GLOBAL_DATA_PTR;

#define STM32_TIM2_BASE	(STM32_APB1PERIPH_BASE + 0x0000)

#define RCC_APB1ENR_TIM2EN	(1 << 0)

struct stm32_tim2_5 {
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
	u32 reserved1;
	u32 ccr1;
	u32 ccr2;
	u32 ccr3;
	u32 ccr4;
	u32 reserved2;
	u32 dcr;
	u32 dmar;
	u32 or;
};

#define TIM_CR1_CEN	(1 << 0)

#define TIM_EGR_UG	(1 << 0)

int timer_init(void)
{
	struct stm32_tim2_5 *tim = (struct stm32_tim2_5 *)STM32_TIM2_BASE;

	setbits_le32(&STM32_RCC->apb1enr, RCC_APB1ENR_TIM2EN);

	if (clock_get(CLOCK_AHB) == clock_get(CLOCK_APB1))
		writel((clock_get(CLOCK_APB1) / CONFIG_SYS_HZ_CLOCK) - 1,
		       &tim->psc);
	else
		writel(((clock_get(CLOCK_APB1) * 2) / CONFIG_SYS_HZ_CLOCK) - 1,
		       &tim->psc);

	writel(0xFFFFFFFF, &tim->arr);
	writel(TIM_CR1_CEN, &tim->cr1);
	setbits_le32(&tim->egr, TIM_EGR_UG);

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;
	gd->arch.lastinc = 0;

	return 0;
}

ulong get_timer(ulong base)
{
	return (get_ticks() / (CONFIG_SYS_HZ_CLOCK / CONFIG_SYS_HZ)) - base;
}

unsigned long long get_ticks(void)
{
	struct stm32_tim2_5 *tim = (struct stm32_tim2_5 *)STM32_TIM2_BASE;
	u32 now;

	now = readl(&tim->cnt);

	if (now >= gd->arch.lastinc)
		gd->arch.tbl += (now - gd->arch.lastinc);
	else
		gd->arch.tbl += (0xFFFFFFFF - gd->arch.lastinc) + now;

	gd->arch.lastinc = now;

	return gd->arch.tbl;
}

void reset_timer(void)
{
	struct stm32_tim2_5 *tim = (struct stm32_tim2_5 *)STM32_TIM2_BASE;

	gd->arch.lastinc = readl(&tim->cnt);
	gd->arch.tbl = 0;
}

/* delay x useconds */
void __udelay(ulong usec)
{
	unsigned long long start;

	start = get_ticks();		/* get current timestamp */
	while ((get_ticks() - start) < usec)
		;			/* loop till time has passed */
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ_CLOCK;
}
