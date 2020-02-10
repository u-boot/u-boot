// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-17 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * Copyright (C) 2019 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <hang.h>
#include <irq_func.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/encoding.h>

static void show_regs(struct pt_regs *regs)
{
#ifdef CONFIG_SHOW_REGS
	printf("RA: " REG_FMT " SP:  " REG_FMT " GP:  " REG_FMT "\n",
	       regs->ra, regs->sp, regs->gp);
	printf("TP: " REG_FMT " T0:  " REG_FMT " T1:  " REG_FMT "\n",
	       regs->tp, regs->t0, regs->t1);
	printf("T2: " REG_FMT " S0:  " REG_FMT " S1:  " REG_FMT "\n",
	       regs->t2, regs->s0, regs->s1);
	printf("A0: " REG_FMT " A1:  " REG_FMT " A2:  " REG_FMT "\n",
	       regs->a0, regs->a1, regs->a2);
	printf("A3: " REG_FMT " A4:  " REG_FMT " A5:  " REG_FMT "\n",
	       regs->a3, regs->a4, regs->a5);
	printf("A6: " REG_FMT " A7:  " REG_FMT " S2:  " REG_FMT "\n",
	       regs->a6, regs->a7, regs->s2);
	printf("S3: " REG_FMT " S4:  " REG_FMT " S5:  " REG_FMT "\n",
	       regs->s3, regs->s4, regs->s5);
	printf("S6: " REG_FMT " S7:  " REG_FMT " S8:  " REG_FMT "\n",
	       regs->s6, regs->s7, regs->s8);
	printf("S9: " REG_FMT " S10: " REG_FMT " S11: " REG_FMT "\n",
	       regs->s9, regs->s10, regs->s11);
	printf("T3: " REG_FMT " T4:  " REG_FMT " T5:  " REG_FMT "\n",
	       regs->t3, regs->t4, regs->t5);
	printf("T6: " REG_FMT "\n", regs->t6);
#endif
}

static void _exit_trap(ulong code, ulong epc, ulong tval, struct pt_regs *regs)
{
	static const char * const exception_code[] = {
		"Instruction address misaligned",
		"Instruction access fault",
		"Illegal instruction",
		"Breakpoint",
		"Load address misaligned",
		"Load access fault",
		"Store/AMO address misaligned",
		"Store/AMO access fault",
		"Environment call from U-mode",
		"Environment call from S-mode",
		"Reserved",
		"Environment call from M-mode",
		"Instruction page fault",
		"Load page fault",
		"Reserved",
		"Store/AMO page fault",
	};

	if (code < ARRAY_SIZE(exception_code))
		printf("Unhandled exception: %s\n", exception_code[code]);
	else
		printf("Unhandled exception code: %ld\n", code);

	printf("EPC: " REG_FMT " TVAL: " REG_FMT "\n", epc, tval);
	show_regs(regs);
	hang();
}

int interrupt_init(void)
{
	return 0;
}

/*
 * enable interrupts
 */
void enable_interrupts(void)
{
}

/*
 * disable interrupts
 */
int disable_interrupts(void)
{
	return 0;
}

ulong handle_trap(ulong cause, ulong epc, ulong tval, struct pt_regs *regs)
{
	ulong is_irq, irq;

	is_irq = (cause & MCAUSE_INT);
	irq = (cause & ~MCAUSE_INT);

	if (is_irq) {
		switch (irq) {
		case IRQ_M_EXT:
		case IRQ_S_EXT:
			external_interrupt(0);	/* handle external interrupt */
			break;
		case IRQ_M_TIMER:
		case IRQ_S_TIMER:
			timer_interrupt(0);	/* handle timer interrupt */
			break;
		default:
			_exit_trap(cause, epc, tval, regs);
			break;
		};
	} else {
		_exit_trap(cause, epc, tval, regs);
	}

	return epc;
}

/*
 *Entry Point for PLIC Interrupt Handler
 */
__attribute__((weak)) void external_interrupt(struct pt_regs *regs)
{
}

__attribute__((weak)) void timer_interrupt(struct pt_regs *regs)
{
}
