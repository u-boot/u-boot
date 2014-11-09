/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * TSC calibration codes are adapted from Linux kernel
 * arch/x86/kernel/tsc_msr.c and arch/x86/kernel/tsc.c
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

/* CPU reference clock frequency: in KHz */
#define FREQ_83		83200
#define FREQ_100	99840
#define FREQ_133	133200
#define FREQ_166	166400

#define MAX_NUM_FREQS	8

DECLARE_GLOBAL_DATA_PTR;

/*
 * According to Intel 64 and IA-32 System Programming Guide,
 * if MSR_PERF_STAT[31] is set, the maximum resolved bus ratio can be
 * read in MSR_PLATFORM_ID[12:8], otherwise in MSR_PERF_STAT[44:40].
 * Unfortunately some Intel Atom SoCs aren't quite compliant to this,
 * so we need manually differentiate SoC families. This is what the
 * field msr_plat does.
 */
struct freq_desc {
	u8 x86_family;	/* CPU family */
	u8 x86_model;	/* model */
	u8 msr_plat;	/* 1: use MSR_PLATFORM_INFO, 0: MSR_IA32_PERF_STATUS */
	u32 freqs[MAX_NUM_FREQS];
};

static struct freq_desc freq_desc_tables[] = {
	/* PNW */
	{ 6, 0x27, 0, { 0, 0, 0, 0, 0, FREQ_100, 0, FREQ_83 } },
	/* CLV+ */
	{ 6, 0x35, 0, { 0, FREQ_133, 0, 0, 0, FREQ_100, 0, FREQ_83 } },
	/* TNG */
	{ 6, 0x4a, 1, { 0, FREQ_100, FREQ_133, 0, 0, 0, 0, 0 } },
	/* VLV2 */
	{ 6, 0x37, 1, { FREQ_83, FREQ_100, FREQ_133, FREQ_166, 0, 0, 0, 0 } },
	/* ANN */
	{ 6, 0x5a, 1, { FREQ_83, FREQ_100, FREQ_133, FREQ_100, 0, 0, 0, 0 } },
};

static int match_cpu(u8 family, u8 model)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(freq_desc_tables); i++) {
		if ((family == freq_desc_tables[i].x86_family) &&
		    (model == freq_desc_tables[i].x86_model))
			return i;
	}

	return -1;
}

/* Map CPU reference clock freq ID(0-7) to CPU reference clock freq(KHz) */
#define id_to_freq(cpu_index, freq_id) \
	(freq_desc_tables[cpu_index].freqs[freq_id])

/*
 * Do MSR calibration only for known/supported CPUs.
 *
 * Returns the calibration value or 0 if MSR calibration failed.
 */
static unsigned long try_msr_calibrate_tsc(void)
{
	u32 lo, hi, ratio, freq_id, freq;
	unsigned long res;
	int cpu_index;

	cpu_index = match_cpu(gd->arch.x86, gd->arch.x86_model);
	if (cpu_index < 0)
		return 0;

	if (freq_desc_tables[cpu_index].msr_plat) {
		rdmsr(MSR_PLATFORM_INFO, lo, hi);
		ratio = (lo >> 8) & 0x1f;
	} else {
		rdmsr(MSR_IA32_PERF_STATUS, lo, hi);
		ratio = (hi >> 8) & 0x1f;
	}
	debug("Maximum core-clock to bus-clock ratio: 0x%x\n", ratio);

	if (!ratio)
		goto fail;

	/* Get FSB FREQ ID */
	rdmsr(MSR_FSB_FREQ, lo, hi);
	freq_id = lo & 0x7;
	freq = id_to_freq(cpu_index, freq_id);
	debug("Resolved frequency ID: %u, frequency: %u KHz\n", freq_id, freq);
	if (!freq)
		goto fail;

	/* TSC frequency = maximum resolved freq * maximum resolved bus ratio */
	res = freq * ratio / 1000;
	debug("TSC runs at %lu MHz\n", res);

	return res;

fail:
	debug("Fast TSC calibration using MSR failed\n");
	return 0;
}

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

/* Get the speed of the TSC timer in MHz */
unsigned __attribute__((no_instrument_function)) long get_tbclk_mhz(void)
{
	unsigned long fast_calibrate;

	fast_calibrate = try_msr_calibrate_tsc();
	if (!fast_calibrate)
		panic("TSC frequency is ZERO");

	return fast_calibrate;
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
