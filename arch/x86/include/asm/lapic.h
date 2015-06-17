/*
 * From coreboot file of same name
 *
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ARCH_ASM_LAPIC_H
#define _ARCH_ASM_LAPIC_H

#include <asm/io.h>
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <asm/processor.h>

#define LAPIC_DEFAULT_BASE		0xfee00000

#define LAPIC_ID			0x020
#define LAPIC_LVR			0x030

#define LAPIC_TASKPRI			0x080
#define LAPIC_TPRI_MASK			0xff

#define LAPIC_RRR			0x0c0

#define LAPIC_SPIV			0x0f0
#define LAPIC_SPIV_ENABLE		0x100

#define LAPIC_ICR			0x300
#define LAPIC_DEST_SELF			0x40000
#define LAPIC_DEST_ALLINC		0x80000
#define LAPIC_DEST_ALLBUT		0xc0000
#define LAPIC_ICR_RR_MASK		0x30000
#define LAPIC_ICR_RR_INVALID		0x00000
#define LAPIC_ICR_RR_INPROG		0x10000
#define LAPIC_ICR_RR_VALID		0x20000
#define LAPIC_INT_LEVELTRIG		0x08000
#define LAPIC_INT_ASSERT		0x04000
#define LAPIC_ICR_BUSY			0x01000
#define LAPIC_DEST_LOGICAL		0x00800
#define LAPIC_DM_FIXED			0x00000
#define LAPIC_DM_LOWEST			0x00100
#define LAPIC_DM_SMI			0x00200
#define LAPIC_DM_REMRD			0x00300
#define LAPIC_DM_NMI			0x00400
#define LAPIC_DM_INIT			0x00500
#define LAPIC_DM_STARTUP		0x00600
#define LAPIC_DM_EXTINT			0x00700
#define LAPIC_VECTOR_MASK		0x000ff

#define LAPIC_ICR2			0x310
#define GET_LAPIC_DEST_FIELD(x)		(((x) >> 24) & 0xff)
#define SET_LAPIC_DEST_FIELD(x)		((x) << 24)

#define LAPIC_LVT0			0x350
#define LAPIC_LVT1			0x360
#define LAPIC_LVT_MASKED		(1 << 16)
#define LAPIC_LVT_LEVEL_TRIGGER		(1 << 15)
#define LAPIC_LVT_REMOTE_IRR		(1 << 14)
#define LAPIC_INPUT_POLARITY		(1 << 13)
#define LAPIC_SEND_PENDING		(1 << 12)
#define LAPIC_LVT_RESERVED_1		(1 << 11)
#define LAPIC_DELIVERY_MODE_MASK	(7 << 8)
#define LAPIC_DELIVERY_MODE_FIXED	(0 << 8)
#define LAPIC_DELIVERY_MODE_NMI		(4 << 8)
#define LAPIC_DELIVERY_MODE_EXTINT	(7 << 8)

static inline __attribute__((always_inline))
		unsigned long lapic_read(unsigned long reg)
{
	return readl(LAPIC_DEFAULT_BASE + reg);
}

static inline __attribute__((always_inline))
		void lapic_write(unsigned long reg, unsigned long val)
{
	writel(val, LAPIC_DEFAULT_BASE + reg);
}

static inline __attribute__((always_inline)) void lapic_wait_icr_idle(void)
{
	do { } while (lapic_read(LAPIC_ICR) & LAPIC_ICR_BUSY);
}

static inline void enable_lapic(void)
{
	msr_t msr;

	msr = msr_read(MSR_IA32_APICBASE);
	msr.hi &= 0xffffff00;
	msr.lo |= MSR_IA32_APICBASE_ENABLE;
	msr.lo &= ~MSR_IA32_APICBASE_BASE;
	msr.lo |= LAPIC_DEFAULT_BASE;
	msr_write(MSR_IA32_APICBASE, msr);
}

static inline void disable_lapic(void)
{
	msr_t msr;

	msr = msr_read(MSR_IA32_APICBASE);
	msr.lo &= ~MSR_IA32_APICBASE_ENABLE;
	msr_write(MSR_IA32_APICBASE, msr);
}

static inline __attribute__((always_inline)) unsigned long lapicid(void)
{
	return lapic_read(LAPIC_ID) >> 24;
}

static inline __attribute__((always_inline)) void stop_this_cpu(void)
{
	/* Called by an AP when it is ready to halt and wait for a new task */
	for (;;)
		cpu_hlt();
}

#define xchg(ptr, v)	((__typeof__(*(ptr)))__xchg((unsigned long)(v), (ptr), \
						    sizeof(*(ptr))))

struct __xchg_dummy	{ unsigned long a[100]; };
#define __xg(x)		((struct __xchg_dummy *)(x))

/*
 * Note: no "lock" prefix even on SMP. xchg always implies lock anyway.
 *
 * Note 2: xchg has side effect, so that attribute volatile is necessary,
 *         but generally the primitive is invalid, *ptr is output argument.
 */
static inline unsigned long __xchg(unsigned long x, volatile void *ptr,
				   int size)
{
	switch (size) {
	case 1:
		__asm__ __volatile__("xchgb %b0,%1"
			: "=q" (x)
			: "m" (*__xg(ptr)), "0" (x)
			: "memory");
		break;
	case 2:
		__asm__ __volatile__("xchgw %w0,%1"
			: "=r" (x)
			: "m" (*__xg(ptr)), "0" (x)
			: "memory");
		break;
	case 4:
		__asm__ __volatile__("xchgl %0,%1"
			: "=r" (x)
			: "m" (*__xg(ptr)), "0" (x)
			: "memory");
		break;
	}

	return x;
}

static inline void lapic_write_atomic(unsigned long reg, unsigned long v)
{
	(void)xchg((volatile unsigned long *)(LAPIC_DEFAULT_BASE + reg), v);
}

#define lapic_read_around(x)		lapic_read(x)
#define lapic_write_around(x, y)	lapic_write_atomic((x), (y))

static inline int lapic_remote_read(int apicid, int reg, unsigned long *pvalue)
{
	int timeout;
	unsigned long status;
	int result;

	lapic_wait_icr_idle();
	lapic_write_around(LAPIC_ICR2, SET_LAPIC_DEST_FIELD(apicid));
	lapic_write_around(LAPIC_ICR, LAPIC_DM_REMRD | (reg >> 4));

	timeout = 0;
	do {
		status = lapic_read(LAPIC_ICR) & LAPIC_ICR_RR_MASK;
	} while (status == LAPIC_ICR_RR_INPROG && timeout++ < 1000);

	result = -1;
	if (status == LAPIC_ICR_RR_VALID) {
		*pvalue = lapic_read(LAPIC_RRR);
		result = 0;
	}

	return result;
}

void lapic_setup(void);

#endif
