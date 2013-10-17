/*
 * code for switching cores into non-secure state and into HYP mode
 *
 * Copyright (c) 2013	Andre Przywara <andre.przywara@linaro.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/gic.h>
#include <asm/armv7.h>

.arch_extension sec
.arch_extension virt

/* the vector table for secure state and HYP mode */
_monitor_vectors:
	.word 0	/* reset */
	.word 0 /* undef */
	adr pc, _secure_monitor
	.word 0
	.word 0
	adr pc, _hyp_trap
	.word 0
	.word 0

/*
 * secure monitor handler
 * U-boot calls this "software interrupt" in start.S
 * This is executed on a "smc" instruction, we use a "smc #0" to switch
 * to non-secure state.
 * We use only r0 and r1 here, due to constraints in the caller.
 */
	.align	5
_secure_monitor:
	mrc	p15, 0, r1, c1, c1, 0		@ read SCR
	bic	r1, r1, #0x4e			@ clear IRQ, FIQ, EA, nET bits
	orr	r1, r1, #0x31			@ enable NS, AW, FW bits

#ifdef CONFIG_ARMV7_VIRT
	mrc	p15, 0, r0, c0, c1, 1		@ read ID_PFR1
	and	r0, r0, #CPUID_ARM_VIRT_MASK	@ mask virtualization bits
	cmp	r0, #(1 << CPUID_ARM_VIRT_SHIFT)
	orreq	r1, r1, #0x100			@ allow HVC instruction
#endif

	mcr	p15, 0, r1, c1, c1, 0		@ write SCR (with NS bit set)

#ifdef CONFIG_ARMV7_VIRT
	mrceq	p15, 0, r0, c12, c0, 1		@ get MVBAR value
	mcreq	p15, 4, r0, c12, c0, 0		@ write HVBAR
#endif

	movs	pc, lr				@ return to non-secure SVC

_hyp_trap:
	mrs	lr, elr_hyp	@ for older asm: .byte 0x00, 0xe3, 0x0e, 0xe1
	mov pc, lr				@ do no switch modes, but
						@ return to caller

/*
 * Secondary CPUs start here and call the code for the core specific parts
 * of the non-secure and HYP mode transition. The GIC distributor specific
 * code has already been executed by a C function before.
 * Then they go back to wfi and wait to be woken up by the kernel again.
 */
ENTRY(_smp_pen)
	mrs	r0, cpsr
	orr	r0, r0, #0xc0
	msr	cpsr, r0			@ disable interrupts
	ldr	r1, =_start
	mcr	p15, 0, r1, c12, c0, 0		@ set VBAR

	bl	_nonsec_init
	mov	r12, r0				@ save GICC address
#ifdef CONFIG_ARMV7_VIRT
	bl	_switch_to_hyp
#endif

	ldr	r1, [r12, #GICC_IAR]		@ acknowledge IPI
	str	r1, [r12, #GICC_EOIR]		@ signal end of interrupt

	adr	r0, _smp_pen			@ do not use this address again
	b	smp_waitloop			@ wait for IPIs, board specific
ENDPROC(_smp_pen)

/*
 * Switch a core to non-secure state.
 *
 *  1. initialize the GIC per-core interface
 *  2. allow coprocessor access in non-secure modes
 *  3. switch the cpu mode (by calling "smc #0")
 *
 * Called from smp_pen by secondary cores and directly by the BSP.
 * Do not assume that the stack is available and only use registers
 * r0-r3 and r12.
 *
 * PERIPHBASE is used to get the GIC address. This could be 40 bits long,
 * though, but we check this in C before calling this function.
 */
ENTRY(_nonsec_init)
#ifdef CONFIG_ARM_GIC_BASE_ADDRESS
	ldr	r2, =CONFIG_ARM_GIC_BASE_ADDRESS
#else
	mrc	p15, 4, r2, c15, c0, 0		@ read CBAR
	bfc	r2, #0, #15			@ clear reserved bits
#endif
	add	r3, r2, #GIC_DIST_OFFSET	@ GIC dist i/f offset
	mvn	r1, #0				@ all bits to 1
	str	r1, [r3, #GICD_IGROUPRn]	@ allow private interrupts

	mrc	p15, 0, r0, c0, c0, 0		@ read MIDR
	ldr	r1, =MIDR_PRIMARY_PART_MASK
	and	r0, r0, r1			@ mask out variant and revision

	ldr	r1, =MIDR_CORTEX_A7_R0P0 & MIDR_PRIMARY_PART_MASK
	cmp	r0, r1				@ check for Cortex-A7

	ldr	r1, =MIDR_CORTEX_A15_R0P0 & MIDR_PRIMARY_PART_MASK
	cmpne	r0, r1				@ check for Cortex-A15

	movne	r1, #GIC_CPU_OFFSET_A9		@ GIC CPU offset for A9
	moveq	r1, #GIC_CPU_OFFSET_A15		@ GIC CPU offset for A15/A7
	add	r3, r2, r1			@ r3 = GIC CPU i/f addr

	mov	r1, #1				@ set GICC_CTLR[enable]
	str	r1, [r3, #GICC_CTLR]		@ and clear all other bits
	mov	r1, #0xff
	str	r1, [r3, #GICC_PMR]		@ set priority mask register

	movw	r1, #0x3fff
	movt	r1, #0x0006
	mcr	p15, 0, r1, c1, c1, 2		@ NSACR = all copros to non-sec

/* The CNTFRQ register of the generic timer needs to be
 * programmed in secure state. Some primary bootloaders / firmware
 * omit this, so if the frequency is provided in the configuration,
 * we do this here instead.
 * But first check if we have the generic timer.
 */
#ifdef CONFIG_SYS_CLK_FREQ
	mrc	p15, 0, r0, c0, c1, 1		@ read ID_PFR1
	and	r0, r0, #CPUID_ARM_GENTIMER_MASK	@ mask arch timer bits
	cmp	r0, #(1 << CPUID_ARM_GENTIMER_SHIFT)
	ldreq	r1, =CONFIG_SYS_CLK_FREQ
	mcreq	p15, 0, r1, c14, c0, 0		@ write CNTFRQ
#endif

	adr	r1, _monitor_vectors
	mcr	p15, 0, r1, c12, c0, 1		@ set MVBAR to secure vectors

	mrc	p15, 0, ip, c12, c0, 0		@ save secure copy of VBAR

	isb
	smc	#0				@ call into MONITOR mode

	mcr	p15, 0, ip, c12, c0, 0		@ write non-secure copy of VBAR

	mov	r1, #1
	str	r1, [r3, #GICC_CTLR]		@ enable non-secure CPU i/f
	add	r2, r2, #GIC_DIST_OFFSET
	str	r1, [r2, #GICD_CTLR]		@ allow private interrupts

	mov	r0, r3				@ return GICC address

	bx	lr
ENDPROC(_nonsec_init)

#ifdef CONFIG_SMP_PEN_ADDR
/* void __weak smp_waitloop(unsigned previous_address); */
ENTRY(smp_waitloop)
	wfi
	ldr	r1, =CONFIG_SMP_PEN_ADDR	@ load start address
	ldr	r1, [r1]
	cmp	r0, r1			@ make sure we dont execute this code
	beq	smp_waitloop		@ again (due to a spurious wakeup)
	mov	pc, r1
ENDPROC(smp_waitloop)
.weak smp_waitloop
#endif

ENTRY(_switch_to_hyp)
	mov	r0, lr
	mov	r1, sp				@ save SVC copy of LR and SP
	isb
	hvc #0			 @ for older asm: .byte 0x70, 0x00, 0x40, 0xe1
	mov	sp, r1
	mov	lr, r0				@ restore SVC copy of LR and SP

	bx	lr
ENDPROC(_switch_to_hyp)
