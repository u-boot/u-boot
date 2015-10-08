/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/nios2.h>
#include <asm/types.h>
#include <asm/io.h>

struct nios_timer {
	u32	status;		/* Timer status reg */
	u32	control;	/* Timer control reg */
	u32	periodl;	/* Timeout period low */
	u32	periodh;	/* Timeout period high */
	u32	snapl;		/* Snapshot low */
	u32	snaph;		/* Snapshot high */
};

/* status register */
#define NIOS_TIMER_TO		(1 << 0)	/* Timeout */
#define NIOS_TIMER_RUN		(1 << 1)	/* Timer running */

/* control register */
#define NIOS_TIMER_ITO		(1 << 0)	/* Timeout interrupt enable */
#define NIOS_TIMER_CONT		(1 << 1)	/* Continuous mode */
#define NIOS_TIMER_START	(1 << 2)	/* Start timer */
#define NIOS_TIMER_STOP		(1 << 3)	/* Stop timer */

/*************************************************************************/
unsigned long notrace timer_read_counter(void)
{
	struct nios_timer *tmr = (struct nios_timer *)CONFIG_SYS_TIMER_BASE;
	u32 val;

	/* Trigger update */
	writel(0x0, &tmr->snapl);

	/* Read timer value */
	val = readl(&tmr->snapl) & 0xffff;
	val |= (readl(&tmr->snaph) & 0xffff) << 16;

	return ~val;
}

int timer_init(void)
{
	struct nios_timer *tmr = (struct nios_timer *)CONFIG_SYS_TIMER_BASE;

	writel(0, &tmr->status);
	writel(0, &tmr->control);
	writel(NIOS_TIMER_STOP, &tmr->control);

	writel(0xffff, &tmr->periodl);
	writel(0xffff, &tmr->periodh);

	writel(NIOS_TIMER_CONT | NIOS_TIMER_START, &tmr->control);

	return 0;
}
