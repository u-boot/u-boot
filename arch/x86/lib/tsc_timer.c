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
	/* 2: use 100MHz, 1: use MSR_PLATFORM_INFO, 0: MSR_IA32_PERF_STATUS */
	u8 msr_plat;
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
	/* Ivybridge */
	{ 6, 0x3a, 2, { 0, 0, 0, 0, 0, 0, 0, 0 } },
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
static unsigned long __maybe_unused try_msr_calibrate_tsc(void)
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

	if (freq_desc_tables[cpu_index].msr_plat == 2) {
		/* TODO: Figure out how best to deal with this */
		freq = FREQ_100;
		debug("Using frequency: %u KHz\n", freq);
	} else {
		/* Get FSB FREQ ID */
		rdmsr(MSR_FSB_FREQ, lo, hi);
		freq_id = lo & 0x7;
		freq = id_to_freq(cpu_index, freq_id);
		debug("Resolved frequency ID: %u, frequency: %u KHz\n",
		      freq_id, freq);
	}
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

/*
 * This reads the current MSB of the PIT counter, and
 * checks if we are running on sufficiently fast and
 * non-virtualized hardware.
 *
 * Our expectations are:
 *
 *  - the PIT is running at roughly 1.19MHz
 *
 *  - each IO is going to take about 1us on real hardware,
 *    but we allow it to be much faster (by a factor of 10) or
 *    _slightly_ slower (ie we allow up to a 2us read+counter
 *    update - anything else implies a unacceptably slow CPU
 *    or PIT for the fast calibration to work.
 *
 *  - with 256 PIT ticks to read the value, we have 214us to
 *    see the same MSB (and overhead like doing a single TSC
 *    read per MSB value etc).
 *
 *  - We're doing 2 reads per loop (LSB, MSB), and we expect
 *    them each to take about a microsecond on real hardware.
 *    So we expect a count value of around 100. But we'll be
 *    generous, and accept anything over 50.
 *
 *  - if the PIT is stuck, and we see *many* more reads, we
 *    return early (and the next caller of pit_expect_msb()
 *    then consider it a failure when they don't see the
 *    next expected value).
 *
 * These expectations mean that we know that we have seen the
 * transition from one expected value to another with a fairly
 * high accuracy, and we didn't miss any events. We can thus
 * use the TSC value at the transitions to calculate a pretty
 * good value for the TSC frequencty.
 */
static inline int pit_verify_msb(unsigned char val)
{
	/* Ignore LSB */
	inb(0x42);
	return inb(0x42) == val;
}

static inline int pit_expect_msb(unsigned char val, u64 *tscp,
				 unsigned long *deltap)
{
	int count;
	u64 tsc = 0, prev_tsc = 0;

	for (count = 0; count < 50000; count++) {
		if (!pit_verify_msb(val))
			break;
		prev_tsc = tsc;
		tsc = rdtsc();
	}
	*deltap = rdtsc() - prev_tsc;
	*tscp = tsc;

	/*
	 * We require _some_ success, but the quality control
	 * will be based on the error terms on the TSC values.
	 */
	return count > 5;
}

/*
 * How many MSB values do we want to see? We aim for
 * a maximum error rate of 500ppm (in practice the
 * real error is much smaller), but refuse to spend
 * more than 50ms on it.
 */
#define MAX_QUICK_PIT_MS 50
#define MAX_QUICK_PIT_ITERATIONS (MAX_QUICK_PIT_MS * PIT_TICK_RATE / 1000 / 256)

static unsigned long __maybe_unused quick_pit_calibrate(void)
{
	int i;
	u64 tsc, delta;
	unsigned long d1, d2;

	/* Set the Gate high, disable speaker */
	outb((inb(0x61) & ~0x02) | 0x01, 0x61);

	/*
	 * Counter 2, mode 0 (one-shot), binary count
	 *
	 * NOTE! Mode 2 decrements by two (and then the
	 * output is flipped each time, giving the same
	 * final output frequency as a decrement-by-one),
	 * so mode 0 is much better when looking at the
	 * individual counts.
	 */
	outb(0xb0, 0x43);

	/* Start at 0xffff */
	outb(0xff, 0x42);
	outb(0xff, 0x42);

	/*
	 * The PIT starts counting at the next edge, so we
	 * need to delay for a microsecond. The easiest way
	 * to do that is to just read back the 16-bit counter
	 * once from the PIT.
	 */
	pit_verify_msb(0);

	if (pit_expect_msb(0xff, &tsc, &d1)) {
		for (i = 1; i <= MAX_QUICK_PIT_ITERATIONS; i++) {
			if (!pit_expect_msb(0xff-i, &delta, &d2))
				break;

			/*
			 * Iterate until the error is less than 500 ppm
			 */
			delta -= tsc;
			if (d1+d2 >= delta >> 11)
				continue;

			/*
			 * Check the PIT one more time to verify that
			 * all TSC reads were stable wrt the PIT.
			 *
			 * This also guarantees serialization of the
			 * last cycle read ('d2') in pit_expect_msb.
			 */
			if (!pit_verify_msb(0xfe - i))
				break;
			goto success;
		}
	}
	debug("Fast TSC calibration failed\n");
	return 0;

success:
	/*
	 * Ok, if we get here, then we've seen the
	 * MSB of the PIT decrement 'i' times, and the
	 * error has shrunk to less than 500 ppm.
	 *
	 * As a result, we can depend on there not being
	 * any odd delays anywhere, and the TSC reads are
	 * reliable (within the error).
	 *
	 * kHz = ticks / time-in-seconds / 1000;
	 * kHz = (t2 - t1) / (I * 256 / PIT_TICK_RATE) / 1000
	 * kHz = ((t2 - t1) * PIT_TICK_RATE) / (I * 256 * 1000)
	 */
	delta *= PIT_TICK_RATE;
	delta /= (i*256*1000);
	debug("Fast TSC calibration using PIT\n");
	return delta / 1000;
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

	if (gd->arch.tsc_mhz)
		return gd->arch.tsc_mhz;

#ifdef CONFIG_TSC_CALIBRATION_BYPASS
	fast_calibrate = CONFIG_TSC_FREQ_IN_MHZ;
#else
	fast_calibrate = try_msr_calibrate_tsc();
	if (!fast_calibrate) {

		fast_calibrate = quick_pit_calibrate();
		if (!fast_calibrate)
			panic("TSC frequency is ZERO");
	}
#endif

	gd->arch.tsc_mhz = fast_calibrate;
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
