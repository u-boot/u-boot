// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <command.h>

int arch_misc_init(void)
{
	return 0;
}

/* This function is needed if CONFIG_CMDLINE is not enabled */
int board_run_command(const char *cmdline)
{
	printf("No command line\n");

	return 0;
}
