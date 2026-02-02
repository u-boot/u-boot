// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * CPU specific code for m68040
 *
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#include <config.h>
#include <cpu_func.h>
#include <init.h>
#include <stdio.h>
#include <asm/global_data.h>
#include <linux/types.h>

DECLARE_GLOBAL_DATA_PTR;

void m68k_virt_init_reserve(ulong base)
{
	struct global_data *gd_ptr = (struct global_data *)base;
	char *p = (char *)gd_ptr;
	unsigned int i;

	/* FIXME: usage of memset() here caused a hang on QEMU m68k virt. */
	for (i = 0; i < sizeof(*gd_ptr); i++)
		p[i] = 0;

	gd = gd_ptr;

	gd->malloc_base = base + sizeof(*gd_ptr);
}

int print_cpuinfo(void)
{
	puts("CPU:   M68040 (QEMU Virt)\n");

	return 0;
}

int get_clocks(void)
{
	return 0;
}

int cpu_init_r(void)
{
	return 0;
}

/*
 * Relocation Stub
 * We skip actual relocation for this QEMU bring-up and jump directly
 * to board_init_r.
 */

void relocate_code(ulong sp, struct global_data *new_gd, ulong relocaddr)
{
	board_init_r(new_gd, relocaddr);
}

/* Stubs for Standard Facilities (Cache, Interrupts) */

int disable_interrupts(void) { return 0; }
void enable_interrupts(void) { return; }
int interrupt_init(void) { return 0; }

void icache_enable(void) {}
void icache_disable(void) {}
int icache_status(void) { return 0; }
void dcache_enable(void) {}
void dcache_disable(void) {}
int dcache_status(void) { return 0; }
void flush_cache(unsigned long start, unsigned long size) {}
void flush_dcache_range(unsigned long start, unsigned long stop) {}
