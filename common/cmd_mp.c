/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc.
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

#include <common.h>
#include <command.h>

int
cpu_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long cpuid;

	if (argc < 3)
		return cmd_usage(cmdtp);

	cpuid = simple_strtoul(argv[1], NULL, 10);
	if (cpuid >= cpu_numcores()) {
		printf ("Core num: %lu is out of range[0..%d]\n",
				cpuid, cpu_numcores() - 1);
		return 1;
	}


	if (argc == 3) {
		if (strncmp(argv[2], "reset", 5) == 0)
			cpu_reset(cpuid);
		else if (strncmp(argv[2], "status", 6) == 0)
			cpu_status(cpuid);
		else if (strncmp(argv[2], "disable", 7) == 0)
			return cpu_disable(cpuid);
		else
			return cmd_usage(cmdtp);

		return 0;
	}

	/* 4 or greater, make sure its release */
	if (strncmp(argv[2], "release", 7) != 0)
		return cmd_usage(cmdtp);

	if (cpu_release(cpuid, argc - 3, argv + 3))
		return cmd_usage(cmdtp);

	return 0;
}

#ifdef CONFIG_PPC
#define CPU_ARCH_HELP \
	"                         [args] : <pir> <r3> <r6>\n" \
	"                                   pir - processor id (if writeable)\n" \
	"                                    r3 - value for gpr 3\n" \
	"                                    r6 - value for gpr 6\n" \
	"\n" \
	"     Use '-' for any arg if you want the default value.\n" \
	"     Default for r3 is <num> and r6 is 0\n" \
	"\n" \
	"     When cpu <num> is released r4 and r5 = 0.\n" \
	"     r7 will contain the size of the initial mapped area"
#endif

U_BOOT_CMD(
	cpu, CONFIG_SYS_MAXARGS, 1, cpu_cmd,
	"Multiprocessor CPU boot manipulation and release",
	    "<num> reset                 - Reset cpu <num>\n"
	"cpu <num> status                - Status of cpu <num>\n"
	"cpu <num> disable               - Disable cpu <num>\n"
	"cpu <num> release <addr> [args] - Release cpu <num> at <addr> with [args]"
#ifdef CPU_ARCH_HELP
	"\n"
	CPU_ARCH_HELP
#endif
);
