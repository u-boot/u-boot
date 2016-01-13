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
asm(".arch_extension sec\n\t");

static int mon_install(u32 addr, u32 dpsc, u32 freq)
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

static int do_mon_install(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	u32 addr, dpsc_base = 0x1E80000, freq;
	int     rcode = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	freq = CONFIG_SYS_HZ_CLOCK;

	addr = simple_strtoul(argv[1], NULL, 16);

	rcode = mon_install(addr, dpsc_base, freq);
	printf("## installed monitor, freq [%d], status %d\n",
	       freq, rcode);

	return 0;
}

U_BOOT_CMD(mon_install, 2, 0, do_mon_install,
	   "Install boot kernel at 'addr'",
	   ""
);

static void core_spin(void)
{
	while (1) {
		asm volatile (
			"dsb\n"
			"isb\n"
			"wfi\n"
		);
	}
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

int do_mon_power(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int     rcode = 0, core_id, on;
	void (*fn)(void);

	fn = core_spin;

	if (argc < 3)
		return CMD_RET_USAGE;

	core_id = simple_strtoul(argv[1], NULL, 16);
	on = simple_strtoul(argv[2], NULL, 16);

	if (on)
		rcode = mon_power_on(core_id, fn);
	else
		rcode = mon_power_off(core_id);

	if (on) {
		if (!rcode)
			printf("core %d powered on successfully\n", core_id);
		else
			printf("core %d power on failure\n", core_id);
	} else {
		printf("core %d powered off successfully\n", core_id);
	}

	return 0;
}

U_BOOT_CMD(mon_power, 3, 0, do_mon_power,
	   "Power On/Off secondary core",
	   "mon_power <coreid> <oper>\n"
	   "- coreid (1-3) and oper (1 - ON, 0 - OFF)\n"
	   ""
);
