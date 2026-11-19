/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Early MMU Disable for Snagboot Mode
 *
 * Disable MMU at the earliest possible point for EL3 entry.
 * Snagboot mode always enters U-Boot at EL3.
 *
 * After disabling MMU, invalidate TLB to clear any stale entries that might
 * cause issues when MMU is re-enabled later.
 */

	mrs	x0, CurrentEL
	cmp	x0, #(3 << 2)	/* Verify EL3 entry */
	b.ne	reset		/* Unexpected: not at EL3 */

	/* Disable MMU at EL3 */
	mrs	x0, sctlr_el3
	bic	x0, x0, #1	/* Clear M bit (MMU enable) */
	bic	x0, x0, #(1 << 2)   /* Clear C bit (Dcache enable) */
	bic	x0, x0, #(1 << 19)  /* Clear WXN bit (Write XOR Execute) */
	msr	sctlr_el3, x0
	isb
	/* Invalidate entire TLB for all ELs */
	tlbi	alle3
	dsb	sy
	isb
	b	reset
