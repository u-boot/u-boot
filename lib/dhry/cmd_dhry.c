/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include "dhry.h"

static int do_dhry(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong start, duration, dhry_per_sec, vax_mips;
	int iterations = 1000000;

	if (argc > 1)
		iterations = simple_strtoul(argv[1], NULL, 10);

	start = get_timer(0);
	dhry(iterations);
	duration = get_timer(start);
	dhry_per_sec = iterations * 1000 / duration;
	vax_mips = dhry_per_sec / 1757;
	printf("%d iterations in %lu ms: %lu/s, %lu DMIPS\n", iterations,
	       duration, dhry_per_sec, vax_mips);

	return 0;
}

U_BOOT_CMD(
	dhry,	2,	1,	do_dhry,
	"[iterations] - run dhrystone benchmark",
	"\n    - run the Dhrystone 2.1 benchmark, a rough measure of CPU speed\n"
);
