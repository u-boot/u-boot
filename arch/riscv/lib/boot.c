/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long do_go_exec(ulong (*entry)(int, char * const []),
			 int argc, char * const argv[])
{
	cleanup_before_linux();

	return entry(argc, argv);
}
