// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#include <common.h>
#include <command.h>

static int do_test_stackprot_fail(struct cmd_tbl *cmdtp, int flag, int argc,
				  char *const argv[])
{
	char a[128];

	memset(a, 0xa5, 512);
	return 0;
}

U_BOOT_CMD(stackprot_test, 1, 1, do_test_stackprot_fail,
	   "test stack protector fail", "");
