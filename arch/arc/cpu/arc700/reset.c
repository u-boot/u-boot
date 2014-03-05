/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <command.h>
#include <common.h>

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printf("Put your restart handler here\n");

#ifdef DEBUG
	/* Stop debug session here */
	__asm__("brk");
#endif
	return 0;
}
