/*
 * TNETV107X: Timer implementation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

struct timer_regs {
	u_int32_t pid12;
	u_int32_t pad[3];
	u_int32_t tim12;
	u_int32_t tim34;
	u_int32_t prd12;
	u_int32_t prd34;
	u_int32_t tcr;
	u_int32_t tgcr;
	u_int32_t wdtcr;
};

#define regs ((struct timer_regs *)CONFIG_SYS_TIMERBASE)

#define TIMER_LOAD_VAL	(CONFIG_SYS_HZ_CLOCK / CONFIG_SYS_HZ)
#define TIM_CLK_DIV	16

static ulong timestamp;
static ulong lastinc;

int timer_init(void)
{
	clk_enable(TNETV107X_LPSC_TIMER0);

	lastinc = timestamp = 0;

	/* We are using timer34 in unchained 32-bit mode, full speed */
	__raw_writel(0x0, &regs->tcr);
	__raw_writel(0x0, &regs->tgcr);
	__raw_writel(0x06 | ((TIM_CLK_DIV - 1) << 8), &regs->tgcr);
	__raw_writel(0x0, &regs->tim34);
	__raw_writel(TIMER_LOAD_VAL, &regs->prd34);
	__raw_writel(2 << 22, &regs->tcr);

	return 0;
}

static ulong get_timer_raw(void)
{
	ulong now = __raw_readl(&regs->tim34);

	if (now >= lastinc)
		timestamp += now - lastinc;
	else
		timestamp += now + TIMER_LOAD_VAL - lastinc;

	lastinc = now;

	return timestamp;
}

ulong get_timer(ulong base)
{
	return (get_timer_raw() / (TIMER_LOAD_VAL / TIM_CLK_DIV)) - base;
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

void __udelay(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	tmo = CONFIG_SYS_HZ_CLOCK / 1000;
	tmo *= usec;
	tmo /= (1000 * TIM_CLK_DIV);

	endtime = get_timer_raw() + tmo;

	do {
		ulong now = get_timer_raw();
		diff = endtime - now;
	} while (diff >= 0);
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
