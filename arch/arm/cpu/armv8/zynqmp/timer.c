/*
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2011-2012 Xilinx, Inc. All rights reserved.
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
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
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

struct ttc0_timer {
	u32 clkctrl; /* Timer Load Register */
	u32 reserved0[2];
	u32 cntctrl; /* Timer Counter Register */
	u32 reserved1[2];
	u32 counter; /* Timer Control Register */
};

static struct ttc0_timer *timer_base =
			      (struct ttc0_timer *)ZYNQ_TTC_BASEADDR0;

#define TIMER_LOAD_VAL 0xFFFFFFFF

#define PRESCALE_EXPONENT	11	/* 2 ^ PRESCALE_EXPONENT = PRESCALE */
#define PRESCALE		2048	/* The exponent must match this */
#define CLK_CNTRL_PRESCALE	((PRESCALE_EXPONENT - 1) << 1)
#define CLK_CNTRL_PRESCALE_EN	1
#define CNT_CNTRL_RESET		(1 << 4)

int timer_init(void)
{
	gd->arch.timer_rate_hz = get_ttc_clk(0) / PRESCALE;

	writel(CLK_CNTRL_PRESCALE | CLK_CNTRL_PRESCALE_EN,
	       &timer_base->clkctrl);
	writel(CNT_CNTRL_RESET, &timer_base->cntctrl);

	/* Reset time */
	gd->arch.lastinc = readl(&timer_base->counter) /
				(gd->arch.timer_rate_hz / CONFIG_SYS_HZ);
	gd->arch.tbl = 0;

	return 0;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
ulong get_timer_masked(void)
{
	ulong now;

	now = readl(&timer_base->counter) /
			(gd->arch.timer_rate_hz / CONFIG_SYS_HZ);

	if (gd->arch.lastinc <= now) {
		/* Normal mode */
		gd->arch.tbl += now - gd->arch.lastinc;
	} else {
		/* We have an overflow ... */
		gd->arch.tbl += now + TIMER_LOAD_VAL - gd->arch.lastinc + 1;
	}
	gd->arch.lastinc = now;

	return gd->arch.tbl;
}
void __udelay(unsigned long usec)
{
	u32 countticks;
	u32 timeend;
	u32 timediff;
	u32 timenow;

	if (usec == 0)
		return;

	countticks = lldiv(((unsigned long long)gd->arch.timer_rate_hz * usec),
			   1000000);

	/* decrementing timer */
	timeend = readl(&timer_base->counter) + countticks;

#if TIMER_LOAD_VAL != 0xFFFFFFFF
	/* do not manage multiple overflow */
	if (countticks >= TIMER_LOAD_VAL)
		countticks = TIMER_LOAD_VAL - 1;
#endif

	do {
		timenow = readl(&timer_base->counter);

		if (timenow <= timeend) {
			/* normal case */
			timediff = timeend - timenow;
		} else {
			if ((TIMER_LOAD_VAL - timenow + timeend) <=
								countticks) {
				/* overflow */
				timediff = TIMER_LOAD_VAL - timenow + timeend;
			} else {
				/* missed the exact match */
				break;
			}
		}
	} while (timediff > 0);
}

/* Timer without interrupts */
ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
