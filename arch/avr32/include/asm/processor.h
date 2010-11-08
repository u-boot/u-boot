/*
 * Copyright (C) 2006 Atmel Corporation
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
#ifndef __ASM_AVR32_PROCESSOR_H
#define __ASM_AVR32_PROCESSOR_H

#ifndef __ASSEMBLY__

#define current_text_addr() ({ void *pc; __asm__("mov %0,pc" : "=r"(pc)); pc; })

struct avr32_cpuinfo {
	unsigned long loops_per_jiffy;
};

extern struct avr32_cpuinfo boot_cpu_data;

#ifdef CONFIG_SMP
extern struct avr32_cpuinfo cpu_data[];
#define current_cpu_data cpu_data[smp_processor_id()]
#else
#define cpu_data (&boot_cpu_data)
#define current_cpu_data boot_cpu_data
#endif

/* TODO: Make configurable (2GB will serve as a reasonable default) */
#define TASK_SIZE	0x80000000

/* This decides where the kernel will search for a free chunk of vm
 * space during mmap's
 */
#define TASK_UNMAPPED_BASE	(TASK_SIZE / 3)

#define cpu_relax()		barrier()
#define cpu_sync_pipeline()	asm volatile("sub pc, -2" : : : "memory")

/* This struct contains the CPU context as stored by switch_to() */
struct thread_struct {
	unsigned long pc;
	unsigned long ksp;	/* Kernel stack pointer */
	unsigned long r7;
	unsigned long r6;
	unsigned long r5;
	unsigned long r4;
	unsigned long r3;
	unsigned long r2;
	unsigned long r1;
	unsigned long r0;
};

#define INIT_THREAD {						\
	.ksp = sizeof(init_stack) + (long)&init_stack,		\
}

/*
 * Do necessary setup to start up a newly executed thread.
 */
#define start_thread(regs, new_pc, new_sp)	 \
	set_fs(USER_DS);			 \
	regs->sr = 0;		/* User mode. */ \
	regs->gr[REG_PC] = new_pc;		 \
	regs->gr[REG_SP] = new_sp

struct task_struct;

/* Free all resources held by a thread */
extern void release_thread(struct task_struct *);

/* Create a kernel thread without removing it from tasklists */
extern int kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);

/* Prepare to copy thread state - unlazy all lazy status */
#define prepare_to_copy(tsk) do { } while(0)

/* Return saved PC of a blocked thread */
#define thread_saved_pc(tsk)    (tsk->thread.pc)

#endif /* __ASSEMBLY__ */

#endif /* __ASM_AVR32_PROCESSOR_H */
