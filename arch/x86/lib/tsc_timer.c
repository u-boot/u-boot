/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/i8254.h>
#include <asm/ibmpc.h>
#include <asm/msr.h>
#include <asm/u-boot-x86.h>

DECLARE_GLOBAL_DATA_PTR;

void timer_set_base(u64 base)
{
	gd->arch.tsc_base = base;
}

/*
 * Get the number of CPU time counter ticks since it was read first time after
 * restart. This yields a free running counter guaranteed to take almost 6
 * years to wrap around even at 100GHz clock rate.
 */
u64 __attribute__((no_instrument_function)) get_ticks(void)
{
	u64 now_tick = rdtsc();

	/* We assume that 0 means the base hasn't been set yet */
	if (!gd->arch.tsc_base)
		panic("No tick base available");
	return now_tick - gd->arch.tsc_base;
}

#define PLATFORM_INFO_MSR 0xce

/* Get the speed of the TSC timer in MHz */
unsigned __attribute__((no_instrument_function)) long get_tbclk_mhz(void)
{
	u32 ratio;
	u64 platform_info = native_read_msr(PLATFORM_INFO_MSR);

	/* 100MHz times Max Non Turbo ratio */
	ratio = (platform_info >> 8) & 0xff;
	return 100 * ratio;
}

unsigned long get_tbclk(void)
{
	return get_tbclk_mhz() * 1000 * 1000;
}

static ulong get_ms_timer(void)
{
	return (get_ticks() * 1000) / get_tbclk();
}

ulong get_timer(ulong base)
{
	return get_ms_timer() - base;
}

ulong __attribute__((no_instrument_function)) timer_get_us(void)
{
	return get_ticks() / get_tbclk_mhz();
}

ulong timer_get_boot_us(void)
{
	return timer_get_us();
}

void __udelay(unsigned long usec)
{
	u64 now = get_ticks();
	u64 stop;

	stop = now + usec * get_tbclk_mhz();

	while ((int64_t)(stop - get_ticks()) > 0)
		;
}

int timer_init(void)
{
#ifdef CONFIG_SYS_PCAT_TIMER
	/* Set up the PCAT timer if required */
	pcat_timer_init();
#endif

	return 0;
}
