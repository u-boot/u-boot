/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef __ASM_LOONGARCH_SYSTEM_H
#define __ASM_LOONGARCH_SYSTEM_H

#include <asm/loongarch.h>

struct event;

/*
 * Interrupt configuration macros
 */

static inline void arch_local_irq_enable(void)
{
	u32 flags = CSR_CRMD_IE;

	__asm__ __volatile__(
		"csrxchg %[val], %[mask], %[reg]\n\t"
		: [val] "+r" (flags)
		: [mask] "r" (CSR_CRMD_IE), [reg] "i" (LOONGARCH_CSR_CRMD)
		: "memory");
}

#define local_irq_enable arch_local_irq_enable

static inline void arch_local_irq_disable(void)
{
	u32 flags = 0;

	__asm__ __volatile__(
		"csrxchg %[val], %[mask], %[reg]\n\t"
		: [val] "+r" (flags)
		: [mask] "r" (CSR_CRMD_IE), [reg] "i" (LOONGARCH_CSR_CRMD)
		: "memory");
}

#define local_irq_disable arch_local_irq_disable

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags = 0;

	__asm__ __volatile__(
		"csrxchg %[val], %[mask], %[reg]\n\t"
		: [val] "+r" (flags)
		: [mask] "r" (CSR_CRMD_IE), [reg] "i" (LOONGARCH_CSR_CRMD)
		: "memory");
	return flags;
}

#define local_irq_save(__flags)                                 \
	do {                                                        \
		__flags = arch_local_irq_save(CSR_SSTATUS, SR_SIE) & SR_SIE; \
	} while (0)

static inline void arch_local_irq_restore(unsigned long flags)
{
	__asm__ __volatile__(
		"csrxchg %[val], %[mask], %[reg]\n\t"
		: [val] "+r" (flags)
		: [mask] "r" (CSR_CRMD_IE), [reg] "i" (LOONGARCH_CSR_CRMD)
		: "memory");
}

#define local_irq_restore(__flags)              \
	do {                                        \
		arch_local_irq_restore(__flags); \
	} while (0)

#endif
