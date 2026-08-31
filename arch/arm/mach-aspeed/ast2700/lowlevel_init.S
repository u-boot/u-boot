/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) ASPEED Technology Inc.
 */
#include <config.h>
#include <linux/linkage.h>

/*
 *        SMP mailbox
 * +-----------------------+ 0x40
 * |                       |
 * | mailbox insn. for     |
 * | cpuN GO sign polling  |
 * |                       |
 * +-----------------------+ 0x20
 * | cpu3 entrypoint       |
 * +-----------------------+ 0x18
 * | cpu2 entrypoint       |
 * +-----------------------+ 0x10
 * | cpu1 entrypoint       |
 * +-----------------------+ 0x8
 * | reserved              |
 * +-----------------------+ 0x4
 * | mailbox ready         |
 * +-----------------------+ SCU_CPU + 0x780
 */

#define SCU_CPU_BASE		0x12c02000
#define SCU_CPU_SMP_READY	(SCU_CPU_BASE + 0x780)
#define SCU_CPU_SMP_EP1		(SCU_CPU_BASE + 0x788)
#define SCU_CPU_SMP_EP2		(SCU_CPU_BASE + 0x790)
#define SCU_CPU_SMP_EP3		(SCU_CPU_BASE + 0x798)
#define SCU_CPU_SMP_POLLINSN	(SCU_CPU_BASE + 0x7a0)

ENTRY(lowlevel_init)
	/* backup LR */
	mov	x29, lr

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	/* reset SMP mailbox ASAP */
	ldr	x0, =SCU_CPU_SMP_READY
	str	wzr, [x0]

	/*
	 * get cpu core id
	 *
	 * ast2700 has 1-cluster, 4-cores CPU topology.
	 * Affinity level 0 in MPIDR is sufficient.
	 */
	mrs	x4, mpidr_el1
	ands	x4, x4, #0xff

	/* cpu0 is the primary core to setup SMP mailbox */
	beq	do_primary_core_setup

	/* hold cpuN until mailbox is ready */
	ldr	x0, =SCU_CPU_SMP_READY
	movz	w1, #0xcafe
	movk	w1, #0xbabe, lsl #16

poll_mailbox_ready:
	wfe
	ldr	w2, [x0]
	cmp	w1, w2
	bne	poll_mailbox_ready

	/*
	 * parameters for relocated SMP go polling insn.
	 *  x4 = cpu id
	 *  x5 = SCU_CPU_SMP_EPx
	 */
	add	x5, x0, x4, lsl #3

	/* jump to the polling loop in SMP mailbox, no return */
	ldr	x0, =SCU_CPU_SMP_POLLINSN
	br	x0

do_primary_core_setup:
	/* relocate mailbox insn. for cpuN to poll for SMP go signal */
	adr	x0, smp_mbox_insn
	adr	x1, smp_mbox_insn_end
	ldr	x2, =SCU_CPU_SMP_POLLINSN

relocate_smp_mbox_insn:
	ldr	w3, [x0], #0x4
	str	w3, [x2], #0x4
	cmp	x0, x1
	bne relocate_smp_mbox_insn

	/* reset cpuN entrypoints */
	ldr	x0, =SCU_CPU_SMP_EP1
	str	xzr, [x0], #8
	str	xzr, [x0], #8
	str	xzr, [x0]

	/* notify cpuN that SMP mailbox is ready */
	movz	w0, #0xcafe
	movk	w0, #0xbabe, lsl #16
	ldr	x1, =SCU_CPU_SMP_READY
	str	w0, [x1]

	sev
#endif	/* !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT) */

	/* back to arch calling code */
	mov	lr, x29
	ret
ENDPROC(lowlevel_init)

/*
 * insn. inside mailbox to poll SMP go signal.
 *
 * Note that this code will be relocated, any absolute
 * addressing should NOT be used.
 */
smp_mbox_insn:
	/*
	 * x4 = cpu id
	 * x5 = SCU_CPU_SMP_EPx
	 */
poll_smp_mbox_go:
	wfe
	ldr	x0, [x5]
	cmp	x0, xzr
	beq	poll_smp_mbox_go

	/* jump to secondary core entrypoint */
	br	x0

smp_mbox_insn_end:
	/* should never reach */
	b	.
