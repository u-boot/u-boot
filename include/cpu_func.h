/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Copyright 2019 Google LLC
 */

#ifndef __CPU_LEGACY_H
#define __CPU_LEGACY_H

#include <linux/types.h>

/*
 * Multicore arch functions
 *
 * These should be moved to use the CPU uclass.
 */
int cpu_status(u32 nr);
int cpu_reset(u32 nr);
int cpu_disable(u32 nr);
int cpu_release(u32 nr, int argc, char *const argv[]);

static inline int cpumask_next(int cpu, unsigned int mask)
{
	for (cpu++; !((1 << cpu) & mask); cpu++)
		;

	return cpu;
}

#define for_each_cpu(iter, cpu, num_cpus, mask) \
	for (iter = 0, cpu = cpumask_next(-1, mask); \
	     iter < num_cpus; \
	     iter++, cpu = cpumask_next(cpu, mask)) \

int cpu_numcores(void);
int cpu_num_dspcores(void);
u32 cpu_mask(void);
u32 cpu_dsp_mask(void);
int is_core_valid(unsigned int core);

/**
 * checkcpu() - perform an early check of the CPU
 *
 * This is used on PowerPC, SH and X86 machines as a CPU init mechanism. It is
 * called during the pre-relocation init sequence in board_init_f().
 *
 * Return: 0 if oK, -ve on error
 */
int checkcpu(void);

void smp_set_core_boot_addr(unsigned long addr, int corenr);
void smp_kick_all_cpus(void);

int icache_status(void);
void icache_enable(void);
void icache_disable(void);
int dcache_status(void);
void dcache_enable(void);
void dcache_disable(void);
void mmu_disable(void);
int mmu_status(void);

/* arch/$(ARCH)/lib/cache.c */
void enable_caches(void);
void flush_cache(unsigned long addr, unsigned long size);
void flush_dcache_all(void);
void flush_dcache_range(unsigned long start, unsigned long stop);
void invalidate_dcache_range(unsigned long start, unsigned long stop);
void invalidate_dcache_all(void);
void invalidate_icache_all(void);

enum pgprot_attrs {
	MMU_ATTR_RO,
	MMU_ATTR_RX,
	MMU_ATTR_RW,
};

/** pgprot_set_attrs() - Set page table permissions
 *
 * @addr: Physical address start
 * @size: size of memory to change
 * @perm: New permissions
 *
 * Return: 0 on success, error otherwise.
 **/
int pgprot_set_attrs(phys_addr_t addr, size_t size, enum pgprot_attrs perm);

/**
 * noncached_init() - Initialize non-cached memory region
 *
 * Initialize non-cached memory area. This memory region will be typically
 * located right below the malloc() area and mapped uncached in the MMU.
 *
 * It is called during the generic post-relocation init sequence.
 *
 * Return: 0 if OK
 */
int noncached_init(void);
void noncached_set_region(void);

phys_addr_t noncached_alloc(size_t size, size_t align);

enum {
	/* Disable caches (else flush caches but leave them active) */
	CBL_DISABLE_CACHES		= 1 << 0,
	CBL_SHOW_BOOTSTAGE_REPORT	= 1 << 1,

	CBL_ALL				= 3,
};

/**
 * Clean up ready for linux
 *
 * @param flags		Flags to control what is done
 */
int cleanup_before_linux_select(int flags);

void reset_cpu(void);

#endif
