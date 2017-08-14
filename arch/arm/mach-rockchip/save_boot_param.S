/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <linux/linkage.h>

#if defined(CONFIG_ARM64)
.globl	SAVE_SP_ADDR
SAVE_SP_ADDR:
	.quad 0

ENTRY(save_boot_params)
	sub	sp, sp, #0x60
	stp	x29, x30, [sp, #0x50]
	stp	x27, x28, [sp, #0x40]
	stp	x25, x26, [sp, #0x30]
	stp	x23, x24, [sp, #0x20]
	stp	x21, x22, [sp, #0x10]
	stp	x19, x20, [sp, #0]
	ldr	x8, =SAVE_SP_ADDR
	mov	x9, sp
	str	x9, [x8]
	b	save_boot_params_ret  /* back to my caller */
ENDPROC(save_boot_params)

.globl _back_to_bootrom_s
ENTRY(_back_to_bootrom_s)
	ldr	x0, =SAVE_SP_ADDR
	ldr	x0, [x0]
	mov	sp, x0
	ldp	x29, x30, [sp, #0x50]
	ldp	x27, x28, [sp, #0x40]
	ldp	x25, x26, [sp, #0x30]
	ldp	x23, x24, [sp, #0x20]
	ldp	x21, x22, [sp, #0x10]
	ldp	x19, x20, [sp]
	add	sp, sp, #0x60
	mov	x0, xzr
	ret
ENDPROC(_back_to_bootrom_s)
#else
.globl	SAVE_SP_ADDR
SAVE_SP_ADDR:
	.word 0

/*
 * void save_boot_params
 *
 * Save sp, lr, r1~r12
 */
ENTRY(save_boot_params)
	push	{r1-r12, lr}
	ldr	r0, =SAVE_SP_ADDR
	str	sp, [r0]
	b	save_boot_params_ret		@ back to my caller
ENDPROC(save_boot_params)


.globl _back_to_bootrom_s
ENTRY(_back_to_bootrom_s)
	ldr	r0, =SAVE_SP_ADDR
	ldr	sp, [r0]
	mov	r0, #0
	pop	{r1-r12, pc}
ENDPROC(_back_to_bootrom_s)
#endif
