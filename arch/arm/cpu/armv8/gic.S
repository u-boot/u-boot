/*
 * GIC Initialization Routines.
 *
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/gic.h>


/*************************************************************************
 *
 * void gic_init(void) __attribute__((weak));
 *
 * Currently, this routine only initialize secure copy of GIC
 * with Security Extensions at EL3.
 *
 *************************************************************************/
WEAK(gic_init)
	branch_if_slave	x0, 2f

	/* Initialize Distributor and SPIs */
	ldr	x1, =GICD_BASE
	mov	w0, #0x3		/* EnableGrp0 | EnableGrp1 */
	str	w0, [x1, GICD_CTLR]	/* Secure GICD_CTLR */
	ldr	w0, [x1, GICD_TYPER]
	and	w2, w0, #0x1f		/* ITLinesNumber */
	cbz	w2, 2f			/* No SPIs */
	add	x1, x1, (GICD_IGROUPRn + 4)
	mov	w0, #~0			/* Config SPIs as Grp1 */
1:	str	w0, [x1], #0x4
	sub	w2, w2, #0x1
	cbnz	w2, 1b

	/* Initialize SGIs and PPIs */
2:	ldr	x1, =GICD_BASE
	mov	w0, #~0			/* Config SGIs and PPIs as Grp1 */
	str	w0, [x1, GICD_IGROUPRn]	/* GICD_IGROUPR0 */
	mov	w0, #0x1		/* Enable SGI 0 */
	str	w0, [x1, GICD_ISENABLERn]

	/* Initialize Cpu Interface */
	ldr	x1, =GICC_BASE
	mov	w0, #0x1e7		/* Disable IRQ/FIQ Bypass & */
					/* Enable Ack Group1 Interrupt & */
					/* EnableGrp0 & EnableGrp1 */
	str	w0, [x1, GICC_CTLR]	/* Secure GICC_CTLR */

	mov	w0, #0x1 << 7		/* Non-Secure access to GICC_PMR */
	str	w0, [x1, GICC_PMR]

	ret
ENDPROC(gic_init)


/*************************************************************************
 *
 * void gic_send_sgi(u64 sgi) __attribute__((weak));
 *
 *************************************************************************/
WEAK(gic_send_sgi)
	ldr	x1, =GICD_BASE
	mov	w2, #0x8000
	movk	w2, #0x100, lsl #16
	orr	w2, w2, w0
	str	w2, [x1, GICD_SGIR]
	ret
ENDPROC(gic_send_sgi)


/*************************************************************************
 *
 * void wait_for_wakeup(void) __attribute__((weak));
 *
 * Wait for SGI 0 from master.
 *
 *************************************************************************/
WEAK(wait_for_wakeup)
	ldr	x1, =GICC_BASE
0:	wfi
	ldr	w0, [x1, GICC_AIAR]
	str	w0, [x1, GICC_AEOIR]
	cbnz	w0, 0b
	ret
ENDPROC(wait_for_wakeup)


/*************************************************************************
 *
 * void smp_kick_all_cpus(void) __attribute__((weak));
 *
 *************************************************************************/
WEAK(smp_kick_all_cpus)
	/* Kick secondary cpus up by SGI 0 interrupt */
	mov	x0, xzr			/* SGI 0 */
	mov	x29, lr			/* Save LR */
	bl	gic_send_sgi
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(smp_kick_all_cpus)
