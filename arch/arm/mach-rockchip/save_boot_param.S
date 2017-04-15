/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <linux/linkage.h>

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
