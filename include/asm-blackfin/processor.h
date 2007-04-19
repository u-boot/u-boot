/*
 * U-boot - processor.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file is based on
 * include/asm-m68k/processor.h
 * Changes made by Akbar Hussain Lineo, Inc, May 2001 for BLACKFIN
 * Copyright (C) 1995 Hamish Macdonald
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __ASM_BLACKFIN_PROCESSOR_H
#define __ASM_BLACKFIN_PROCESSOR_H

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr()	({ __label__ _l; _l: &&_l;})

#include <linux/config.h>
#include <asm/segment.h>
#include <asm/ptrace.h>
#include <asm/current.h>

extern inline unsigned long rdusp(void)
{
	unsigned long usp;

	__asm__ __volatile__("%0 = usp;\n\t":"=da"(usp));
	return usp;
}

extern inline void wrusp(unsigned long usp)
{
	__asm__ __volatile__("usp = %0;\n\t"::"da"(usp));
}

/*
 * User space process size: 3.75GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE		(0xF0000000UL)

/*
 * Bus types
 */
#define EISA_bus		0
#define MCA_bus			0

/*  There is no pc register avaliable for BLACKFIN, so we are going to get
 *  it indirectly
 */

#if 0
inline unsigned long obtain_pc_indirectly(void)
{
	unsigned long pc;
	__asm__ __volatile__("%0 = rets;\n":"=d"(pc));
	return (pc - 4);	/* call pcrel24 is 4 bytes long  */
}
#endif

/*
 * if you change this structure, you must change the code and offsets
 * in m68k/machasm.S
 */

struct thread_struct {
	unsigned long ksp;	/* kernel stack pointer */
	unsigned long usp;	/* user stack pointer */
	unsigned short seqstat;	/* saved status register */
	unsigned long esp0;	/* points to SR of stack frame pt_regs */
	unsigned long pc;	/* instruction pointer */
};

#define INIT_MMAP { &init_mm, 0, 0x40000000, NULL, __pgprot(_PAGE_PRESENT|_PAGE_ACCESSED), VM_READ | VM_WRITE | VM_EXEC, 1, NULL, NULL }

#define INIT_THREAD  { \
	sizeof(init_stack) + (unsigned long) init_stack, 0, \
	PS_S, 0\
}

/*
 * Do necessary setup to start up a newly executed thread.
 *
 * pass the data segment into user programs if it exists,
 * it can't hurt anything as far as I can tell
 */
#define start_thread(_regs, _pc, _usp)           \
do {                                             \
	set_fs(USER_DS); /* reads from user space */ \
	(_regs)->pc = (_pc);                         \
	if (current->mm)                             \
		(_regs)->r5 = current->mm->start_data;   \
	(_regs)->seqstat &= ~0x0c00;                      \
	wrusp(_usp);                                 \
	/* Adde by HuTao, May 26, 2003 3:39PM */\
	if ((_regs)->ipend & 0x8000) /* check whether system in supper mode - StChen */\
		(_regs)->ipend = 0x0;\
} while(0)

/* Forward declaration, a strange C thing */
struct task_struct;

/* Free all resources held by a thread. */
static inline void release_thread(struct task_struct *dead_task)
{
}

extern int kernel_thread(int (*fn) (void *), void *arg, unsigned long flags);

#define copy_segments(tsk, mm)		do { } while (0)
#define release_segments(mm)		do { } while (0)
#define forget_segments()		do { } while (0)

/*
 * Free current thread data structures etc..
 */
static inline void exit_thread(void)
{
}

/*
 * Return saved PC of a blocked thread.
 */
extern inline unsigned long thread_saved_pc(struct thread_struct *t)
{
	extern void scheduling_functions_start_here(void);
	extern void scheduling_functions_end_here(void);
	return 0;
}

unsigned long get_wchan(struct task_struct *p);

#define	KSTK_EIP(tsk)	\
	({			\
	unsigned long eip = 0;	 \
	if ((tsk)->thread.esp0 > PAGE_SIZE && \
		MAP_NR((tsk)->thread.esp0) < max_mapnr) \
		eip = ((struct pt_regs *) (tsk)->thread.esp0)->pc; \
	eip; })
#define	KSTK_ESP(tsk)	((tsk) == current ? rdusp() : (tsk)->thread.usp)
#define THREAD_SIZE	(2*PAGE_SIZE)

/* Allocation and freeing of basic task resources. */
#define alloc_task_struct() \
	((struct task_struct *) __get_free_pages(GFP_KERNEL,1))
#define free_task_struct(p)	free_pages((unsigned long)(p),1)
#define get_task_struct(tsk)	atomic_inc(&mem_map[MAP_NR(tsk)].count)

#define init_task		(init_task_union.task)
#define init_stack		(init_task_union.stack)

#endif
