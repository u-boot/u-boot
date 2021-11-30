/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/smp.h>
#include <asm/u-boot.h>
#include <compiler.h>

/* Architecture-specific global data */
struct arch_global_data {
	long boot_hart;		/* boot hart id */
	phys_addr_t firmware_fdt_addr;
#if CONFIG_IS_ENABLED(SIFIVE_CLINT)
	void __iomem *clint;	/* clint base address */
#endif
#ifdef CONFIG_ANDES_PLIC
	void __iomem *plic;	/* plic base address */
#endif
#if CONFIG_IS_ENABLED(SMP)
	struct ipi_data ipi[CONFIG_NR_CPUS];
#endif
#ifndef CONFIG_XIP
	ulong available_harts;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR register gd_t *gd asm ("gp")

static inline void set_gd(volatile gd_t *gd_ptr)
{
#ifdef CONFIG_64BIT
	asm volatile("ld gp, %0\n" : : "m"(gd_ptr));
#else
	asm volatile("lw gp, %0\n" : : "m"(gd_ptr));
#endif
}

#endif /* __ASM_GBL_DATA_H */
