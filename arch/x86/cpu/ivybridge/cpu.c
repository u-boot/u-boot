/*
 * Copyright (c) 2014 Google, Inc
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
 * Some portions from coreboot src/mainboard/google/link/romstage.c
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/cpu.h>
#include <asm/post.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
	int ret;

	post_code(POST_CPU_INIT);
	timer_set_base(rdtsc());

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	return 0;
}

int print_cpuinfo(void)
{
	char processor_name[CPU_MAX_NAME_LEN];
	const char *name;

	/* Print processor name */
	name = cpu_get_name(processor_name);
	printf("CPU:   %s\n", name);

	return 0;
}
