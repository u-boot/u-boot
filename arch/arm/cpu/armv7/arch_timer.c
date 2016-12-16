/*
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>
#include <bootstage.h>

DECLARE_GLOBAL_DATA_PTR;

int timer_init(void)
{
	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	gd->arch.timer_rate_hz = CONFIG_SYS_HZ_CLOCK / CONFIG_SYS_HZ;
	return 0;
}

unsigned long long get_ticks(void)
{
	ulong nowl, nowu;

	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (nowl), "=r" (nowu));

	gd->arch.tbl = nowl;
	gd->arch.tbu = nowu;

	return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}


ulong get_timer(ulong base)
{
	return lldiv(get_ticks(), gd->arch.timer_rate_hz) - base;
}

ulong timer_get_boot_us(void)
{
	return lldiv(get_ticks(), CONFIG_SYS_HZ_CLOCK / (CONFIG_SYS_HZ * 1000));
}

void __udelay(unsigned long usec)
{
	unsigned long long endtime;

	endtime = lldiv((unsigned long long)usec * gd->arch.timer_rate_hz,
			1000UL);

	endtime += get_ticks();

	while (get_ticks() < endtime)
		;
}

ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
