// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#include <config.h>
#include <init.h>
#include <time.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/timer_defs.h>
#include <div64.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

static struct davinci_timer * const timer =
	(struct davinci_timer *)CFG_SYS_TIMERBASE;

#define TIMER_LOAD_VAL	0xffffffff

#define TIM_CLK_DIV	16

int timer_init(void)
{
	/* We are using timer34 in unchained 32-bit mode, full speed */
	writel(0x0, &timer->tcr);
	writel(0x0, &timer->tgcr);
	writel(0x06 | ((TIM_CLK_DIV - 1) << 8), &timer->tgcr);
	writel(0x0, &timer->tim34);
	writel(TIMER_LOAD_VAL, &timer->prd34);
	writel(2 << 22, &timer->tcr);
	gd->arch.timer_rate_hz = CFG_SYS_HZ_CLOCK / TIM_CLK_DIV;
	gd->arch.timer_reset_value = 0;

	return(0);
}

/*
 * Get the current 64 bit timer tick count
 */
unsigned long long get_ticks(void)
{
	unsigned long now = readl(&timer->tim34);

	/* increment tbu if tbl has rolled over */
	if (now < gd->arch.tbl)
		gd->arch.tbu++;
	gd->arch.tbl = now;

	return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}

ulong get_timer(ulong base)
{
	unsigned long long timer_diff;

	timer_diff = get_ticks() - gd->arch.timer_reset_value;

	return lldiv(timer_diff,
		     (gd->arch.timer_rate_hz / CONFIG_SYS_HZ)) - base;
}

void __udelay(unsigned long usec)
{
	unsigned long long endtime;

	endtime = lldiv((unsigned long long)usec * gd->arch.timer_rate_hz,
			1000000UL);
	endtime += get_ticks();

	while (get_ticks() < endtime)
		;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
