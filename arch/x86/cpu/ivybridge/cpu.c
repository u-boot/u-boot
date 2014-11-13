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
#include <errno.h>
#include <fdtdec.h>
#include <asm/cpu.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/arch/pch.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
	const void *blob = gd->fdt_blob;
	struct pci_controller *hose;
	int node;
	int ret;

	post_code(POST_CPU_INIT);
	timer_set_base(rdtsc());

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	ret = pci_early_init_hose(&hose);
	if (ret)
		return ret;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INTEL_LPC);
	if (node < 0)
		return -ENOENT;
	ret = lpc_early_init(gd->fdt_blob, node, PCH_LPC_DEV);
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
