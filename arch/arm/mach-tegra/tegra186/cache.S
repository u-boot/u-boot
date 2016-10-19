/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <linux/linkage.h>

#define SMC_SIP_INVOKE_MCE		0x82FFFF00
#define MCE_SMC_ROC_FLUSH_CACHE		(SMC_SIP_INVOKE_MCE | 11)

ENTRY(__asm_flush_l3_cache)
	mov	x0, #(MCE_SMC_ROC_FLUSH_CACHE & 0xffff)
	movk	x0, #(MCE_SMC_ROC_FLUSH_CACHE >> 16), lsl #16
	mov	x1, #0
	mov	x2, #0
	mov	x3, #0
	mov	x4, #0
	mov	x5, #0
	mov	x6, #0
	smc	#0
	mov	x0, #0
	ret
ENDPROC(__asm_flush_l3_cache)
