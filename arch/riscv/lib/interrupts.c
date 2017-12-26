/*
 * Copyright (c) 2016-17 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/encoding.h>

static void _exit_trap(int code, uint epc, struct pt_regs *regs);

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

uint handle_trap(uint mcause, uint epc, struct pt_regs *regs)
{
	uint is_int;

	is_int = (mcause & MCAUSE_INT);
	if ((is_int) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_EXT))
		external_interrupt(0);	/* handle_m_ext_interrupt */
	else if ((is_int) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_TIMER))
		timer_interrupt(0);	/* handle_m_timer_interrupt */
	else
		_exit_trap(mcause, epc, regs);

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

static void _exit_trap(int code, uint epc, struct pt_regs *regs)
{
	static const char *exception_code[] = {
		"Instruction address misaligned",
		"Instruction access fault",
		"Illegal instruction",
		"Breakpoint",
		"Load address misaligned"
	};

	printf("exception code: %d , %s , epc %08x , ra %08lx\n",
		code, exception_code[code], epc, regs->ra);
}
