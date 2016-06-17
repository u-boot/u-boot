/*
 * U-Boot - system.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _BLACKFIN_SYSTEM_H
#define _BLACKFIN_SYSTEM_H

/*
 * Interrupt configuring macros.
 */

extern int irq_flags;

#define local_irq_enable() \
	__asm__ __volatile__ ( \
		"sti %0;" \
		: \
		: "d" (irq_flags) \
	)

#define local_irq_disable() \
	do { \
		int __tmp_dummy; \
		__asm__ __volatile__ ( \
			"cli %0;" \
			: "=d" (__tmp_dummy) \
		); \
	} while (0)

# define local_irq_save(x) \
	__asm__ __volatile__ ( \
		"cli %0;" \
		: "=&d" (x) \
	)

#define local_save_flags(x) \
	__asm__ __volatile__ ( \
		"cli %0;" \
		"sti %0;" \
		: "=d" (x) \
	)

#define irqs_enabled_from_flags(x) ((x) != 0x1f)

#define local_irq_restore(x) \
	do { \
		if (irqs_enabled_from_flags(x)) \
			local_irq_enable(); \
	} while (0)

/*
 * Force strict CPU ordering.
 */
#define nop()			asm volatile ("nop;\n\t"::)
#define mb()			asm volatile (""   : : :"memory")
#define rmb()			asm volatile (""   : : :"memory")
#define wmb()			asm volatile (""   : : :"memory")
#define set_rmb(var, value)	do { xchg(&var, value); } while (0)
#define set_mb(var, value)	set_rmb(var, value)
#define set_wmb(var, value)	do { var = value; wmb(); } while (0)

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))

struct __xchg_dummy {
	unsigned long a[100];
};
#define __xg(x) ((volatile struct __xchg_dummy *)(x))

static inline unsigned long __xchg(unsigned long x, volatile void *ptr,
				   int size)
{
	unsigned long tmp = 0;
	unsigned long flags = 0;

	local_irq_save(flags);

	switch (size) {
	case 1:
		__asm__ __volatile__
			("%0 = b%2 (z);\n\t"
			 "b%2 = %1;\n\t"
			 : "=&d" (tmp) : "d" (x), "m" (*__xg(ptr)) : "memory");
		break;
	case 2:
		__asm__ __volatile__
			("%0 = w%2 (z);\n\t"
			 "w%2 = %1;\n\t"
			 : "=&d" (tmp) : "d" (x), "m" (*__xg(ptr)) : "memory");
		break;
	case 4:
		__asm__ __volatile__
			("%0 = %2;\n\t"
			 "%2 = %1;\n\t"
			 : "=&d" (tmp) : "d" (x), "m" (*__xg(ptr)) : "memory");
		break;
	}
	local_irq_restore(flags);
	return tmp;
}

void bfin_reset_boot_spi_cs(unsigned short pin);

#endif	/* _BLACKFIN_SYSTEM_H */
