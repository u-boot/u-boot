// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <init.h>

int misc_init_r(void)
{
	return 0;
}

#ifndef CONFIG_SYS_COREBOOT
int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return 0;
}
#endif
