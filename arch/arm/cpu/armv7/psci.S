/*
 * Copyright (C) 2013,2014 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/psci.h>

	.pushsection ._secure.text, "ax"

	.arch_extension	sec

	.align	5
	.globl _psci_vectors
_psci_vectors:
	b	default_psci_vector	@ reset
	b	default_psci_vector	@ undef
	b	_smc_psci		@ smc
	b	default_psci_vector	@ pabort
	b	default_psci_vector	@ dabort
	b	default_psci_vector	@ hyp
	b	default_psci_vector	@ irq
	b	psci_fiq_enter		@ fiq

ENTRY(psci_fiq_enter)
	movs	pc, lr
ENDPROC(psci_fiq_enter)
.weak psci_fiq_enter

ENTRY(default_psci_vector)
	movs	pc, lr
ENDPROC(default_psci_vector)
.weak default_psci_vector

ENTRY(psci_cpu_suspend)
ENTRY(psci_cpu_off)
ENTRY(psci_cpu_on)
ENTRY(psci_migrate)
	mov	r0, #ARM_PSCI_RET_NI	@ Return -1 (Not Implemented)
	mov	pc, lr
ENDPROC(psci_migrate)
ENDPROC(psci_cpu_on)
ENDPROC(psci_cpu_off)
ENDPROC(psci_cpu_suspend)
.weak psci_cpu_suspend
.weak psci_cpu_off
.weak psci_cpu_on
.weak psci_migrate

_psci_table:
	.word	ARM_PSCI_FN_CPU_SUSPEND
	.word	psci_cpu_suspend
	.word	ARM_PSCI_FN_CPU_OFF
	.word	psci_cpu_off
	.word	ARM_PSCI_FN_CPU_ON
	.word	psci_cpu_on
	.word	ARM_PSCI_FN_MIGRATE
	.word	psci_migrate
	.word	0
	.word	0

_smc_psci:
	push	{r4-r7,lr}

	@ Switch to secure
	mrc	p15, 0, r7, c1, c1, 0
	bic	r4, r7, #1
	mcr	p15, 0, r4, c1, c1, 0
	isb

	adr	r4, _psci_table
1:	ldr	r5, [r4]		@ Load PSCI function ID
	ldr	r6, [r4, #4]		@ Load target PC
	cmp	r5, #0			@ If reach the end, bail out
	moveq	r0, #ARM_PSCI_RET_INVAL	@ Return -2 (Invalid)
	beq	2f
	cmp	r0, r5			@ If not matching, try next entry
	addne	r4, r4, #8
	bne	1b

	blx	r6			@ Execute PSCI function

	@ Switch back to non-secure
2:	mcr	p15, 0, r7, c1, c1, 0

	pop	{r4-r7, lr}
	movs	pc, lr			@ Return to the kernel

	.popsection
