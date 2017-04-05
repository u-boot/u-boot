/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Generic timer implementation of get_tbclk()
 */
unsigned long get_tbclk(void)
{
	unsigned long cntfrq;
	asm volatile("mrs %0, cntfrq_el0" : "=r" (cntfrq));
	return cntfrq;
}

/*
 * Generic timer implementation of timer_read_counter()
 */
unsigned long timer_read_counter(void)
{
	unsigned long cntpct;
#ifdef CONFIG_SYS_FSL_ERRATUM_A008585
	/* This erratum number needs to be confirmed to match ARM document */
	unsigned long temp;
#endif
	isb();
	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));
#ifdef CONFIG_SYS_FSL_ERRATUM_A008585
	asm volatile("mrs %0, cntpct_el0" : "=r" (temp));
	while (temp != cntpct) {
		asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));
		asm volatile("mrs %0, cntpct_el0" : "=r" (temp));
	}
#endif
	return cntpct;
}

uint64_t get_ticks(void)
{
	unsigned long ticks = timer_read_counter();

	gd->arch.tbl = ticks;

	return ticks;
}

unsigned long usec2ticks(unsigned long usec)
{
	ulong ticks;
	if (usec < 1000)
		ticks = ((usec * (get_tbclk()/1000)) + 500) / 1000;
	else
		ticks = ((usec / 10) * (get_tbclk() / 100000));

	return ticks;
}
