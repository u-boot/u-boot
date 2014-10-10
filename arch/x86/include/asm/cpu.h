/*
 * Copyright (c) 2014 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __X86_CPU_H
#define __X86_CPU_H

 /**
 * cpu_enable_paging_pae() - Enable PAE-paging
 *
 * @pdpt:	Value to set in cr3 (PDPT or PML4T)
 */
void cpu_enable_paging_pae(ulong cr3);

/**
 * cpu_disable_paging_pae() - Disable paging and PAE
 */
void cpu_disable_paging_pae(void);

/**
 * cpu_has_64bit() - Check if the CPU has 64-bit support
 *
 * @return 1 if this CPU supports long mode (64-bit), 0 if not
 */
int cpu_has_64bit(void);

#endif
