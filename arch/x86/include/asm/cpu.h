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

#endif
