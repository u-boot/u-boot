/*
 * From Coreboot file of same name
 *
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ARCH_ASM_LAPIC_H
#define _ARCH_ASM_LAPIC_H

#include <asm/io.h>
#include <asm/lapic_def.h>
#include <asm/msr.h>
#include <asm/processor.h>

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

	msr = msr_read(LAPIC_BASE_MSR);
	msr.hi &= 0xffffff00;
	msr.lo &= 0x000007ff;
	msr.lo |= LAPIC_DEFAULT_BASE | (1 << 11);
	msr_write(LAPIC_BASE_MSR, msr);
}

static inline void disable_lapic(void)
{
	msr_t msr;

	msr = msr_read(LAPIC_BASE_MSR);
	msr.lo &= ~(1 << 11);
	msr_write(LAPIC_BASE_MSR, msr);
}

static inline __attribute__((always_inline)) unsigned long lapicid(void)
{
	return lapic_read(LAPIC_ID) >> 24;
}

#endif
