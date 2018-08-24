// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * (C) Copyright 2007-2012
 * Nobobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <sh_tmu.h>

#define TCR_TPSC 0x07

static struct tmu_regs *tmu = (struct tmu_regs *)TMU_BASE;

unsigned long get_tbclk(void)
{
	return get_tmu0_clk_rate() >> 2;
}

unsigned long timer_read_counter(void)
{
	return ~readl(&tmu->tcnt0);
}

static void tmu_timer_start(unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(&tmu->tstr) | (1 << timer), &tmu->tstr);
}

static void tmu_timer_stop(unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(&tmu->tstr) & ~(1 << timer), &tmu->tstr);
}

int timer_init(void)
{
	writew(readw(&tmu->tcr0) & ~TCR_TPSC, &tmu->tcr0);

	tmu_timer_stop(0);
	tmu_timer_start(0);

	return 0;
}

