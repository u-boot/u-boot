/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Loop to run on secondary cores for LoongArch CPU
 *
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <linux/linkage.h>
#include <asm/asm.h>
#include <asm/loongarch.h>
#include <asm/arch/entry-init.h>

ENTRY(secondary_core_loop)
	smp_secondary_setup

	PTR_LI		t0, LOONGARCH_IOCSR_MBUF0
	LONG_IOCSRWR	zero, t0

	/* Enable IPI interrupt for wakeup */
	LONG_LI		t0, ECFGF_IPI
	csrxchg		t0, t0, LOONGARCH_CSR_ECFG

	LONG_LI		t0, 0xffffffff
	li.w		t1, LOONGARCH_IOCSR_IPI_CLEAR
	iocsrwr.w	t0, t1
	li.w		t1, LOONGARCH_IOCSR_IPI_EN
	iocsrwr.w	t0, t1

	/* t1 for spin table */
	PTR_LI		t1, LOONGARCH_IOCSR_MBUF0

1:
	/* Query spin table */
	idle 0
	nop
	iocsrrd.w	t0, t1
	beqz		t0, 1b

	/* Clear IPI interrupt */
	PTR_LI		t1, LOONGARCH_IOCSR_IPI_STATUS
	iocsrrd.w	t0, t1
	PTR_LI		t1, LOONGARCH_IOCSR_IPI_CLEAR
	iocsrwr.w	t0, t1

	/* Mask all interrupts */
	LONG_LI		t0, ECFG0_IM
	csrxchg		zero, t0, LOONGARCH_CSR_ECFG

	/* Jump to secondary core */
	PTR_LI		t1, LOONGARCH_IOCSR_MBUF0
	LONG_IOCSRRD	t0, t1
	/* If we ever return, put us back to the loop */
	la.pcrel	ra, secondary_core_loop
	jirl		zero, t0, 0
END(secondary_core_loop)
