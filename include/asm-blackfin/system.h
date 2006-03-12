/*
 * U-boot - system.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _BLACKFIN_SYSTEM_H
#define _BLACKFIN_SYSTEM_H

#include <linux/config.h>	/* get configuration macros */
#include <asm/linkage.h>
#include <asm/blackfin.h>
#include <asm/segment.h>
#include <asm/entry.h>

#define prepare_to_switch()	do { } while(0)

/*
 * switch_to(n) should switch tasks to task ptr, first checking that
 * ptr isn't the current task, in which case it does nothing.  This
 * also clears the TS-flag if the task we switched to has used the
 * math co-processor latest.
 *
 * 05/25/01 - Tony Kou (tonyko@lineo.ca)
 *
 * Adapted for BlackFin (ADI) by Ted Ma, Metrowerks, and Motorola GSG
 * Copyright (c) 2002 Arcturus Networks Inc. (www.arcturusnetworks.com)
 * Copyright (c) 2003 Metrowerks (www.metrowerks.com)
 */

asmlinkage void resume(void);

#define switch_to(prev,next,last)	{					\
	void *_last;								\
	__asm__ __volatile__(							\
  			"r0 = %1;\n\t"						\
			"r1 = %2;\n\t"						\
			"call resume;\n\t" 					\
			"%0 = r0;\n\t"						\
			: "=d" (_last)						\
			: "d" (prev),						\
			"d" (next)						\
			: "CC", "R0", "R1", "R2", "R3", "R4", "R5", "P0", "P1");\
			(last) = _last;						\
}

/* Force kerenl switch to user mode -- Steven Chen */
#define switch_to_user_mode()	{						\
	__asm__ __volatile__(							\
			"call kernel_to_user_mode;\n\t"				\
			::							\
			: "CC", "R0", "R1", "R2", "R3", "R4", "R5", "P0", "P1");\
}

/*
 * Interrupt configuring macros.
 */

extern int irq_flags;

#define __sti()	{			\
	__asm__ __volatile__ (		\
		"r3 = %0;"		\
		"sti r3;"		\
		::"m"(irq_flags):"R3");	\
}

#define __cli()	{			\
	__asm__ __volatile__ (		\
		"cli r3;"		\
		:::"R3");		\
}

#define __save_flags(x)	{		\
	__asm__ __volatile__ (		\
		"cli r3;"		\
		"%0 = r3;"		\
		"sti r3;"		\
		::"m"(x):"R3");		\
}

#define __save_and_cli(x)	{	\
	__asm__ __volatile__ (          \
		"cli r3;"		\
		"%0 = r3;"		\
		::"m"(x):"R3");		\
}

#define __restore_flags(x) {		\
	__asm__ __volatile__ (		\
		"r3 = %0;"		\
		"sti r3;"		\
		::"m"(x):"R3");		\
}

/* For spinlocks etc */
#define local_irq_save(x)	__save_and_cli(x)
#define local_irq_restore(x)	__restore_flags(x)
#define local_irq_disable()	__cli()
#define local_irq_enable()	__sti()

#define cli()			__cli()
#define sti()			__sti()
#define save_flags(x)		__save_flags(x)
#define restore_flags(x)	__restore_flags(x)
#define save_and_cli(x)		__save_and_cli(x)

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

#ifdef CONFIG_SMP
#define smp_mb()		mb()
#define smp_rmb()		rmb()
#define smp_wmb()		wmb()
#else
#define smp_mb()		barrier()
#define smp_rmb()		barrier()
#define smp_wmb()		barrier()
#endif

#define xchg(ptr,x)		((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
#define tas(ptr)		(xchg((ptr),1))

struct __xchg_dummy {
	unsigned long a[100];
};
#define __xg(x)			((volatile struct __xchg_dummy *)(x))

static inline unsigned long __xchg(unsigned long x, volatile void *ptr,
				   int size)
{
	unsigned long tmp;
	unsigned long flags = 0;

	save_and_cli(flags);

	switch (size) {
	case 1:
	      __asm__ __volatile__("%0 = %2;\n\t" "%2 = %1;\n\t": "=&d"(tmp): "d"(x), "m"(*__xg(ptr)):"memory");
		break;
	case 2:
	      __asm__ __volatile__("%0 = %2;\n\t" "%2 = %1;\n\t": "=&d"(tmp): "d"(x), "m"(*__xg(ptr)):"memory");
		break;
	case 4:
	      __asm__ __volatile__("%0 = %2;\n\t" "%2 = %1;\n\t": "=&d"(tmp): "d"(x), "m"(*__xg(ptr)):"memory");
		break;
	}
	restore_flags(flags);
	return tmp;
}

/* Depend on whether Blackfin has hard reset function */
/* YES it does, but it is tricky to implement - FIXME later ...MaTed--- */
#define HARD_RESET_NOW() ({})

#endif	/* _BLACKFIN_SYSTEM_H */
