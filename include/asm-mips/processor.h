/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 Waldorf GMBH
 * Copyright (C) 1995, 1996, 1997, 1998, 1999, 2001 Ralf Baechle
 * Copyright (C) 1996 Paul M. Antoine
 * Copyright (C) 1999 Silicon Graphics, Inc.
 */
#ifndef _ASM_PROCESSOR_H
#define _ASM_PROCESSOR_H

#include <linux/config.h>

#include <asm/isadep.h>

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#if !defined (_LANGUAGE_ASSEMBLY)
#if 0
#include <linux/threads.h>
#endif
#include <asm/cachectl.h>
#include <asm/mipsregs.h>
#include <asm/reg.h>
#include <asm/system.h>

struct mips_cpuinfo {
	unsigned long udelay_val;
	unsigned long *pgd_quick;
	unsigned long *pte_quick;
	unsigned long pgtable_cache_sz;
};

/*
 * System setup and hardware flags..
 * XXX: Should go into mips_cpuinfo.
 */
extern void (*cpu_wait)(void);	/* only available on R4[26]00 and R3081 */
extern void r3081_wait(void);
extern void r4k_wait(void);
extern char cyclecounter_available;	/* only available from R4000 upwards. */

extern struct mips_cpuinfo boot_cpu_data;
extern unsigned int vced_count, vcei_count;

#ifdef CONFIG_SMP
extern struct mips_cpuinfo cpu_data[];
#define current_cpu_data cpu_data[smp_processor_id()]
#else
#define cpu_data &boot_cpu_data
#define current_cpu_data boot_cpu_data
#endif

/*
 * Bus types (default is ISA, but people can check others with these..)
 * MCA_bus hardcoded to 0 for now.
 *
 * This needs to be extended since MIPS systems are being delivered with
 * numerous different types of bus systems.
 */
extern int EISA_bus;
#define MCA_bus 0
#define MCA_bus__is_a_macro /* for versions in ksyms.c */

/*
 * MIPS has no problems with write protection
 */
#define wp_works_ok 1
#define wp_works_ok__is_a_macro /* for versions in ksyms.c */

/* Lazy FPU handling on uni-processor */
extern struct task_struct *last_task_used_math;

/*
 * User space process size: 2GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.  TASK_SIZE
 * for a 64 bit kernel expandable to 8192EB, of which the current MIPS
 * implementations will "only" be able to use 1TB ...
 */
#define TASK_SIZE	(0x7fff8000UL)

/* This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE	(TASK_SIZE / 3)

/*
 * Size of io_bitmap in longwords: 32 is ports 0-0x3ff.
 */
#define IO_BITMAP_SIZE	32

#define NUM_FPU_REGS	32

struct mips_fpu_hard_struct {
	double fp_regs[NUM_FPU_REGS];
	unsigned int control;
};

/*
 * It would be nice to add some more fields for emulator statistics, but there
 * are a number of fixed offsets in offset.h and elsewhere that would have to
 * be recalculated by hand.  So the additional information will be private to
 * the FPU emulator for now.  See asm-mips/fpu_emulator.h.
 */
typedef u64 fpureg_t;
struct mips_fpu_soft_struct {
	fpureg_t	regs[NUM_FPU_REGS];
	unsigned int	sr;
};

union mips_fpu_union {
	struct mips_fpu_hard_struct hard;
	struct mips_fpu_soft_struct soft;
};

#define INIT_FPU { \
	{{0,},} \
}

typedef struct {
	unsigned long seg;
} mm_segment_t;

/*
 * If you change thread_struct remember to change the #defines below too!
 */
struct thread_struct {
	/* Saved main processor registers. */
	unsigned long reg16;
	unsigned long reg17, reg18, reg19, reg20, reg21, reg22, reg23;
	unsigned long reg29, reg30, reg31;

	/* Saved cp0 stuff. */
	unsigned long cp0_status;

	/* Saved fpu/fpu emulator stuff. */
	union mips_fpu_union fpu;

	/* Other stuff associated with the thread. */
	unsigned long cp0_badvaddr;	/* Last user fault */
	unsigned long cp0_baduaddr;	/* Last kernel fault accessing USEG */
	unsigned long error_code;
	unsigned long trap_no;
#define MF_FIXADE 1			/* Fix address errors in software */
#define MF_LOGADE 2			/* Log address errors to syslog */
	unsigned long mflags;
	mm_segment_t current_ds;
	unsigned long irix_trampoline;  /* Wheee... */
	unsigned long irix_oldctx;

	/*
	 * These are really only needed if the full FPU emulator is configured.
	 * Would be made conditional on MIPS_FPU_EMULATOR if it weren't for the
	 * fact that having offset.h rebuilt differently for different config
	 * options would be asking for trouble.
	 *
	 * Saved EPC during delay-slot emulation (see math-emu/cp1emu.c)
	 */
	unsigned long dsemul_epc;

	/*
	 * Pointer to instruction used to induce address error
	 */
	unsigned long dsemul_aerpc;
};

#endif /* !defined (_LANGUAGE_ASSEMBLY) */

#define INIT_THREAD  { \
	/* \
	 * saved main processor registers \
	 */ \
	0, 0, 0, 0, 0, 0, 0, 0, \
		       0, 0, 0, \
	/* \
	 * saved cp0 stuff \
	 */ \
	0, \
	/* \
	 * saved fpu/fpu emulator stuff \
	 */ \
	INIT_FPU, \
	/* \
	 * Other stuff associated with the process \
	 */ \
	0, 0, 0, 0, \
	/* \
	 * For now the default is to fix address errors \
	 */ \
	MF_FIXADE, { 0 }, 0, 0, \
	/* \
	 * dsemul_epc and dsemul_aerpc should never be used uninitialized, \
	 * but... \
	 */ \
	0 ,0 \
}

#ifdef __KERNEL__

#define KERNEL_STACK_SIZE 8192

#if !defined (_LANGUAGE_ASSEMBLY)

/* Free all resources held by a thread. */
#define release_thread(thread) do { } while(0)

extern int kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);

/* Copy and release all segment info associated with a VM */
#define copy_segments(p, mm) do { } while(0)
#define release_segments(mm) do { } while(0)

/*
 * Return saved PC of a blocked thread.
 */
extern inline unsigned long thread_saved_pc(struct thread_struct *t)
{
	extern void ret_from_fork(void);

	/* New born processes are a special case */
	if (t->reg31 == (unsigned long) ret_from_fork)
		return t->reg31;

	return ((unsigned long *)t->reg29)[10];
}

/*
 * Do necessary setup to start up a newly executed thread.
 */
#define start_thread(regs, new_pc, new_sp) do {				\
	/* New thread looses kernel privileges. */			\
	regs->cp0_status = (regs->cp0_status & ~(ST0_CU0|ST0_KSU)) | KU_USER;\
	regs->cp0_epc = new_pc;						\
	regs->regs[29] = new_sp;					\
	current->thread.current_ds = USER_DS;				\
} while (0)

unsigned long get_wchan(struct task_struct *p);

#define __PT_REG(reg) ((long)&((struct pt_regs *)0)->reg - sizeof(struct pt_regs))
#define __KSTK_TOS(tsk) ((unsigned long)(tsk) + KERNEL_STACK_SIZE - 32)
#define KSTK_EIP(tsk) (*(unsigned long *)(__KSTK_TOS(tsk) + __PT_REG(cp0_epc)))
#define KSTK_ESP(tsk) (*(unsigned long *)(__KSTK_TOS(tsk) + __PT_REG(regs[29])))

/* Allocation and freeing of basic task resources. */
/*
 * NOTE! The task struct and the stack go together
 */
#define THREAD_SIZE (2*PAGE_SIZE)
#define alloc_task_struct() \
	((struct task_struct *) __get_free_pages(GFP_KERNEL,1))
#define free_task_struct(p)	free_pages((unsigned long)(p),1)
#define get_task_struct(tsk)      atomic_inc(&virt_to_page(tsk)->count)

#define init_task	(init_task_union.task)
#define init_stack	(init_task_union.stack)

#define cpu_relax()	do { } while (0)

#endif /* !defined (_LANGUAGE_ASSEMBLY) */
#endif /* __KERNEL__ */

/*
 * Return_address is a replacement for __builtin_return_address(count)
 * which on certain architectures cannot reasonably be implemented in GCC
 * (MIPS, Alpha) or is unuseable with -fomit-frame-pointer (i386).
 * Note that __builtin_return_address(x>=1) is forbidden because GCC
 * aborts compilation on some CPUs.  It's simply not possible to unwind
 * some CPU's stackframes.
 *
 * __builtin_return_address works only for non-leaf functions.  We avoid the
 * overhead of a function call by forcing the compiler to save the return
 * address register on the stack.
 */
#define return_address() ({__asm__ __volatile__("":::"$31");__builtin_return_address(0);})

#endif /* _ASM_PROCESSOR_H */
