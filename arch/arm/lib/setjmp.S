/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <config.h>
#include <asm/assembler.h>
#include <linux/linkage.h>

.pushsection .text.setjmp, "ax"
ENTRY(setjmp)
	/*
	 * A subroutine must preserve the contents of the registers
	 * r4-r8, r10, r11 (v1-v5, v7 and v8) and SP (and r9 in PCS
	 * variants that designate r9 as v6).
	 */
	mov  ip, sp
	stm  a1, {v1-v8, ip, lr}
	mov  a1, #0
	ret  lr
ENDPROC(setjmp)
.popsection

.pushsection .text.longjmp, "ax"
ENTRY(longjmp)
	ldm  a1, {v1-v8, ip, lr}
	mov  sp, ip
	mov  a1, a2
	/* If we were passed a return value of zero, return one instead */
	cmp  a1, #0
	bne  1f
	mov  a1, #1
1:
	ret  lr
ENDPROC(longjmp)
.popsection

.pushsection .text.initjmp, "ax"
ENTRY(initjmp)
	stm  a1, {v1-v8}
	/* a2: entry point address, a3: stack base, a4: stack size */
	add  a3, a3, a4
	str  a3, [a1, #32]  /* where setjmp would save sp */
	str  a2, [a1, #36]  /* where setjmp would save lr */
	mov  a1, #0
	ret  lr
ENDPROC(initjmp)
.popsection
