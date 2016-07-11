/*
 * K2HK: secure kernel command file
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <mach/mon.h>
asm(".arch_extension sec\n\t");

int mon_install(u32 addr, u32 dpsc, u32 freq)
{
	int result;

	__asm__ __volatile__ (
		"stmfd r13!, {lr}\n"
		"mov r0, %1\n"
		"mov r1, %2\n"
		"mov r2, %3\n"
		"blx r0\n"
		"ldmfd r13!, {lr}\n"
		: "=&r" (result)
		: "r" (addr), "r" (dpsc), "r" (freq)
		: "cc", "r0", "r1", "r2", "memory");
	return result;
}

int mon_power_on(int core_id, void *ep)
{
	int result;

	asm volatile (
		"stmfd  r13!, {lr}\n"
		"mov r1, %1\n"
		"mov r2, %2\n"
		"mov r0, #0\n"
		"smc	#0\n"
		"ldmfd  r13!, {lr}\n"
		: "=&r" (result)
		: "r" (core_id), "r" (ep)
		: "cc", "r0", "r1", "r2", "memory");
	return  result;
}

int mon_power_off(int core_id)
{
	int result;

	asm volatile (
		"stmfd  r13!, {lr}\n"
		"mov r1, %1\n"
		"mov r0, #1\n"
		"smc	#1\n"
		"ldmfd  r13!, {lr}\n"
		: "=&r" (result)
		: "r" (core_id)
		: "cc", "r0", "r1", "memory");
	return  result;
}
