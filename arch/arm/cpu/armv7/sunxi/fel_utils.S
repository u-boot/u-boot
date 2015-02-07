/*
 * Utility functions for FEL mode.
 *
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/system.h>
#include <linux/linkage.h>

ENTRY(save_boot_params)
	ldr	r0, =fel_stash
	str	sp, [r0, #0]
	str	lr, [r0, #4]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

ENTRY(return_to_fel)
	mov	sp, r0
	mov	lr, r1
	bx	lr
ENDPROC(return_to_fel)
