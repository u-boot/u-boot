/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SoC specific lowlevel_init for AMD Versal Gen 2
 *
 * Copyright (C) 2026, Advanced Micro Devices, Inc.
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/arch/hardware.h>

ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

#if defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
	/*
	 * Pick the redistributor base for this part. Default to GICR_BASE
	 * and switch to GICR_BASE_2VM3654 when the PMC TAP IDCODE matches
	 * the 2VM3654 device.
	 */
	ldr	x0, =GICR_BASE
	ldr	x1, =PMC_TAP_IDCODE
	ldr	w1, [x1]
	ldr	w2, =GICR_IDCODE_2VM3654
	cmp	w1, w2
	b.ne	3f
	ldr	x0, =GICR_BASE_2VM3654
3:
	bl	gic_init_secure_percpu
#endif

	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)
