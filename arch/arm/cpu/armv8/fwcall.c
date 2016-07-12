/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <asm-offsets.h>
#include <config.h>
#include <version.h>
#include <asm/macro.h>
#include <asm/psci.h>
#include <asm/system.h>

/*
 * Issue the hypervisor call
 *
 * x0~x7: input arguments
 * x0~x3: output arguments
 */
void hvc_call(struct pt_regs *args)
{
	asm volatile(
		"ldr x0, %0\n"
		"ldr x1, %1\n"
		"ldr x2, %2\n"
		"ldr x3, %3\n"
		"ldr x4, %4\n"
		"ldr x5, %5\n"
		"ldr x6, %6\n"
		"ldr x7, %7\n"
		"hvc	#0\n"
		"str x0, %0\n"
		"str x1, %1\n"
		"str x2, %2\n"
		"str x3, %3\n"
		: "+m" (args->regs[0]), "+m" (args->regs[1]),
		  "+m" (args->regs[2]), "+m" (args->regs[3])
		: "m" (args->regs[4]), "m" (args->regs[5]),
		  "m" (args->regs[6]), "m" (args->regs[7])
		: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17");
}

/*
 * void smc_call(arg0, arg1...arg7)
 *
 * issue the secure monitor call
 *
 * x0~x7: input arguments
 * x0~x3: output arguments
 */

void smc_call(struct pt_regs *args)
{
	asm volatile(
		"ldr x0, %0\n"
		"ldr x1, %1\n"
		"ldr x2, %2\n"
		"ldr x3, %3\n"
		"ldr x4, %4\n"
		"ldr x5, %5\n"
		"ldr x6, %6\n"
		"smc	#0\n"
		"str x0, %0\n"
		"str x1, %1\n"
		"str x2, %2\n"
		"str x3, %3\n"
		: "+m" (args->regs[0]), "+m" (args->regs[1]),
		  "+m" (args->regs[2]), "+m" (args->regs[3])
		: "m" (args->regs[4]), "m" (args->regs[5]),
		  "m" (args->regs[6])
		: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17");
}

void __noreturn psci_system_reset(bool conduit_smc)
{
	struct pt_regs regs;

	regs.regs[0] = ARM_PSCI_0_2_FN_SYSTEM_RESET;

	if (conduit_smc)
		smc_call(&regs);
	else
		hvc_call(&regs);

	while (1)
		;
}
