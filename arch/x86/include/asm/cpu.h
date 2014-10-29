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

/**
 * cpu_call64() - Jump to a 64-bit Linux kernel (internal function)
 *
 * The kernel is uncompressed and the 64-bit entry point is expected to be
 * at @target.
 *
 * This function is used internally - see cpu_jump_to_64bit() for a more
 * useful function.
 *
 * @pgtable:	Address of 24KB area containing the page table
 * @setup_base:	Pointer to the setup.bin information for the kernel
 * @target:	Pointer to the start of the kernel image
 */
void cpu_call64(ulong pgtable, ulong setup_base, ulong target);

/**
 * cpu_jump_to_64bit() - Jump to a 64-bit Linux kernel
 *
 * The kernel is uncompressed and the 64-bit entry point is expected to be
 * at @target.
 *
 * @setup_base:	Pointer to the setup.bin information for the kernel
 * @target:	Pointer to the start of the kernel image
 */
int cpu_jump_to_64bit(ulong setup_base, ulong target);

#endif
