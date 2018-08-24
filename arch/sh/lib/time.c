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
#define TSTR_STR0	BIT(0)

static struct tmu_regs *tmu = (struct tmu_regs *)TMU_BASE;

unsigned long get_tbclk(void)
{
#ifdef CONFIG_RCAR_GEN2
	return CONFIG_SYS_CLK_FREQ / 8;
#else
	return CONFIG_SYS_CLK_FREQ / 4;
#endif
}

unsigned long timer_read_counter(void)
{
	return ~readl(&tmu->tcnt0);
}

int timer_init(void)
{
	writew(readw(&tmu->tcr0) & ~TCR_TPSC, &tmu->tcr0);
	writeb(readb(&tmu->tstr) & ~TSTR_STR0, &tmu->tstr);
	writeb(readb(&tmu->tstr) | TSTR_STR0, &tmu->tstr);

	return 0;
}

