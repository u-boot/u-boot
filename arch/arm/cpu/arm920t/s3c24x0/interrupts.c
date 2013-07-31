/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/arch/s3c24x0_cpu.h>
#include <asm/proc-armv/ptrace.h>

void do_irq (struct pt_regs *pt_regs)
{
	struct s3c24x0_interrupt *irq = s3c24x0_get_base_interrupt();
	u_int32_t intpnd = readl(&irq->INTPND);

}
